#ifndef ___NSxChannel_h__
#define ___NSxChannel_h__

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>

#include "filter.h"

class NSxChannel {
    
public:
    NSxChannel();
    NSxChannel(std::ifstream &file);
    
    /* Getters for labels */
    std::string   getLabel() const { return std::string(electrodeLabel);}
    std::string   getUnits() const { return std::string(unitLabel);}
    
    std::string   getRippleID() const;
    std::uint16_t getNumericID() const {return electrodeID;}

    std::uint8_t  getFrontEnd() const {return frontEndID; }
    std::uint8_t  getPin() const {return pin; }
    
    /* Getters for A/D properties*/
    std::int16_t getDigitalMin() const {return minDigital; }
    std::int16_t getDigitalMax() const {return maxDigital; }
    
    std::int16_t getAnalogMin() const {return minAnalog; }
    std::int16_t getAnalogMax() const {return maxAnalog; }
    
    double getVoltsPerAD() const;
    
    /* Getters for Filters*/
    Filter getLPFilter() const { return lowpass;}
    Filter getHPFilter() const { return highpass;}
    
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
