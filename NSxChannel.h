#ifndef ___NSxChannel_h__
#define ___NSxChannel_h__

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>


enum FilterType : std::uint16_t {NONE = 0, BUTTERWORTH = 1, CHEBYSHEV = 2};

#pragma pack(push,1)
struct Filter {
    std::uint32_t cornerFreq;
    std::uint32_t order;
    FilterType type;
};
#pragma pack(pop)


class NSxChannel {
    
public:
    NSxChannel();
    NSxChannel(std::ifstream &file);
    
    std::string rippleSpec() const;
    std::uint16_t getNumericID() const {return electrodeID;}

    friend std::ostream& operator<<(std::ostream& out, const NSxChannel& c);
protected:
    std::uint16_t electrodeID;
    char electrodeLabel[16];
    char unitLabel[16];
    
    std::uint8_t frontEndID;
    std::uint8_t pin;
    
    std::int16_t minDigital;
    std::int16_t maxDigital;
    
    std::int16_t minAnalog;
    std::int16_t maxAnalog;
    
    Filter lowpass;
    Filter highpass;
    
};
#endif /* ___NSxChannel_h__ */
