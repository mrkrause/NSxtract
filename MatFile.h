//
//  MatFile.h
//  
//
//  Created by Matthew Krause on 6/16/15.
//
//

#ifndef ____MatFile__
#define ____MatFile__

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

namespace MW {
    // These are the original header files from the MathWorks.
    #include "mat.h"
    #include "matrix.h"
    const mwSize SCALAR_SIZE[2] = { static_cast<mwSize>(1), static_cast<mwSize>(1)};
}

#include "typeHelper.h"


class MATFile {
    
    /* Describes some properties of the MAT file. Note that these are only properties that
     were explicitly requested. If you open the file with 'u', for example, writes will mimic the
     locale, compression, and version information of the existing data without setting these flags. */
    enum MATFILE_MODE {
        READABLE = 1,
        WRITABLE = 2,
        V4_OR_OLDER = 4,   //Opened with w4, which allows saving for very old versions of Matlab (<4)
        LOCAL_VARCHAR = 8, //Opened with wL, which uses current locale instead of unicode for variable names
        COMPRESSED = 16,   //Opened with wz, which compresses the data (a good idea!)
        BIGVARS = 32       //Opened with w7.3, which uses the newer HDF format that can store variables > 2 Gb
    };

public:
    MATFile(const std::string& _filename, const std::string& _mode);
    ~MATFile();
  
    FILE * getFP();
    void close(void);
    
    bool isReadable() const   { return(mode & READABLE); }
    bool isWritable() const  { return(mode & WRITABLE); }
    
    MW::mxArray* rawGetVar(const std::string& varname);
    MW::mxArray* rawGetInfo(const std::string& varname);

    

    template <typename Scalar>
    void putScalar(const std::string& varname, const Scalar value, bool asGlobal = false) {
        
        const MW::mwSize scalar[2] = {static_cast<MW::mwSize>(1), static_cast<MW::mwSize>(1)};
        MW::mxArray* newval = MW::mxCreateNumericArray(static_cast<MW::mwSize>(2),
                                                       scalar,
                                                       typeHelper<Scalar>(),
                                                       static_cast<MW::mxComplexity>(false));        
        if(newval) {
            Scalar* data = static_cast<Scalar*>(MW::mxGetData(newval));
            data[0] = value;
        }
        check_put_and_dealloc(varname, newval, asGlobal);
    }
    
    template <typename Scalar>
    void putArray(const std::string& varname, const Scalar* values, MW::mwSize ndims, const MW::mwSize* dims, bool asGlobal = false) {
        MW::mxArray* newval = MW::mxCreateNumericArray(ndims,
                                                       dims,
                                                       typeHelper<Scalar>(),
                                                       static_cast<MW::mxComplexity>(false));
        MW::mwSize n =1;
        for(MW::mwSize i=0; i < ndims; i++) {
            n*= dims[i];
        }
        
        if(newval) {
            Scalar* data = static_cast<Scalar*>(MW::mxGetData(newval));
            for(MW::mwSize i=0; i < n; i++) {
                data[i] = values[i];
            }
        }
        
        check_put_and_dealloc(varname, newval, asGlobal);
    }
   
    void rmVar(const std::string& varname);
    
    std::vector<std::string> getDir();
    

protected:
    MW::MATFile *mfp;
        
    int mode;
    std::string filename;
    
    void  check_put_and_dealloc(const std::string& varname, MW::mxArray* newval, bool asGlobal);
};

/* Specializations for MATFile::putScalar() */
template <>
void MATFile::putScalar<bool>(const std::string& varname, const bool value, bool asGlobal);

template <>
void MATFile::putScalar<const char*>(const std::string& varname, const char* value, bool asGlobal);

template <>
void MATFile::putScalar<double>(const std::string& varname, const double value, bool asGlobal);

template <>
void MATFile::putScalar<std::string>(const std::string& varname, const std::string value, bool asGlobal);


template <>
void MATFile::putScalar<>(const std::string& varname, MW::mxArray* value, bool asGlobal);

template <>
void MATFile::putScalar<const std::string&>(const std::string& varname, const std::string& value, bool asGlobal);



#endif /* defined(____MatFile__) */
