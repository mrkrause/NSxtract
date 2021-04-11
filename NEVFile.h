#pragma once
#ifndef __NEVFile_h__
#define __NEVFile_h__

#include <stddef.h>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "filter.h"
#include "systemtime.h"
#include "extheader.h"
#include "datapacket.h"

const uint16_t STIM_CHANNEL_OFFSET = 5120;

enum DigitalMode: std::uint8_t {
  SERIAL_MODE = 0,
  PARALLEL_MODE = 1
 };

std::ostream& operator<<(std::ostream& out, const DigitalMode& m) {
  switch(m) {
    case SERIAL_MODE: out << "Serial";   break;	
    case PARALLEL:    out << "Parallel"; break;
    }
  return out;
}

class NEVFile {
public:
  NEVFile(std::string filename, size_t BUFFERSIZE=1000);
  ~NEVFile();

  bool eof() const;
  std::shared_ptr<Packet> readPacket(bool digital=true, bool stim=true, bool spike=true);
  
  // Iterators to access spike channel headers
  auto spikeChannels_cbegin() const { return spikeHeaders.cbegin(); }
  auto spikeChannels_cend()   const { return spikeHeaders.cend();   }
  auto spikeChannels_cfind(std::uint16_t electrodeID) const {
    return spikeHeaders.at(electrodeID);
  }
  // Iterators to access spike filters
  auto spikeFilters_cbegin()  const { return spikeFilters.cbegin(); }
  auto spikeFilters_cend()    const { return spikeFilters.cend();   }

  // Iterators to access stimulation channel headers
  auto stimChannels_cbegin()  const { return stimHeaders.cbegin(); }
  auto stimChannels_cend()    const { return stimHeaders.cend();   }
  auto stimChannels_cfind(std::uint16_t electrodeID) const {
    return stimHeaders.at(electrodeID);
  }
  
  friend std::ostream& operator<<(std::ostream& out, const NEVFile& f);

  auto get_timestampFS()   const {return fsTimestamp; }
  auto get_waveformFS()    const {return fsWaveforms; }
  auto get_start_sys()     const {return origin;}
  auto get_start_proc()    const {return processorTime;}
  auto get_comment()       const {return comment;}
  auto get_creator()       const {return creator;}
  auto get_spike_label(std::uint16_t id) {return labels[id];}
  auto allWaves16Bit()         const {return flags&1; }
  auto get_digital_mode()      const {return digitalMode;}
 protected:
  std::ifstream file;

  // File format information
  std::uint8_t majorVersion;
  std::uint8_t minorVersion;

  // Flags (mostly uninteresting)
  std::uint16_t flags;

  // Size of header and data packets
  std::uint32_t headerSize;
  std::uint32_t packetSize;

  // Timing information
  std::uint32_t fsTimestamp;   // Clock rate for timestamps
  std::uint32_t fsWaveforms;   // Clock rate for snips 
  SystemTime origin;           // Wall time when recording started
  std::uint32_t processorTime; // Processor time when recording started(?)

  // Strings containing creator and content information
  std::string creator;
  std::string comment;

  bool hasDigitalEvents;       // True if there are digital events
  DigitalMode digitalMode;     // Serial or parallel mode?
  

  // Extended header information
  std::map<std::uint16_t, StimHeader> stimHeaders;    // Stimulation amplifers 
  std::map<std::uint16_t, SpikeHeader> spikeHeaders;  // Recording amplifiers
  std::map<std::uint16_t, SpikeFilter> spikeFilters;  // Filters applied to spiking data
  std::map<std::uint16_t, std::string> labels;



  std::shared_ptr<Packet> readPacketOrNull(bool digital=true, bool stim=true, bool spike=true);
  
 private:
  //Internal buffer stuff
  const size_t BUFFERSIZE;
  uint8_t* buffer;
  size_t buffer_capacity;
  size_t buffer_pos;

  
  std::uint32_t readBasicHeader();
  void readExtendedHeaders(const std::uint32_t nHeaders);
  void refillBuffer();

  std::shared_ptr<DigitalPacket> parseCurrentAsDigital();
  std::shared_ptr<SpikePacket>   parseCurrentAsSpike();
  std::shared_ptr<StimPacket>    parseCurrentAsStim();
  
};

#endif
