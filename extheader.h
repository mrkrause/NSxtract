#pragma once
#ifndef EXTHEADER_INCLUDED
#define EXTHEADER_INCLUDED

/* This file contains structs that describe the Extended Headers found in Ripple NEV Files (version 2.2).
   See pages 5-6 of the documentation for more details about the actual format.

   The code here deviates from it in a few resepcts. First, we do not define structs for the labels
   (NEUEVLBL) or digital input front end (DIGLABEL) headers, since those are trivial:
      - The electrode ID --> label information fits nicely into a single std::map (or equivalent)
      - There is very little digital input informationa and the hardware only supports one digital input
        front end anyway.
   Therefore, these two events are just bundled into the main NEVFile class. 

   The other major difference concerns how we handle the NEUEVWAV headers. This one type can describe the
   properties of the stimulation and recording amplifiers, which is confusing. The code reads in a raw
   NEUEVWAV header, then converts it to either a StimHeader (for stimulation amplifiers) or SpikeHeader 
   (for recording spiking). These are descended from a common parent (Header). 

   We also convert units to volts in both cases
   
   Note that we're (trying) to read the entire NEUEVWAV and NEUEVFLT header in one go, then using
   reinterpret_cast<> to form it into a structure. This requires that structure layout matches the 
   on-disk layout bit-for-bit (i.e., no padding for alignment). You *must* use pragma pack or similar
   (or rewrite the reading code). Note that this does not apply to Header/SpikeHeader/StimHeader, since
   these are derived from NEUEVWAV.

*/



#include "filter.h"

#pragma pack(push, 1)
struct NEUEVWAV {
  /* This represents a generic NEUEVWAV Header. Parse this into a
     recording or stimulation header and store that instead.
  */
  std::uint16_t electrodeID;
  std::uint8_t  frontEndID;
  std::uint8_t  pin;
  std::uint16_t neuralScaleFactor;
  std::uint16_t energyThreshold;
  std::int16_t highThreshold;
  std::int16_t lowThreshold;
  std::uint8_t  nSorted;
  std::uint8_t  bytesPerSample;
  float stimScaleFactor;
 
};
#pragma pack(pop)


#pragma pack(push, 1)
struct SpikeFilter {
  std::uint16_t electrodeID;
  Filter HPFilter;
  Filter LPFilter;
};
#pragma pack(pop)


struct Header {
  std::uint16_t electrodeID;
  std::uint8_t frontEndID;
  std::uint8_t pin;
  std::uint8_t  bytesPerSample;
  float scaleFactor;

  Header(const NEUEVWAV &n):
    frontEndID(n.frontEndID),
    pin(n.pin) {

    if(n.bytesPerSample == 0)
       bytesPerSample = 1;
    else
      bytesPerSample = n.bytesPerSample;
  }

  Header()  {}
};


struct SpikeHeader : Header{
  std::uint16_t energyThreshold;
  std::int16_t  highThreshold;
  std::int16_t  lowThreshold;
  std::uint8_t  nSorted;

 SpikeHeader(const NEUEVWAV &n) :
   
    energyThreshold(n.energyThreshold),
    highThreshold(n.highThreshold),
    lowThreshold(n.lowThreshold),
    nSorted(n.nSorted) {
      Header::electrodeID = n.electrodeID;
      Header::scaleFactor = static_cast<float>(n.neuralScaleFactor) * 1e9F; 
    }   
};


struct StimHeader: Header {

 StimHeader(const NEUEVWAV &n) {
    
   Header::electrodeID = n.electrodeID - 5120;
   
    if(n.bytesPerSample==0)
      Header::bytesPerSample = 1; //Per spec, zero OR one indicates 1 byte per sample
    else
      Header::bytesPerSample = n.bytesPerSample;
    
    if(n.stimScaleFactor!=0)
      Header::scaleFactor = n.stimScaleFactor;
    else
      Header::scaleFactor = (float) n.neuralScaleFactor;
    }	  
};  


std::ostream& operator<<(std::ostream& out, const NEUEVWAV& n);
std::ostream& operator<<(std::ostream& out, const SpikeFilter& sf);
std::ostream& operator<<(std::ostream& out, const StimHeader& sh);
std::ostream& operator<<(std::ostream& out, const SpikeHeader& n);


#endif
