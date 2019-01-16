//
//  nsx2mat.cpp
//  
//
//  Created by Matthew Krause on 6/22/15.
//
//

#include "NSxFile.h"
#include "NSxConfig.h"

MW::mxArray* filterToMxArray(const Filter &f);


void NSxFile::writeMatHeader(const NSxConfig& config) {
    
    MATFile m(config.matlabHeaderFilename(), "wz");
    
    m.putScalar("file_version_major", header.getMajorVersion());
    m.putScalar("file_version_minor", header.getMinorVersion());
    
    m.putScalar("samplingPeriod", header.getSamplingPeriod());
    m.putScalar("timeResolution", header.getTimeResolution());
    
    m.putScalar("comment", header.getComment().c_str());
    m.putScalar("label",   header.getLabel().c_str());

    std::string tstr = header.getStartTime().str();
    //std::cerr << "Starting time is " << tstr << std::endl;
    m.putScalar("start_time", header.getStartTime().str().c_str());
    
    m.putScalar("sampling_frequency", header.getSamplingFreq());
    
    
    /*Okay, now the channels (which are more complicated */
    const char* channel_fieldnames[] = {
        "number",     // 0
        "ripple_ID",  // 1
        "label",      // 2
        "units",      // 3
        "front_end",  // 4
        "pin",        // 5
        "min_digital_value",  // 6
        "max_digital_value",  // 7
        "min_analog_value",   // 8
        "max_analog_value",   // 9
        "lp_filter",          // 10
        "hp_filter" ,         // 11
        "d2a_scale_factor",   // 12
        "filename"            // 13
    };
    
    MW::mwSize channel_dims[2] = {
      static_cast<MW::mwSize>(header.getChannelCount()), 
      1};
    
    MW::mxArray* chandata = MW::mxCreateStructArray(2, channel_dims, 14, channel_fieldnames);
    if(!chandata) {
      throw(std::runtime_error("Could not initalize channel data"));
    }

    int index = 0;
    for(auto chan_iter=channelBegin(); chan_iter!=channelEnd(); chan_iter++, index++) {
        NSxChannel chan = *chan_iter;
                
        MW::mxSetFieldByNumber(chandata, index, 0,
                               MW::mxCreateDoubleScalar(static_cast<double>(chan.getNumericID())));
        MW::mxSetFieldByNumber(chandata, index, 1,
                               MW::mxCreateString(chan.getRippleID().c_str()));
        MW::mxSetFieldByNumber(chandata, index, 2,
                               MW::mxCreateString(chan.getLabel().c_str()));
        MW::mxSetFieldByNumber(chandata, index, 3,
                               MW::mxCreateString(chan.getUnits().c_str()));
        MW::mxSetFieldByNumber(chandata, index, 4,
                               MW::mxCreateDoubleScalar(static_cast<double>(chan.getFrontEnd())));
        MW::mxSetFieldByNumber(chandata, index, 5,
                               MW::mxCreateDoubleScalar(static_cast<double>(chan.getPin())));
        MW::mxSetFieldByNumber(chandata, index, 6,
                               MW::mxCreateDoubleScalar(static_cast<double>(chan.getDigitalMin())));
        MW::mxSetFieldByNumber(chandata, index, 7,
                               MW::mxCreateDoubleScalar(static_cast<double>(chan.getDigitalMax())));
        MW::mxSetFieldByNumber(chandata, index, 8,
                               MW::mxCreateDoubleScalar(static_cast<double>(chan.getAnalogMin())));
        MW::mxSetFieldByNumber(chandata, index, 9,
                               MW::mxCreateDoubleScalar(static_cast<double>(chan.getAnalogMax())));
        MW::mxSetFieldByNumber(chandata, index, 10,
                               filterToMxArray(chan.getLPFilter()));
        MW::mxSetFieldByNumber(chandata, index, 11,
                               filterToMxArray(chan.getHPFilter()));
        MW::mxSetFieldByNumber(chandata, index, 12,
                               MW::mxCreateDoubleScalar(chan.getVoltsPerAD()));
        MW::mxSetFieldByNumber(chandata, index, 13,
                               MW::mxCreateString(config.outputFilename(chan.getNumericID(), false).c_str()));
    }
    
    m.putScalar("channels", chandata);
    MW::mxDestroyArray(chandata); //This should clean up everything

}

MW::mxArray* filterToMxArray(const Filter &f) {
    static const char* FILTER_FIELDNAMES[] = {
        "filter_type",
        "corner_freq",
        "order"
    };
    const MW::mwSize N_FILTER_FIELDNAMES = 3;
    
    static const char* filter_types[] = {
        "NONE",
        "BUTTERWORTH",
        "CHEBYSHEV"
    };

    double corner_freq, order;
    if(f.type) {
        corner_freq  = static_cast<double>(f.cornerFreq)/1000; //Freq is stored as mHz in file, convert to Hz
        order = static_cast<double>(f.order);
    } else {
        corner_freq = std::numeric_limits<double>::quiet_NaN();
        order = 0.0 / 0.0;
    }
    
    MW::mxArray* filter_params = MW::mxCreateStructArray(2, MW::SCALAR_SIZE, N_FILTER_FIELDNAMES, FILTER_FIELDNAMES);
    
    MW::mxSetFieldByNumber(filter_params, 0, 0,
                           MW::mxCreateString(filter_types[f.type]));
    MW::mxSetFieldByNumber(filter_params, 0, 1,
                           MW::mxCreateDoubleScalar(corner_freq));
    MW::mxSetFieldByNumber(filter_params, 0, 2,
                           MW::mxCreateDoubleScalar(order));
    return filter_params;
}
