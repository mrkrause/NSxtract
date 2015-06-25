//
//  NsxHeader.cpp
//  dumpNEV
//
//  Created by Matthew Krause on 2/22/15.
//  Copyright (c) 2015 Matthew Krause. All rights reserved.
//

#include "NSxHeader.h"

NSxHeader::NSxHeader(std::ifstream& file) {
    
    // All NSx files start with the magic word "NEURALCD".
    // Check to make sure this one does too
    std::streampos start= file.tellg();
    
    try {
        std::string typeString = "NEURALCD";
        for(auto i = 0U; i<typeString.size(); i++) {
            char tmp;
            file >> tmp;
            if(tmp != typeString[i]) {                
	      throw(std::runtime_error("File is not a 'NEURALCD' file; expecting a Ripple .ns5 or similar"));
            }
        }
        
        // See page 8 of the Trellis NEV spec for sizes
        file.read(reinterpret_cast<char*>(&majorVersion), sizeof(majorVersion));
        file.read(reinterpret_cast<char*>(&minorVersion), sizeof(minorVersion));
        file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        file.read(reinterpret_cast<char*>(&label), sizeof(char)*16);
        file.read(reinterpret_cast<char*>(&comment), sizeof(char)*256);
        file.read(reinterpret_cast<char*>(&samplingPeriod), sizeof(samplingPeriod));
        file.read(reinterpret_cast<char*>(&timeResolution), sizeof(timeResolution));
        file.read(reinterpret_cast<char*>(&time), sizeof(time));
        file.read(reinterpret_cast<char*>(&channelCount), sizeof(channelCount));
    } catch (...) {
        file.seekg(start); //Rewind the file, as if this never happened
        throw;
    }

    /* We explicitly DO NOT CLOSE the file here, because the rest of the class
    is going to continue reading from the file */
}

std::ostream& operator<<(std::ostream& out, const NSxHeader& h) // output
{
    out << "NSx Header (file format " << int(h.majorVersion) << '.' << int(h.minorVersion) <<  ')' << std::endl;
    out << "Data collection began at " << h.time << std::endl;
    out << "Contents: " << h.label << ", beginning at offset " << int(h.offset) << std::endl;
    out << h.channelCount << " channels, sampled at " << h.getSamplingFreq() << "Hz. Time resolution " << h.timeResolution << " Hz." << std::endl;
    
    out << "Comments: " << h.comment << std::endl;
    
    return out;
}

std::ostream& operator<<(std::ostream& out, const SystemTime& t) {
    out << t.year << '-' << t.month << '-' << t.day << ' ' << t.hour << ':' << t.minute << ':' << t.second << '.' << t.millisecond;
    return out;
}

std::string SystemTime::str() {
    std::ostringstream s;
    
    s << this;
    return(s.str());
    
}

double NSxHeader::getSamplingFreq() const {
    return double(timeResolution)/double(samplingPeriod);
}

