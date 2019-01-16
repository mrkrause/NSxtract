//
//  nsx2txt.cpp
//  
//
//  Created by Matthew Krause on 7/2/15.
//
//
#include "NSxFile.h"
#include "NSxConfig.h"
#include <fstream>

void NSxFile::writeTxtHeader(const NSxConfig& config) {

    std::string filename = config.textHeaderFilename();
    std::ofstream txtfile(filename, std::ofstream::out | std::ofstream::trunc);
    
    if(!txtfile.is_open())
        throw(std::runtime_error("Cannot open text header file " + filename + " for writing"));
    
    
    txtfile << "Ripple NSx --> FLAC conversion" << std::endl;
    txtfile << "NSx Version: " << (int)header.getMajorVersion() << '.' << (int)header.getMinorVersion() << std::endl;
    
    txtfile << "File Label: "  << header.getLabel() << std::endl;
    txtfile << "File Comment: "  << header.getComment() << std::endl;
    
    txtfile << "Data acquisition T0: " << header.getStartTime() << std::endl;
    txtfile << "Sampling period: " << header.getSamplingPeriod() << std::endl;
    txtfile << "Time resolution: " << header.getTimeResolution() << std::endl;
    txtfile << "= Sampling frequency: " << header.getSamplingFreq() << std::endl;
    
    for(auto chan_iter=channelBegin(); chan_iter!=channelEnd(); chan_iter++) {
        NSxChannel chan = *chan_iter;
        
        txtfile << "-------------------------------" << std::endl;
        txtfile << "Channel: " << chan.getNumericID() << " / "
        << chan.getRippleID() << " / " << chan.getLabel() << std::endl << std::endl;
        txtfile << "Filename: " << config.outputFilename(chan.getNumericID(), false) << std::endl;
        txtfile << "Front End: " << (int)chan.getFrontEnd() << std::endl;
        txtfile << "Pin: " << (int)chan.getPin() << std::endl << std::endl;
        txtfile << "Digital Range: " << chan.getDigitalMin() <<  " to " << chan.getDigitalMax() << std::endl;
        txtfile << "Analog Range: " << chan.getAnalogMin() << " to " << chan.getAnalogMax() << std::endl;
        txtfile << "Data in: " << chan.getUnits() << std::endl;
        txtfile << "= Scale factor: " << chan.getVoltsPerAD() << std::endl << std::endl;;
        txtfile << "Low-pass filter: " << chan.getLPFilter() << std::endl;
        txtfile << "High-pass filter: " << chan.getHPFilter() << std::endl;
        
    }
    
}
