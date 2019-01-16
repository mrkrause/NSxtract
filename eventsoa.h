#pragma once
#ifndef EVENTSOA_H_INCLUDED
#define EVENTSOA_H_INCLUDED

#include "datapacket.h"

/* This is a trivial class for converting a stream of digital packets 
   (an array of structs, if you will) into a struct of individual arrays that can be 
   written to a Matlab file. 

   It does absolutely nothing interesting. 
*/

struct EventSOA {
  /*
    This seems to work on GCC, but not clang++

  std::vector< typeof(DigitalPacket::timestamp) > ts;
  std::vector< typeof(DigitalPacket::reason) > reason;
  std::vector< typeof(DigitalPacket::parallel) > parallel;
  std::vector< typeof(DigitalPacket::SMA1) > sma1;
  std::vector< typeof(DigitalPacket::SMA2) > sma2;
  std::vector< typeof(DigitalPacket::SMA3) > sma3;
  std::vector< typeof(DigitalPacket::SMA4) > sma4;
  */

  std::vector<std::uint32_t> ts;
  std::vector<DigitalReason> reason;
  std::vector<std::uint16_t> parallel;
  std::vector<std::int16_t> sma1;
  std::vector<std::int16_t> sma2;
  std::vector<std::int16_t> sma3;
  std::vector<std::int16_t> sma4;
  
  
  EventSOA(size_t initial = 1000) {
    ts.reserve(initial);
    reason.reserve(initial);
    parallel.reserve(initial);
    sma1.reserve(initial);
    sma2.reserve(initial);
    sma3.reserve(initial);
    sma4.reserve(initial);
  }
  
  
  void addPacket(std::shared_ptr<DigitalPacket> p) {
    ts.push_back(p->timestamp);
    reason.push_back(p->reason);
    parallel.push_back(p->parallel);
    sma1.push_back(p->SMA1);
    sma2.push_back(p->SMA2);
    sma3.push_back(p->SMA3);
    sma4.push_back(p->SMA4);
  }
};

#endif
