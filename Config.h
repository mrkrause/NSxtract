#ifndef __CONVERT_CONFIG__
#define __CONVERT_CONFIG__

#include <exception>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

class Config {

 public:
    Config();
    void parse(int argc, char* argv[]);

    std::string inputFile(void) const;
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
    
 private:
    bool valid;
    opts::positional_options_description pos;
    opts::options_description desc;

    std::string _inputFile;
    std::string _outputDir;
    std::string _outputPrefix;
  
    fs::path outputPath;
    unsigned _nThreads;
    unsigned _readSize;
    unsigned _flacCompression;
    
    bool     _matlabHeader;
    bool     _textHeader;
    bool     _compressData;
    
    std::string getInputFilename(const opts::variables_map& vm);
    std::string getOutputDir(const opts::variables_map& vm);
};


#endif /* __CONVERT_CONFIG__*/
