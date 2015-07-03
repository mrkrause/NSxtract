/* NSxFile: Class for interacting with Ripple/Trellis NSx (continuous
   data) files. Since these files are often huge (10s-100s of GB),
   this mostly supports reading blocks of samples from the file into
   memory

   See also: NSxHeader (contains information about the whole file), and
             NSxChannel (contains information about each channel)
*/

#ifndef __NSxFile__
#define __NSxFile__

#include <limits>
#include <stdexcept>
#include <vector>

#include "NSxHeader.h"
#include "NSxChannel.h"

#include "Config.h"
#ifdef MAT_FILE_SUPPORT
#include "MatFile.h"
#endif

class NSxFile {
public:
    NSxFile(const std::string& filename);
    
    size_t readData(std::uint32_t nSamples, int16_t* &buffer);
    bool hasMoreData() const { return dataAvailable; }
    
    NSxFile(const NSxFile &rhs) = delete;
    NSxFile& operator=(NSxFile & rhs) = delete;
    
    // Access to the header
#ifdef MAT_FILE_SUPPORT
    void writeMatHeader(const Config &c);
#endif
    void writeTxtHeader(const Config &c);
    
    std::uint32_t getChannelCount() { return header.getChannelCount(); }
    double getSamplingFreq() { return header.getSamplingFreq(); }
    
    //Iterate over the channels
    std::vector<NSxChannel>::const_iterator channelBegin() const;
    std::vector<NSxChannel>::const_iterator channelEnd() const;

protected:
    NSxHeader header;
    std::vector<NSxChannel> channels;

    std::ifstream file;
    
    void prepareNextPacket();
    std::uint32_t currentPacket;
    std::uint32_t samplesRemainingInPacket;
    bool dataAvailable;
    
    std::uint32_t basetime;
private:
    
    
};

#endif /* defined(_NSxFile__) */
