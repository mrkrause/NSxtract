#pragma once
#ifndef SYSTEMTIME_H_INCLUDED
#define SYSTEMTIME_H_INCLUDED

#include <sstream>
#include <iomanip>

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
      char old_fill =  out.fill('0');
      out << std::setw(4) << t.year << '-' << std::setw(2) << t.month  << '-' << std::setw(2) << t.day << ' '
	  << std::setw(2) << t.hour << ':' << std::setw(2) << t.minute << ':' << std::setw(2) << t.second << '.'
	  << std::setw(3) << t.millisecond << 'Z';
      out.fill(old_fill);
      return out;
    }
};
#pragma pack(pop)

#endif
