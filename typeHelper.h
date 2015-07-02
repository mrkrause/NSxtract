//
//  typeHelper.h
//  
//
//  Created by Matthew Krause on 6/21/15.
//
//

#ifndef _typeHelper_h
#define _typeHelper_h

#include <string>
#include <exception>
#include <iostream>
#include <cstdint>

namespace MW {
    // These are the original header files from the MathWorks.
#include "mat.h"
#include "matrix.h"
};



template<typename T>
MW::mxClassID typeHelper() {
    throw(std::logic_error("Unknown class of type" + std::string(typeid(T).name())));
    return (MW::mxUNKNOWN_CLASS); //Obviously can't reach this...
}

template <>
MW::mxClassID typeHelper<std::uint8_t>();

template <>
MW::mxClassID typeHelper<std::int8_t>();

template <>
MW::mxClassID typeHelper<std::uint16_t>();

template <>
MW::mxClassID typeHelper<std::int16_t>();

template <>
MW::mxClassID typeHelper<std::uint32_t>();

template <>
MW::mxClassID typeHelper<std::int32_t>();

template <>
MW::mxClassID typeHelper<std::uint64_t>();

template <>
MW::mxClassID typeHelper<std::int64_t>();

template <>
MW::mxClassID typeHelper<float>();

template <>
MW::mxClassID typeHelper<double>();


#endif
