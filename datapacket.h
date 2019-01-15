#ifndef DATAPACKET_H_INCLUDED
#define DATAPACKET_H_INCLUDED

#include <cstdint>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>

/* This file contains structs describing the "packets" generated in response to events.
   In particular, you may be interested in:
     - DigitalPacket: changes in any of the digital inputs (parallel port + SMA connectors)
     - SpikePacket:   timestamp, channel, online sort, and waveform of a spike
     - StimPacket:    timestamp, channel, and waveform of a microstimulation pulse.

   NOTES:
     - These could be classes, but there's no point in wrapping everything up with accessors.
       There are destructors and stuff so memory is somewhat managed, but since everything
       is public, feel free to shoot your own foot if you'd like....

     - Waveforms are all in BYTES. See the NEVFile headers to determine how to assemble them 
       into arbitrary units (1, 2, or 3 bytes per sample) and then physical units (e.g., volts)

     - Some (untested) code reassembles the continuation packets and appends them.
*/


struct Packet {
  std::uint32_t timestamp;
  virtual ~Packet() { }
};

struct WavePacket : Packet {
  char* waveform;
  std::size_t len;
  std::uint16_t electrodeID;
  
  WavePacket();
  WavePacket(const WavePacket& rhs);
  WavePacket(WavePacket&& rhs);
  WavePacket& operator=(const WavePacket& rhs);
  WavePacket& operator=(WavePacket&& rhs) noexcept;
  ~WavePacket();

  
 private:
  
};


struct SpikePacket : WavePacket {
  std::uint8_t  unit;

  SpikePacket();
  SpikePacket(const SpikePacket& rhs);
  SpikePacket(SpikePacket&& rhs);  
  SpikePacket& operator=(const SpikePacket &rhs);
  SpikePacket& operator=(SpikePacket &&rhs);
  
  friend std::ostream& operator<<(std::ostream& out, const SpikePacket &p);
};


struct StimPacket : WavePacket {


  StimPacket();
  StimPacket(const StimPacket& rhs);
  StimPacket(StimPacket&& rhs);
  StimPacket& operator=(const StimPacket &rhs);
  StimPacket& operator=(StimPacket&& rhs);
    
  friend std::ostream& operator<<(std::ostream& out, const StimPacket &p);


  
};


/***********************************************************************
                    DIGITAL I/O Packets and Types
***********************************************************************/

enum DigitalReason : std::uint8_t {
  PARALLEL =  1,   // Parallel port changed *or* serial was strobed
  SMA1 = 2,        // SMA 1 changed 
  SMA2 = 4,        // SMA 2 changed
  SMA3 = 8,        // SMA 3 changed
  SMA4 = 16,       // SMA 4 changed
                   // Per the spec, there is no bit five.
  PERIODIC = 64,   // A periodic sampling event occured
  SERIAL = 128     // Serial channel changed (see bit 0)
 };
    

struct DigitalPacket : Packet {
  DigitalReason reason;
  std::uint16_t parallel;
  std::int16_t SMA1;
  std::int16_t SMA2;
  std::int16_t SMA3;
  std::int16_t SMA4;

  ~DigitalPacket() { }; //Nothing to do
  friend std::ostream& operator<<(std::ostream &out, const DigitalPacket &p);
};

#endif
