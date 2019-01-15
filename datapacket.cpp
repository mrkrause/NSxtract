#include "datapacket.h"
#include <cstring>
#include <algorithm>
#include <string>
#include <sstream>

/***********************************************************************
                    DIGITAL I/O Packets and Types
***********************************************************************/

WavePacket::WavePacket() : waveform(0), len(0) {
}

WavePacket::WavePacket(const WavePacket& rhs) {
  this->waveform = new char[rhs.len];
  std::copy(rhs.waveform, rhs.waveform + rhs.len, this->waveform);
  this->len = rhs.len;
  this->electrodeID = rhs.electrodeID;
}

WavePacket::WavePacket(WavePacket &&rhs) {
  delete [] this->waveform;
  this->waveform = rhs.waveform;
  this->len      = rhs.len;
  this->electrodeID = rhs.electrodeID;
  rhs.len = 0;
}

WavePacket& WavePacket::operator=(const WavePacket& rhs) {
  delete [] this->waveform;
  this->waveform = new char[rhs.len];
  std::copy(rhs.waveform, rhs.waveform + rhs.len, this->waveform);
  this->len = rhs.len;
  this->electrodeID = rhs.electrodeID;

  return *this;
}

WavePacket& WavePacket::operator=(WavePacket&& rhs) noexcept {
  delete [] this->waveform;
  this->waveform = rhs.waveform;
  rhs.waveform = nullptr;

  this->len = rhs.len;
  rhs.len = 0;

  this->electrodeID = rhs.electrodeID;
  return *this;
}

WavePacket::~WavePacket() {
  delete [] this->waveform;
}




  
  

/***********************************************************************
                           SpikePacket
***********************************************************************/

SpikePacket::SpikePacket() : WavePacket(), unit(0) {
}

SpikePacket::SpikePacket(const SpikePacket& rhs) : WavePacket(rhs),		    						   unit(rhs.unit) {
}

SpikePacket::SpikePacket(SpikePacket&& rhs) : WavePacket(rhs),
					      unit(rhs.unit) {
}

SpikePacket& SpikePacket::operator=(const SpikePacket & rhs) {
  WavePacket::operator=(rhs);
  unit = rhs.unit;
  return *this;
}


SpikePacket& SpikePacket::operator=(SpikePacket&& rhs) {
    WavePacket::operator=(rhs);
    unit = rhs.unit;
    return *this;
}


/***********************************************************************
                           StimPacket
***********************************************************************/

StimPacket::StimPacket() : WavePacket() {
}

StimPacket::StimPacket(const StimPacket& rhs) : WavePacket(rhs) {
}

StimPacket::StimPacket(StimPacket&& rhs) : WavePacket(rhs) {
}

StimPacket& StimPacket::operator=(const StimPacket & rhs) {
  WavePacket::operator=(rhs);
  electrodeID = rhs.electrodeID;
  return *this;
}


StimPacket& StimPacket::operator=(StimPacket&& rhs) {
    WavePacket::operator=(rhs);
    electrodeID = rhs.electrodeID;
    return *this;
}


/***********************************************************************
                    Output (mostly for debugging)
            (Note that these assume a 30 kHz sampling rate)
***********************************************************************/




std::ostream& operator<<(std::ostream& out, const DigitalPacket &p) {
  out << "Digital Event at t=" << p.timestamp/30000.0 << "\n"
      << "\t Reason: " << p.reason << "\n"
      << "\t Parallel Value: " << p.parallel << "\n"
      << "\t SMA Values: " << p.SMA1 << " " << p.SMA2 << " " << p.SMA3 << " " << p.SMA4 << "\n";
  return out;
}

std::ostream& operator<<(std::ostream& out, const SpikePacket &p) {
  out << "Spiking Event at t=" << p.timestamp/30000.0 << "\n"
      << "\t Unit ID: " << p.electrodeID << "." << (int)p.unit << "\n"
      << "\t Waveform: [";

  for(auto i=0; i<p.len; i++) {
    out << (int)p.waveform[i];
    if(i < (p.len - 1))
      out << ", ";
  }
  out << "]\n" << std::endl;
  return out;
}


std::ostream& operator<<(std::ostream& out, const StimPacket &p) {
  out << "µStim Event at t=" << p.timestamp/30000.0 << "\n"
      << "\t Electrode ID: " << p.electrodeID << "\n"
      << "\t Waveform: [";

  for(auto i=0; i<p.len; i++) {
    out << (int)p.waveform[i];
    if(i < (p.len - 1))
      out << ", ";
  }
  out << "]\n" << std::endl;
  return out;
}
  
  

