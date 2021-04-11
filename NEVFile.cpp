#include "NEVFile.h"

#include "datapacket.h"

#include <iostream>
#include <cstring>
NEVFile::NEVFile(std::string filename, size_t buffersize) :
  BUFFERSIZE(buffersize)
{

  this->file.open(filename, std::ios_base::binary);
  //  this->file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  if(!this->file) {
    std::cerr << "Failed to open file " << filename << "|\n" << std::endl;
    throw(std::runtime_error("Cannot open file for reading"));
  }
  
  auto nHeaders = readBasicHeader();
  readExtendedHeaders(nHeaders);
  
  
  buffer = new uint8_t[BUFFERSIZE*packetSize];
  file.read(reinterpret_cast<char*>(buffer), BUFFERSIZE*packetSize);
  buffer_capacity = file.gcount();
  buffer_pos = 0;
}


NEVFile::~NEVFile() {
  this->file.close();
  delete [] this->buffer;
}

std::uint32_t NEVFile::readBasicHeader() {
  char* buffer = new char[200]; //largest field is 200 bytes (we'll reuse this)

  /* Read the magic word to ensure this is the right kind of file*/      
  file.read(buffer, sizeof(char)*8);
  if(!std::equal(buffer, buffer+8, "NEURALEV")) {
    std::cerr << buffer << std::endl << "vs" << std::endl << "NEURALEV" << std::endl;
    throw(std::runtime_error("Not a Ripple NEV (neural event) file"));
  }
  
  file.read(reinterpret_cast<char*>(&majorVersion), sizeof(majorVersion));
  file.read(reinterpret_cast<char*>(&minorVersion), sizeof(minorVersion));
  
  file.read(reinterpret_cast<char*>(&flags), sizeof(flags));
  
  file.read(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));
  file.read(reinterpret_cast<char*>(&packetSize), sizeof(packetSize));
  
  file.read(reinterpret_cast<char*>(&fsTimestamp), sizeof(fsTimestamp));
  file.read(reinterpret_cast<char*>(&fsWaveforms), sizeof(fsWaveforms));
  file.read(reinterpret_cast<char*>(&origin), sizeof(origin));
    
  // The next two reads only work because each is bigger than the last (so no need to clear the buffer)
  file.read(buffer, sizeof(char)*32);
  creator = std::string(buffer);
  
  file.read(buffer, sizeof(char)*200);
  comment = std::string(buffer);

  file.ignore(52); // Per spec, reserved for future use

  file.read(reinterpret_cast<char*>(&processorTime), sizeof(processorTime));

  std::uint32_t nHeaders;
  file.read(reinterpret_cast<char*>(&nHeaders), sizeof(nHeaders));

  delete [] buffer;
  return nHeaders;
}

void NEVFile::readExtendedHeaders(const std::uint32_t nHeaders) {
  const size_t EXTENDED_HEADER_SIZE = 32;
  char buffer[EXTENDED_HEADER_SIZE];
    
  for(auto i=0; i < nHeaders; i++) {
    file.read(buffer, EXTENDED_HEADER_SIZE);

    if(std::equal(buffer, buffer+7, "NEUEVWAV")) {
      /* NEUEVWAV events contain metadata about the stimulating and recording amplifiers. This can be
	 distinguished by looking at the neural scale factor, which is zero for stimulation. Thus,
	 we extract everything into an NEUEVWAV object, and then move it into the "right" kind. */
      
      NEUEVWAV tmpHeader;
      std::copy(buffer+8, buffer+8+sizeof(tmpHeader), reinterpret_cast<char*>(&tmpHeader));
      
      if(tmpHeader.electrodeID > 5120)
	stimHeaders.emplace(tmpHeader.electrodeID - 5120, StimHeader(tmpHeader));
      else 
	spikeHeaders.emplace(tmpHeader.electrodeID, SpikeHeader(tmpHeader));      
    }

    else if(std::equal(buffer, buffer+7, "NEUEVFLT")) {
      /* Spike filters are easy--just copy the data over 
	 (depends on getting the alignment right!) */
      SpikeFilter sf;
      std::copy(buffer+8, buffer+8+sizeof(SpikeFilter), reinterpret_cast<char*>(&sf));
      this->spikeFilters.emplace(sf.electrodeID, sf);
    }

    else if(std::equal(buffer, buffer+7, "NEUEVLBL")) {
      /*Label markers are so trivial we can just unpack it right here*/
      std::uint16_t electrodeID;
      char label[16];

      std::copy(buffer+8, buffer+10, reinterpret_cast<char*>(&electrodeID));
      std::copy(buffer+9, buffer+9+16, label);
      this->labels.emplace(electrodeID, std::string(label));
    }

    else if(std::equal(buffer, buffer+7, "DIGLABEL")) {
      this->hasDigitalEvents = true;
      this->digitalMode = static_cast<DigitalMode>(buffer[24]);
    }

    else {
      std::cerr << "Unrecognized extended header detected" << std::endl;
    }
  }   
  return;
}

    
std::ostream& operator<<(std::ostream& out, const NEVFile& f) {

    out << "NEV File (version " << int(f.majorVersion) << '.' << int(f.minorVersion) <<  ")\n";
    out << "Created by " << f.creator << "\n";
    out << "Data collection began at " << f.origin << std::endl;
    out << "Comments: " << f.comment << std::endl;
    out << "Packet Size: " << f.packetSize << std::endl;
    return out;
}

bool NEVFile::eof() const {
  return file.eof() && (this->buffer_pos == this->buffer_capacity);
}


void NEVFile::refillBuffer() {
  if(!file.eof()) {    
    try {
      file.read(reinterpret_cast<char*>(buffer), BUFFERSIZE*packetSize);
    } catch (std::ifstream::failure &e) {
      if(!file.eof())
	throw(e);
    }
    buffer_capacity = file.gcount(); 
    buffer_pos = 0;    
  }
  return;
}

std::shared_ptr<Packet> NEVFile::readPacket(bool digital, bool stim, bool spike) {
  /* Read the next packet of the specifed type(s) and returns a shared_ptr to it. */
  std::shared_ptr<Packet> packet;
  while(!packet && !this->eof()) {
    packet = readPacketOrNull(digital, stim, spike);
  }
  return packet;
}


std::shared_ptr<Packet> NEVFile::readPacketOrNull(bool keep_digital, bool keep_stim, bool keep_spike) {
  /* Read the next packet.  If the corresponding type (digital, stim,
     or spike) is true, parse it and return a shared_ptr.  Otherwise,
     return nullptr.

     This avoids pointlessly allocating and then deallocating structures, particularly 
     SpikePackets. There are a ton of them,  we're not particularly interested in them 
     and the alloc/dealloc consumes a massive amount of runtime (>30% in the destructors alone).
  */

  if(this->eof())
    throw(std::runtime_error("Read past end of time"));

  if(this->buffer_capacity == this->buffer_pos) {
    refillBuffer();
  }
  
  std::uint32_t timestamp;
  std::uint16_t packetID;

  auto start = buffer + buffer_pos + sizeof(timestamp);
  std::copy(start, start+sizeof(packetID), reinterpret_cast<char*>(&packetID));

  std::shared_ptr<Packet> p = nullptr;

  if(packetID == 0 && keep_digital) {
      p = parseCurrentAsDigital();
  } else if(packetID <= 512 && keep_spike) {    
      p = parseCurrentAsSpike();
  } else if (packetID > 512 && keep_stim) {
      p = parseCurrentAsStim();
  }

  //Peek at the next packet to see if it is a continuation packet
  this->buffer_pos+=this->packetSize;
  
  while(true) {
    if(this->buffer_pos == this->buffer_capacity) {
      if(this->eof())
	return p; 
      else
	refillBuffer();
    }

    start = buffer + buffer_pos;
    std::copy(start, start+sizeof(timestamp),
	      reinterpret_cast<char*>(&timestamp));    
    if(timestamp != 0xFFFFFFU) //not a continuation packet
      break;

    if(!p) { // a continuation packet, but we're ignoring it (wrong type)
      this->buffer_pos += this->packetSize;
      continue; 
    } else {
      
      auto wavep = std::dynamic_pointer_cast<WavePacket>(p);
    
      auto   newlen  = wavep->len + this->packetSize - sizeof(timestamp);
      char*  newdata = new char[newlen];
    
      std::copy(wavep->waveform, wavep->waveform + wavep->len, newdata);
      std::copy(start, start + this->packetSize - sizeof(timestamp),
		newdata + wavep->len);

      delete [] wavep->waveform;
      wavep->waveform = newdata;
      wavep->len      = newlen;
      buffer_pos += this->packetSize;
    }
  }

  return p;	          
}


std::shared_ptr<DigitalPacket> NEVFile::parseCurrentAsDigital() {
  auto start = buffer + buffer_pos;
  std::shared_ptr<DigitalPacket> p(new DigitalPacket);

  std::copy(start, start+sizeof(p->timestamp),
	    reinterpret_cast<char*>(&(p->timestamp)));

  start+=sizeof(p->timestamp) + 2; //+ 2 to skip the packetID
  
  DigitalReason reason;
  std::copy(start, start+sizeof(p->reason), reinterpret_cast<char*>(&reason));
  p->reason = reason;
  
  start+=sizeof(p->reason) + 1;  //Skipping a byte reserved for future use

  std::copy(start, start+sizeof(p->parallel), reinterpret_cast<char*>(&(p->parallel)));
  start+=sizeof(p->parallel);

  std::copy(start, start+sizeof(p->SMA1), reinterpret_cast<char*>(&(p->SMA1)));
  start+=sizeof(p->SMA1);
  
  std::copy(start, start+sizeof(p->SMA2), reinterpret_cast<char*>(&(p->SMA2)));
  start+=sizeof(p->SMA2);

  std::copy(start, start+sizeof(p->SMA3), reinterpret_cast<char*>(&(p->SMA3)));
  start+=sizeof(p->SMA3);

  std::copy(start, start+sizeof(p->SMA4), reinterpret_cast<char*>(&(p->SMA4)));
  start+=sizeof(p->SMA4);

  return p;
}


std::shared_ptr<SpikePacket> NEVFile::parseCurrentAsSpike() {
   auto start = buffer + buffer_pos;
   std::shared_ptr<SpikePacket> p(new SpikePacket);

   std::copy(start, start+sizeof(p->timestamp), reinterpret_cast<char*>(&(p->timestamp)));
   start+=sizeof(p->timestamp);

   std::copy(start, start+sizeof(p->electrodeID), reinterpret_cast<char*>(&(p->electrodeID)));
   start+=sizeof(p->electrodeID);

   std::copy(start, start+sizeof(p->unit), reinterpret_cast<char*>(&(p->unit)));
   start+=sizeof(p->unit) + 1; // Skip reserved byte

   p->waveform = new char[this->packetSize  - 8]; //Now managed by SpikePacket
   p->len = this->packetSize - 8;
   std::copy(start, start+this->packetSize-8, p->waveform);
     

   return p;
}

std::shared_ptr<StimPacket> NEVFile::parseCurrentAsStim() {
   auto start = buffer + buffer_pos;
   std::shared_ptr<StimPacket> p(new StimPacket);

   std::copy(start, start+sizeof(p->timestamp), reinterpret_cast<char*>(&(p->timestamp)));
   start+=sizeof(p->timestamp);
   std::copy(start, start+sizeof(p->electrodeID), reinterpret_cast<char*>(&(p->electrodeID)));
   p->electrodeID -= 5120; //Remove offset
   start+=sizeof(p->electrodeID) + 2;

   p->waveform = new char[this->packetSize  - 8]; //Now managed by StimPacket
   p->len = this->packetSize - 8;
   std::copy(start, start+this->packetSize-8, p->waveform);     

   return p;
  
}

std::ostream& operator<<(std::ostream& out, const DigitalMode& m) {
  switch(m) {
    case SERIAL_MODE: out << "Serial";   break;	
    case PARALLEL:    out << "Parallel"; break;
    }
  return out;
}

	    
  
  

  
  
  
