#include <cstddef>
#include <iostream>
#include <cstdint>


#include "MatFile.h"
#include "NEVFile.h"
#include "NEVConfig.h"
#include "datapacket.h"
#include "eventsoa.h"


void saveEventsCSV(const NEVConfig &config, const NEVFile &f, const EventSOA &ev);
void saveEventsMatlab(const NEVConfig &c, const NEVFile &f, const EventSOA &ev);
void saveEventsText(const NEVConfig &config, const NEVFile &file, const EventSOA &ev);

void saveStimText(const NEVConfig &config, const NEVFile &file, const std::vector<std::shared_ptr<StimPacket>> &sp);
void saveStimCSV(const NEVConfig &config, const NEVFile &file, const std::vector<std::shared_ptr<StimPacket>> &sp);
void saveStimMatlab(const NEVConfig &config, const NEVFile &file, const std::vector<std::shared_ptr<StimPacket>> &sp);


template <typename T>
void toDouble(std::shared_ptr<StimPacket> ev, double* dest, int bps) {
  std::cerr << "Converting with bps=" << bps << std::endl;
  T* casted = reinterpret_cast<T*>(ev->waveform);
  for(size_t i=0; i < (ev->len / bps); i++) {
    dest[i] = double(casted[i]);
  }
  return;
}

/*void saveStimMatlab(const NEVConfig &config,
		    const NEVFile &f,
		    const std::vector<std::shared_ptr<StimPacket>> &sp) {
}*/

template<typename T>
void notImplemented(const NEVConfig &c, const NEVFile &f, const T &t) {
  throw(std::runtime_error("Method not (yet) implemented"));
}




typedef void (*event_writer_ptr)(const NEVConfig &,
				 const NEVFile &,
				 const EventSOA &);
typedef void  (*stim_writer_ptr)(const NEVConfig &,
	 			 const NEVFile &,
				 const std::vector<std::shared_ptr<StimPacket>>&);
typedef void (*spike_writer_ptr)(const NEVConfig &,
				 const NEVFile &,
				 const std::vector<std::shared_ptr<SpikePacket>>&);


const event_writer_ptr eventWriters[3] = {
  saveEventsText,
  saveEventsCSV,
  saveEventsMatlab
};

const stim_writer_ptr stimWriters[3] = {
  saveStimText,
  saveStimCSV,
  saveStimMatlab,
};

const spike_writer_ptr spikeWriters[3] = {
  notImplemented,
  notImplemented,
  notImplemented
};



int main(int argc, char* argv[]) {

  // Parse config and figure out what to save
  NEVConfig config;
  config.parse(argc, argv);
  bool saveEvent  =  !(config.eventFileTypes().empty());
  bool saveStim    = !(config.stimFileTypes().empty());
  bool saveSpike   = !(config.spikeFileTypes().empty());

  // Accumulate packets into these
  EventSOA ev;
  std::vector<std::shared_ptr<StimPacket>>    stim;
  std::vector<std::shared_ptr<SpikePacket>>   spike;
    
  // Read from the file
  NEVFile nev(config.input());
  std::shared_ptr<Packet> packet;


  while(!nev.eof()) {
    packet = nev.readPacket(saveEvent, saveStim, saveSpike);
    
    if(auto p = std::dynamic_pointer_cast<DigitalPacket>(packet)) {
        ev.addPacket(p);
    } else if(auto p = std::dynamic_pointer_cast<StimPacket>(packet)) {
         stim.push_back(p);
    } else if(auto p = std::dynamic_pointer_cast<SpikePacket>(packet)) {
        spike.push_back(p);
    }
  }

  //Write to output files
  for(auto fmt : config.eventFileTypes()) {
    std::cout << "Starting to write event" << std::endl;
    eventWriters[fmt](config, nev, ev);
    std::cout << "Finished writing event" << std::endl;
  }

  for(auto fmt: config.stimFileTypes()) {
    std::cout << "Starting to write stim" << std::endl;
    stimWriters[fmt](config, nev, stim);
  }

  for(auto fmt: config.spikeFileTypes()) {
    spikeWriters[fmt](config, nev, spike);
  }

  
  return 0;
}


template<typename T>
std::string toStringHelper(std::shared_ptr<WavePacket> wp, char delim) {
  std::ostringstream ss;
  
  T* tmp = reinterpret_cast<T*>(wp->waveform);
  size_t i;
  for(i=0; i<(wp->len)/sizeof(T) - 1; i++)
    ss << tmp[i] << delim;
  ss << tmp[i];
  
  return ss.str();
}


std::uint8_t getBytesPerSample(const NEVFile &file, const NEVConfig &config, std::uint16_t electrodeID, bool isStim=false) {
  std::uint8_t bytesPerSample = 0;

  if(file.allWaves16Bit()) {
    bytesPerSample = 2;
  } else {
    if(isStim) {
      StimHeader h = file.stimChannels_cfind(electrodeID);
      bytesPerSample = h.bytesPerSample;
  } else {
      SpikeHeader h = file.spikeChannels_cfind(electrodeID);
      bytesPerSample = h.bytesPerSample;
    }
  }
  return bytesPerSample;
}

std::string toString(std::shared_ptr<WavePacket> wp, const NEVFile &file,
		     const NEVConfig &config, char delim=',') {

  unsigned char bytesPerSample = getBytesPerSample(file, config, wp->electrodeID);

  switch(bytesPerSample) {
  case 1:
    return toStringHelper<std::int8_t>(wp, delim);
    break;
  case 2:
    return toStringHelper<std::int16_t>(wp, delim);
    break;
  case 4:
    return toStringHelper<std::int32_t>(wp, delim);
    break;
  case 8:
    return toStringHelper<std::int64_t>(wp, delim);
    break;
  default:
    std::ostringstream ss;
    ss << "Unpacking " << bytesPerSample << " is not supported (yet).";
    throw(std::runtime_error(ss.str()));
  }
}






void saveStimText(const NEVConfig &config, const NEVFile &file, const std::vector<std::shared_ptr<StimPacket>> &sp) {

  const double stampToSec =  1.0 / static_cast<double>(file.timestampFS());
  
  std::string filename = config.stimFilename(OutputFormat::TEXT);
  std::ofstream out(filename);
  if(!out) {
    throw(std::runtime_error("Unable to open " + filename + " for writing."));
  }

  out << "Microstimulation events from " << config.input() << "\n\n";

  for(auto p = sp.begin(); p != sp.end(); p++) {
    

    out << "Microstimulation event at t=" << (**p).timestamp * stampToSec
	<< "sec (tick " << (*p)->timestamp << ")\n"
	<< "\t- Electode: " << (*p)->electrodeID << "\n";

    if(config.includeStimWaves()) {
      out << "\t- Waveform: [" << toString(*p, file, config) << "]\n";
    }
    out << "\n";
  }
  out.close();
}


void saveStimCSV(const NEVConfig &config, const NEVFile &file, const std::vector<std::shared_ptr<StimPacket>> &sp) {
  
  const double stampToSec =  1.0 / static_cast<double>(file.timestampFS());
   
  std::string filename = config.stimFilename(OutputFormat::CSV);
  std::ofstream out(filename);
  if(!out) {
    throw(std::runtime_error("Unable to open " + filename + " for writing."));
  }

  out << "Time,Tic,Channel";
  if(config.includeStimWaves())
    out << ",Waveform";
  out << "\n";
  
   for(auto p = sp.begin(); p != sp.end(); p++) {
     out << (*p)->timestamp * stampToSec << ','
	 << (*p)->timestamp << ','
	 << (*p)->electrodeID << ',';

     if(config.includeStimWaves()) {
       out << toString(*p, file, config);
     }
     out << "\n";
   }
}


void saveStimMatlab(const NEVConfig &config, const NEVFile &f, const std::vector<std::shared_ptr<StimPacket>> &sp) {

  std::string filename = config.stimFilename(OutputFormat::MATLAB);
  std::cout << "   Writing events to matlab file as " << filename << std::endl;
  MATFile m(filename, "wz");

  /* Set up the field names for the struct array */
  static const char* event_fieldnames[] = {
    "time",      // 0
    "tick",      // 1
    "electrode", // 2
    "waveform"   // 3
  };

  size_t n_event_fields;
  if(config.includeStimWaves()) {
    n_event_fields = 4;
  } else {
    n_event_fields = 3;
  }

  const MW::mwSize event_dims[2] = { static_cast<MW::mwSize>(sp.size()), static_cast<MW::mwSize>(1) };

  
  MW::mxArray* eventdata = MW::mxCreateStructArray(2, event_dims, 
						n_event_fields, event_fieldnames);

  const double stampToSec =  1.0 / static_cast<double>(f.timestampFS());

  
  /* We're going to cache the amp digitization factor and the bytes per sample so we don't
     have to go back to the file object over and over again. We can use -1*/
  std::vector<float>  toVoltFactor(5120, 0);
  std::vector<std::uint8_t> bytesPerSample(5120,0); 

  unsigned file_index = 0;
  for(auto ev = sp.cbegin(); ev!=sp.end(); ev++, file_index++) {

    std::uint16_t electrodeID = (*ev)->electrodeID;

    
    MW::mxSetFieldByNumber(eventdata, file_index, 0,
			   MW::mxCreateDoubleScalar(static_cast<double>((*ev)->timestamp) * stampToSec));
    MW::mxSetFieldByNumber(eventdata, file_index, 1,
			   MW::mxCreateDoubleScalar(static_cast<double>((*ev)->timestamp)));
    MW::mxSetFieldByNumber(eventdata, file_index, 2,
			   MW::mxCreateDoubleScalar(static_cast<double>(electrodeID)));

    if(config.includeStimWaves()) {
      auto mat = MW::mxCreateDoubleMatrix((*ev)->len, 1, MW::mxREAL);
      auto ptr = MW::mxGetPr(mat);

      // Load things into the cache, if necessary. (There will be a spurious cache miss if there
      // the conversion factor is 0 V/bit, but that's your fault for doing something dumb).
      if(toVoltFactor[electrodeID] == 0) {
          auto hdr = f.stimChannels_cfind(electrodeID);
          toVoltFactor[electrodeID] = hdr.scaleFactor;
          if(f.allWaves16Bit())
              bytesPerSample[electrodeID] = 2;
          else
              bytesPerSample[electrodeID] = int(hdr.bytesPerSample);
        }
	

      switch(bytesPerSample[electrodeID]) {
         case 1:
	   toDouble<char>(*ev, ptr, bytesPerSample[electrodeID]);
	  break;
      
        case 2:
	  toDouble<std::int16_t>(*ev, ptr, bytesPerSample[electrodeID]);
	  break;

        case 4:
	  toDouble<std::int32_t>(*ev, ptr, bytesPerSample[electrodeID]);
	  break;
      default:
	;//std::cerr << "NOT IMPLEMENTED: BPS=" << int(bytesPerSample[electrodeID]) <<  "  2VF=" << toVoltFactor[electrodeID] << std::endl;
      }			       
      std::cout << bytesPerSample[electrodeID];
      MW::mxSetFieldByNumber(eventdata, file_index, 3, mat);			     
    }
  }

  m.putScalar("microstim", eventdata);
   
}



			   

  
