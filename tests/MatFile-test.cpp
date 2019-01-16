/* Simple tests for MatFile*/

#include <cstdint>
#include <iostream>

#include "MatFile.h"

int main() {
    std::cout << "Starting" << std::endl;
    /* Create a new MAT file */
    MATFile m("test.mat", "w");
    
    m.putScalar("greeting", "hello, world");
    
    m.putScalar("tiny_unsigned", static_cast<std::uint8_t>(1));
    m.putScalar("tiny_signed", static_cast<std::int8_t>(-2));
    
    m.putScalar("small_unsigned", static_cast<std::uint16_t>(3));
    m.putScalar("small_signed", static_cast<std::int16_t>(-4));
    
    m.putScalar("regular_unsigned", static_cast<std::uint32_t>(5));
    m.putScalar("regular_signed", static_cast<std::int32_t>(-6));
    
    m.putScalar("long_unsigned", static_cast<std::uint64_t>(7));
    m.putScalar("long_signed", static_cast<std::int64_t>(-8));
    
    m.putScalar("single_precision", static_cast<float>(3.141));
    m.putScalar("double_precision", static_cast<double>(3.141));
    
    double d[8] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 };
    MW::mwSize ddims[3] = {2,2,2};
    m.putArray("vec", d, 3, ddims, true);
    
    std::uint8_t u8[8] = {1, 2, 3, 4, 5, 6, 7, 8 };
    MW::mwSize u8dims[3] = {2,4,1};
    m.putArray("vec_tiny", u8, 2, u8dims, true);
    
    /* Structs are a little more complicated and require using the MW:: stuff still*/
    MW::mwSize scalar[2]= {2,1};
    const char* fieldnames[] = {"foo", "bar"};
    const char* f2[] = {"baz", "florp"};
    
    MW::mxArray* s1 = MW::mxCreateStructArray(2, scalar, 2, fieldnames);
    MW::mxArray* s2 = MW::mxCreateStructArray(2, scalar, 2, f2);
    
    MW::mxSetField(s1, 0, "foo", MW::mxCreateDoubleScalar(24601));
    MW::mxSetField(s1, 0, "bar", MW::mxCreateDoubleScalar(1138));
    MW::mxSetField(s1, 1, "foo", MW::mxCreateDoubleScalar(76));
    MW::mxSetField(s1, 1, "bar", MW::mxCreateDoubleScalar(42));
    
    m.putScalar("inner", s1);
    


    mxSetField(s2, 0, "baz", s1);
    mxSetField(s2, 0, "florp", MW::mxCreateLogicalScalar(true));
    
    m.putScalar("outer", s2);
    
    // MW::mxDestroyArray(s1); //Don't do this! Destroying s2 cleans up S1 as well
    MW::mxDestroyArray(s2);
    
    std::cout << 0.0/0.0 << "Finished successfully!" << std::endl;
    return 0;
}
