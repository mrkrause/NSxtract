#pragma once
#ifndef NEV2PLX_CONFIG_H_INCLUDED
#define NEV2PLX_CONFIG_H_INCLUDED

#include <ostream>
#include <string>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

class Config {
public:
  Config(void);
  std::string get_input(void) const; 
  std::string get_output(void) const; 
  bool get_overwrite(void) const;
  size_t get_buffer_sz(void) const;

  void parse(int argc, char** argv);
  friend std::ostream& operator<<(std::ostream &out, const Config &c);
 private:
  bool _valid;
  opts::positional_options_description pos;
  opts::options_description desc;

  void setInput(const opts::variables_map& vm);
  void setOutput(const opts::variables_map& vm);
  
  std::string _input;
  std::string _output;
  bool _overwrite;  
  size_t _buffer_sz;  
};

#endif /* NEV2PLX_CONFIG_H_INCLUDED */
