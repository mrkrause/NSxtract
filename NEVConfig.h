#ifndef NEVCONFIG_H_INCLUDED
#define NEVCONFIG_H_INCLUDED

#include <cstddef>
#include <exception>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

class NEVConfig;
typedef std::vector<NEVConfig> NEVWorkQueue;

enum OutputFormat {
  TEXT   = 0,
  CSV    = 1,
  MATLAB = 2,
};
std::ostream& operator<<(std::ostream &out, OutputFormat of);
    

    

class NEVConfig {

 public:
    NEVConfig();
    void parse(int argc, char* argv[]);

    std::string input(void) const;          // Filename or directory                                       
    std::string outputPrefix(void) const;   // Start files with this string

    // EVENTS FILES
    std::vector<OutputFormat> eventFileTypes(void) const;
    std::string eventFilename(const OutputFormat type) const;

    // STIMULATION FILES
    std::vector<OutputFormat> stimFileTypes(void) const;
    std::string stimFilename(const OutputFormat type) const;
    bool includeStimWaves(void) const { return _stimWaves;}

    // SPIKE FILES
    std::vector<OutputFormat> spikeFileTypes(void) const;
    std::string spikeFilename(const OutputFormat type) const;
    bool includeSpikeWaves(void) const { return _spikeWaves;}
    
    
    size_t bufferSize() const;          

    bool valid(void) const { return(_valid); }
    bool isSingleFileConfig(void) const { return(_singleFile); }

    friend std::ostream& operator<<(std::ostream &out, const NEVConfig &c);
    NEVWorkQueue toWorkQueue();

    std::vector<OutputFormat> parseFormatString(const std::string &s);

    
 private:
    bool _valid;
    opts::positional_options_description pos;
    opts::options_description desc;

    std::string _input;
    bool _singleFile;
    std::string _outputPrefix;
    fs::path outputPath;

    std::map<OutputFormat, std::string> exts;

    std::vector<OutputFormat> _eventFileTypes;

    std::vector<OutputFormat> _stimFileTypes;
    bool _stimWaves;
    
    std::vector<OutputFormat> _spikeFileTypes;
    bool _spikeWaves;
    
    size_t _bufferSize;

    void setInput(const opts::variables_map& vm);


    
};


#endif /* __CONVERT_CONFIG__*/
