#include "NSxFile.h"
#include <stdexcept>

NSxFile::NSxFile(const std::string& filename) {
    
    file.open(filename, std::ios_base::binary | std::ios_base::binary);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    if(!file) {
      throw(std::runtime_error("Cannot open file for reading"));
    }
    
    header = NSxHeader(file);
    for(auto i = 0U; i<header.getChannelCount(); ++i) {
        channels.push_back(NSxChannel(file));
    }
    dataAvailable = true; 
    prepareNextPacket();
   
}

void NSxFile::prepareNextPacket() {
    
    if(!dataAvailable)
        return;
    
    // Check to make sure we're actually at a packet boundary.
    std::uint8_t checkval;
    try {
        file.read(reinterpret_cast<char *>(&checkval), sizeof(checkval));
    } catch(std::ios_base::failure &e) {
        if(file.eof()) {
            dataAvailable = false;
            samplesRemainingInPacket = 0;
            return;
        } else {
            throw;
        }
    }

    if(checkval !=1) {
      throw(std::runtime_error("Invalid NSx Packet header (should be 1)"));
    }
    
    //If we've gotten this far, we are presumably in a valid packet.
    // Update the timestamp and number of remaining points
    
    file.read(reinterpret_cast<char *>(&basetime), sizeof(basetime));
    file.read(reinterpret_cast<char *>(&samplesRemainingInPacket), sizeof(samplesRemainingInPacket));

    if(samplesRemainingInPacket > 0)
        dataAvailable = true;
}



size_t NSxFile::readData(std::uint32_t samplesRequested, std::int16_t *&buffer) {
     
    auto fetchSize = std::min(samplesRequested, samplesRemainingInPacket);
    auto totalPoints = fetchSize * header.getChannelCount();
    auto totalSize = totalPoints * sizeof(std::int16_t);
    
    if(!buffer) {
        buffer = new std::int16_t[totalPoints];     
    }

    try {
      file.read(reinterpret_cast<char *>(buffer), totalSize);
      samplesRemainingInPacket-=fetchSize;
    }
    catch(std::ios_base::failure &e) {
      fetchSize = (file.gcount()/sizeof(std::int16_t)) / header.getChannelCount();
      samplesRemainingInPacket = 0;
    }
     
    if(!samplesRemainingInPacket)
        prepareNextPacket();

    return fetchSize;
}



std::vector<NSxChannel>::const_iterator NSxFile::channelBegin() const {
  return channels.begin();
}



std::vector<NSxChannel>::const_iterator NSxFile::channelEnd() const {
  return channels.end();
}




