//
//  NSxChannel.cpp
//  dumpNEV
//
//  Created by Matthew Krause on 2/23/15.
//  Copyright (c) 2015 Matthew Krause. All rights reserved.
//

#include "NSxChannel.h"

NSxChannel::NSxChannel(std::ifstream &file) {
    std::streampos start= file.tellg();
    
    try {
        char buffer[2];
        file.read(buffer, 2);
        if((buffer[0] != 'C') || (buffer[1] != 'C')) {
            std::cerr << "Failed on filetype " <<  int(buffer[0]) << int(buffer[1]) <<  "@" << file.tellg() << std::endl;
            throw(std::runtime_error("Incorrect file type; expected a Ripple .ns5 (or other .nsX) file"));
        }
        
        file.read(reinterpret_cast<char *>(&electrodeID), sizeof(electrodeID));
        file.read(reinterpret_cast<char *>(&electrodeLabel), sizeof(electrodeLabel));
        file.read(reinterpret_cast<char *>(&frontEndID), sizeof(frontEndID));
        file.read(reinterpret_cast<char *>(&pin), sizeof(pin));
        file.read(reinterpret_cast<char *>(&minDigital), sizeof(minDigital));
        file.read(reinterpret_cast<char *>(&maxDigital), sizeof(maxDigital));
        file.read(reinterpret_cast<char *>(&minAnalog), sizeof(minAnalog));
        file.read(reinterpret_cast<char *>(&maxAnalog), sizeof(maxAnalog));
        file.read(reinterpret_cast<char *>(&unitLabel), sizeof(unitLabel));
        file.read(reinterpret_cast<char *>(&highpass), sizeof(highpass));
        file.read(reinterpret_cast<char *>(&lowpass), sizeof(lowpass));
    }
    
    catch (...) {
        std::cerr << "Error at " << file.tellg() << std::endl;
        file.seekg(start);
        throw;
    }
}

double NSxChannel::getVoltsPerAD() const {
    return (double(maxAnalog) -  double(minAnalog)) / (double(maxDigital) - double(minDigital));
}

std::string NSxChannel::getRippleID() const {
    std::ostringstream s;
    
    char port = (frontEndID/4) + 'A';
    unsigned frontEnd = unsigned(frontEndID % 4) + 1;
    
    s << char(port) << '-' << frontEnd <<  '-' << int(pin);
    return(s.str());
}


std::ostream& operator<<(std::ostream& out, const NSxChannel& c) {
    out  << "Electrode #" << c.electrodeID << " (" << c.electrodeLabel << "): " << c.getRippleID() << std::endl;
    return out;
}
