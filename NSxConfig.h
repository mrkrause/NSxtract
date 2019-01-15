#pragma once
#ifndef NSXCONFIG_H_INCLUDED
#define NSXCONFIG_H_INCLUDED

#include <exception>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <cstdint>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

class NSxConfig;
typedef std::vector<NSxConfig> WorkQueue;

class NSxConfig {

 public:
    NSxConfig();
    void parse(int argc, char* argv[]);

    std::string input(void) const;
    std::string outputDir(void) const;
    std::string outputPrefix(void) const;

    unsigned int nThreads(void) const;
    unsigned int readSize(void) const;
    unsigned int flacCompression(void) const;
  
    bool matlabHeader(void) const;
    bool textHeader(void) const;
    bool compressData(void) const;
    
    std::string outputFilename(std::uint16_t electrode, bool withPath=true) const;
    std::string matlabHeaderFilename() const;
    std::string textHeaderFilename() const;

    bool valid(void) const { return(_valid); }
    bool isSingleFileConfig(void) const { return(_singleFile); }

    friend std::ostream& operator<<(std::ostream &out, const NSxConfig &c);
    WorkQueue toWorkQueue();
    
 private:
    bool _valid;
    opts::positional_options_description pos;
    opts::options_description desc;

    std::string _input;
    bool _singleFile;
    std::string _outputDir;
    std::string _outputPrefix;

  
    fs::path outputPath;
    unsigned _nThreads;
    unsigned _readSize;
    unsigned _flacCompression;
    
    bool     _matlabHeader;
    bool     _textHeader;
    bool     _compressData;
    
    void setInput(const opts::variables_map& vm);
    void setOutputDir(const opts::variables_map& vm);
    void setOutputDir(const fs::path &p);
};


#endif /* __CONVERT_CONFIG__*/
