#include "nev2plx_config.h"
#include <iostream>

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

Config::Config(void) :
  _valid(false),
  desc("Convert a Ripple NEV file to PLX") {
  
  desc.add_options()
    ("help", "Show this message")
    ("input-file,i",
     opts::value<std::string>(),
     "NEV file to convert")
    ("output-file,o",
     opts::value<std::string>()->default_value(""),
     "Output filename for PLX file")
    ("overwrite",
     opts::value<bool>()->default_value(false),
     "If true, overwrite file on output")
    ("buffer-size",
     opts::value<size_t>()->default_value(1000000),
     "Maximum number of packets to read at once")
    ;
  
  pos.add("input-file", 1);
  pos.add("output-file", 2);
}

std::string Config::get_input() const {
  if(_valid)
    return _input;
  else
    throw(std::runtime_error("Options not initalized"));
}

std::string Config::get_output() const {
  if (_valid)
    return _output;
  else
    throw(std::runtime_error("Options not initalized"));
}

size_t Config::get_buffer_sz() const {
  if (_valid)
    return _buffer_sz;
  else
    throw(std::runtime_error("Options not initalized"));
}

bool Config::get_overwrite() const {
  if(_valid)
    return _overwrite;
  else
    throw(std::runtime_error("Options not initalized"));
}

void Config::parse(int argc, char*argv[]) {
  opts::variables_map vm;
  
  opts::store(opts::command_line_parser(argc, argv).
	      options(desc).positional(pos).run(), vm);

  if(vm.count("help") || argc==1) {
    std::cout << desc << std::endl;
    exit(0);
  }
  
  opts::notify(vm);

  setInput(vm);
  setOutput(vm);

  _buffer_sz = vm["buffer-size"].as<size_t>();
  _overwrite = vm["overwrite"].as<bool>();

  _valid = true;
}

void Config::setInput(const opts::variables_map& vm) {
  
  if(!vm.count("input-file")) 
    throw(std::runtime_error("No input file or directory found!"));
  
  this->_input = vm["input-file"].as<std::string>();
  fs::path inputPath(_input);
  if(!(fs::is_regular_file(inputPath) || fs::is_symlink(inputPath))) {
    throw(std::runtime_error("The input file is not a regular file or directory."));    
  }
}


void Config::setOutput(const  opts::variables_map& vm) {
  std::string f = vm["output-file"].as<std::string>();
  bool overwrite = vm["overwrite"].as<bool>();

  if(f.empty()) {
    f = _input;
    auto start_at = f.find_last_of('.');
    f.replace(start_at, start_at+2, ".plx");
  }
  
  if(fs::exists(f) && !overwrite) {
    throw(std::runtime_error("Output file already exists--remove or enable overwriting"));
  }

  _output = f;
}

std::ostream& operator<<(std::ostream &out, const Config &c) {
  out << "NEV to PLX Conversation\n" 
      << "CAUTION: Scale factors are *probably* incorrect\n\n"
      << "\tInput: "  << c.get_input() << std::endl
      << "\tOutput: " << c.get_output() << std::endl
      << "\t\t Overwrite: " << c.get_overwrite()  << std::endl
      << "\tBuffer:  " << c.get_buffer_sz() << " packets\n";

  return out;
}
  
