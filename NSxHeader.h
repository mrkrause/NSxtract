/* NSxHeader: This class stores/represents the NSx Basic Header
from Ripple's Trellis/NEV data format. It stores information about
the entire recording (sampling rate, time zero for the timestamps, etc).

The entire spec is on page 8 of NEVspec_2_2.pdf, available in /opt/Trellis

See also: NSxChannel.h, which represents individual channels.
*/



#ifndef __NSxHeader__
#define __NSxHeader__

#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdint>

#include "systemtime.h"


class NSxHeader {

public:
    NSxHeader() {}
    NSxHeader(std::ifstream& file);
    friend std::ostream& operator<<(std::ostream& out, const NSxHeader& h);
    
    double getSamplingFreq() const;
    double getVersion() const;
    
    std::uint32_t getChannelCount() const { return channelCount;}
    std::uint32_t getOffset() const { return offset;}

    std::uint32_t getSamplingPeriod() const { return samplingPeriod;}
    std::uint32_t getTimeResolution() const { return timeResolution ;}

    std::uint8_t getMajorVersion() const {  return majorVersion; }
    std::uint8_t getMinorVersion() const {  return minorVersion; }
    
    std::string getLabel() const { return std::string(label); }
    std::string getComment() const { return std::string(comment); }
    
    SystemTime getStartTime() const { return time; }
    
    
private:
    // Version numbers
    std::uint8_t majorVersion;
    std::uint8_t minorVersion;
    
    // User data showing the type of data acquired in this file (label)
    // and some free-form comments. I *think* these are required to be null-terminated
    char label[16];
    char comment[256];

    // Time data acquisition started. Other timestamps are relative to this
    SystemTime time;
    

    
    std::uint32_t samplingPeriod;
    std::uint32_t timeResolution;
    
    std::uint32_t channelCount;
    std::uint32_t offset;
};


#endif /* defined(__NSxHeader__) */
