//
//  MatFile.cpp
//  
//
//  Created by Matthew Krause on 6/16/15.
//
//
#include "MatFile.h"

MATFile::MATFile(const std::string& _filename, const std::string& _mode) {
    filename = _filename;
    
    mode = 0;
    if(_mode == "r") {
        mode = READABLE;
    } else if(_mode == "u") {
        mode = READABLE | WRITABLE;
    } else if (_mode == "w") {
        mode = WRITABLE;
    } else if (_mode == "w4") {
        mode = WRITABLE | V4_OR_OLDER;
    } else if (_mode == "wL") {
        mode = WRITABLE | LOCAL_VARCHAR;
    } else if (_mode == "wz") {
        mode = WRITABLE | COMPRESSED;
    } else if (_mode == "w7.3") {
        mode = WRITABLE | BIGVARS;
    } else {
        throw(std::runtime_error("Invalid I/O mode " + _mode));
    }

    mfp = MW::matOpen(filename.c_str(), _mode.c_str());
    if(!mfp) {
        throw(std::runtime_error("Unable to open file " + filename + " (mode: " + _mode  + ")"));
    }
}

MATFile::~MATFile() {
    int result = MW::matClose(mfp);
}

void MATFile::close() {
    mode = 0; // prevent reading AND writing!
    MW::matClose(mfp);
}

void MATFile::rmVar(const std::string& varname) {
    if(!(isReadable() && isWritable()))
        throw(std::runtime_error("File " + filename + " must be readable and writable to remove variables (open with mode \"u\"."));
    
    MW::matDeleteVariable(mfp, varname.c_str());
}

FILE* MATFile::getFP() {
    /* Returns the underlying file pointer. Be careful, because the file is
     presumably closed when this object is destroyed. */
    return MW::matGetFp(mfp);
}

std::vector<std::string> MATFile::getDir() {
    int n;
    char** names = MW::matGetDir(mfp, &n);
    
    std::vector<std::string> v;
    v.reserve(n);
    
    for(int i=0; i<n; i++) {
        v.push_back(names[i]);
    }
    MW::mxFree(names);
    
    return v;
}

MW::mxArray* MATFile::rawGetVar(const std::string& varname) {
    /* Returns a pointer to the mxArray object associated with varname, or
     returns a null pointer if the variable does not exist in this file.
     
     *YOU* are responsible for deleting this later, using mxDestroyArray().
     
     See the Mathworks docs (matGetVariable) for additional details */
    
    if(!isReadable())
        throw(std::runtime_error("File " + filename + " is was not opened for reading (or has been closed)"));
    
    MW::mxArray* ptr = MW::matGetVariable(mfp, varname.c_str());
    return ptr;
}

MW::mxArray* MATFile::rawGetInfo(const std::string& varname) {
    /* Returns a pointer to the mxArray object's header associated with varname, or
     returns a null pointer if the variable does not exist in this file.
     
     *YOU* are responsible for deleting this later, using mxDestroyArray().
     
     See the Mathworks docs (matGetVariableInfo) for details */
    if(!isReadable())
        throw(std::runtime_error("File " + filename + " is was not opened for reading (or has been closed)"));
    
    MW::mxArray* ptr = MW::matGetVariableInfo(mfp, varname.c_str());
    return ptr;
}


template <>
void MATFile::putScalar<>(const std::string& varname, MW::mxArray* value, bool asGlobal) {
    if(!isWritable())
        throw(std::runtime_error("File " + filename + " is was not opened for writing (or has been closed)"));
    
    int status;
    
    if(asGlobal) {
        status = MW::matPutVariableAsGlobal(mfp, varname.c_str(), value);
    } else {
        status = MW::matPutVariable(mfp, varname.c_str(), value);
    }
    
    if(status) {
        throw(std::runtime_error("Unable to write variable " + varname + " to file " + filename + "."));
    }
}

template <>
void MATFile::putScalar<const std::string>(const std::string& varname, const std::string value, bool asGlobal) {
    MW::mxArray* newval = MW::mxCreateString(value.c_str());
    check_put_and_dealloc(varname, newval, asGlobal);
}

template <>
void MATFile::putScalar<const std::string&>(const std::string& varname, const std::string& value, bool asGlobal) {
    MW::mxArray* newval = MW::mxCreateString(value.c_str());
    check_put_and_dealloc(varname, newval, asGlobal);
}

template <>
void MATFile::putScalar<const char*>(const std::string& varname, const char* value, bool asGlobal) {
    MW::mxArray* newval = MW::mxCreateString(value);
    check_put_and_dealloc(varname, newval, asGlobal);
}

template <>
void MATFile::putScalar<bool>(const std::string& varname, const bool value, bool asGlobal) {
    MW::mxArray* newval = MW::mxCreateLogicalScalar(value);
    check_put_and_dealloc(varname, newval, asGlobal);
}

template <>
void MATFile::putScalar<double>(const std::string& varname, const double value, bool asGlobal) {
    MW::mxArray* newval = MW::mxCreateDoubleScalar(value);
    check_put_and_dealloc(varname, newval, asGlobal);
}


void MATFile::check_put_and_dealloc(const std::string& varname, MW::mxArray* newval, bool asGlobal) {
    /* This is a silly little wrapper around putScalar<MW::mxArray*> that
     - checks whether the mxArray is NULL (and throws if it is),
     - attempts to call putScalar<MW::mxArray*>, and
     - cleans up appropriately.
     
     Use this inside functions that create some sort of mxArray themselves. Use 
     putScalar<MW::mxArray*> for user-provided mxArray*, since cleaning those up is the user's
     responsibility. 
     */
    
    if(!newval) {
        std::runtime_error("Out of memory! (Failed to allocate an mxArray for new data)");
    }

    try {
        putScalar<MW::mxArray*>(varname, newval, asGlobal);
    } catch (...) {
        MW::mxDestroyArray(newval);
        throw;
    }

    MW::mxDestroyArray(newval);
}


