#pragma once
#ifndef SYSTEMTIME_H_INCLUDED
#define SYSTEMTIME_H_INCLUDED

#include <sstream>

// We're doing the lazy thing and reading a whole struct in at once.
// This depends critically on having sizeof(SystemTime)==16, so use
// #pragma pack to remove any alignment.
// This is a little brittle, but should work. 
#pragma pack(push,1)
struct SystemTime {
    std::uint16_t year;
    std::uint16_t month;
    std::uint16_t dayOfWeek;
    std::uint16_t day;
    std::uint16_t hour;
    std::uint16_t minute;
    std::uint16_t second;
    std::uint16_t millisecond;

    std::string str() {
      std::ostringstream s;
    
      s << *this;
      return(s.str());
    }
  
    friend std::ostream& operator<<(std::ostream& out, const SystemTime& t) {
      out << t.year << '-' << t.month << '-' << t.day << ' '
	  << t.hour << ':' << t.minute << ':' << t.second << '.' << t.millisecond;
      return out;
    }
};
#pragma pack(pop)

#endif
