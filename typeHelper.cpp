//
//  typeHelper.cpp
//  
//
//  Created by Matthew Krause on 6/21/15.
//
//

#include "typeHelper.h"


template <>
MW::mxClassID typeHelper<std::uint8_t>()  {
    return(MW::mxUINT8_CLASS);
}

template <>
MW::mxClassID typeHelper<std::int8_t>()  {
    return(MW::mxINT8_CLASS);
}

template <>
MW::mxClassID typeHelper<std::uint16_t>()  {
    return(MW::mxUINT16_CLASS);
}

template <>
MW::mxClassID typeHelper<std::int16_t>()  {
    return(MW::mxINT16_CLASS);
}

template <>
MW::mxClassID typeHelper<std::uint32_t>()  {
    return(MW::mxUINT32_CLASS);
}

template <>
MW::mxClassID typeHelper<std::int32_t>()  {
    return(MW::mxINT32_CLASS);
}

template <>
MW::mxClassID typeHelper<std::uint64_t>()  {
    return(MW::mxUINT64_CLASS);
}

template <>
MW::mxClassID typeHelper<std::int64_t>()  {
    return(MW::mxINT64_CLASS);
}

template <>
MW::mxClassID typeHelper<float>()  {
    return(MW::mxSINGLE_CLASS);
}

template <>
MW::mxClassID typeHelper<double>() {
    return(MW::mxDOUBLE_CLASS);
}

template <>
MW::mxClassID typeHelper<char>() {
    return(MW::mxCHAR_CLASS);
}
