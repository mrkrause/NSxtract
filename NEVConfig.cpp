#include "NEVConfig.h"

#include <map>

NEVConfig::NEVConfig(void) :
  _valid(false),
  desc("Extract events from a Ripple NEV (Neural Events) file") {

  desc.add_options()
    ("help", "Show this help message")
    ("input", 
         opts::value<std::string>(), 
         "NEV file to convert.\nIf a directory, convert all NEV files in the directory.")
    ("output-prefix", 
         opts::value<std::string>()->default_value(""),
        "Output filenames begin with this prefix (defaults to the stem of the .nev file)")    
    ("events-filetype",
     opts::value<std::string>()->default_value("matlab,text"),
     "If empty, or omitted, digital events are ignored. Otherwise, digital events are written in the specified formats. Supply more than one as a comma-separated list.\n\t- matlab: MATLAB .mat file (version 7).\n\t- csv: comma-separated values\n\t- text: Pretty-printed text")
    ("stim-filetype",
         opts::value<std::string>()->default_value(""),
         "If empty or omitted, stimulation events are ignored. Otherwise, stimulation events are written in the specified formats. Supply more than one as a comma separated list.\n\t- matlab: MATLAB .mat file (version 7)\n\t- csv: comma-separated values\n\t- text: Pretty-printed text (no waveforms, regardless of include-stim-waveforms).")
    ("spike-filetype",
          opts::value<std::string>()->default_value(""),
         "Currently unimplemented--throws if not empty. (This wouldn't be hard to fix). ")
    ("include-stim-waveforms",
     opts::value<bool>()->default_value(true),
         "Include stimulation waveforms in output?. Waveforms are never included in the text file.")
    ("include-spike-waveforms",
     opts::value<bool>()->default_value(false),
     "Include spike waveforms in output?");

  pos.add("input", 1);
  pos.add("output-prefix", 2);

  exts = {
    {OutputFormat::TEXT, "txt"},
    {OutputFormat::CSV, "csv"},
    {OutputFormat::MATLAB, "mat"}
  };
};






std::string NEVConfig::input(void) const {
  if(!_valid)
    throw(std::runtime_error("Options not initalized"));

  return _input;
}


std::string NEVConfig::outputPrefix(void) const {
    if(!_valid)
        throw(std::runtime_error("Options not initalized"));
    
    return _outputPrefix;
}



size_t NEVConfig::bufferSize(void) const {
  if(!_valid)
    throw(std::runtime_error("Options not initalized"));

  return _bufferSize;
}



void NEVConfig::parse(int argc, char* argv[]) {

  opts::variables_map vm;
  opts::store(opts::command_line_parser(argc, argv).
	      options(desc).positional(pos).run(), vm);

  if(vm.count("help") || argc==1) {
    std::cout << desc << std::endl;
    exit(0);
  }
			
  opts::notify(vm);

  // Check input file/dir and return it if valid
  setInput(vm);

  _outputPrefix = vm["output-prefix"].as<std::string>();
  if(_singleFile && _outputPrefix.empty()) {
      fs::path p(_input);
      std::string f = p.filename().string();
      
      auto loc = f.find_last_of(".");
      
      _outputPrefix = f.substr(0, loc) ;
  }
  
  _eventFileTypes = parseFormatString(vm["events-filetype"].as<std::string>());
  _stimFileTypes  = parseFormatString(vm["stim-filetype"].as<std::string>());
  _spikeFileTypes = parseFormatString(vm["spike-filetype"].as<std::string>());

  _stimWaves = vm["include-stim-waveforms"].as<bool>();
  _spikeWaves = vm["include-spike-waveforms"].as<bool>();
  _valid = true;
}



void NEVConfig::setInput(const opts::variables_map& vm) {

 if(!vm.count("input")) 
  throw(std::runtime_error("No input file or directory found!"));

 this->_input = vm["input"].as<std::string>();
 fs::path inputPath(_input);
 if(fs::exists(inputPath)) {
   if(fs::is_regular_file(inputPath) || fs::is_symlink(inputPath)) {
     _singleFile = true;
   } else if(fs::is_directory(inputPath)) {
     _singleFile = false;
   } else {
     throw(std::runtime_error("The input file is not a regular file or directory."));
   }
 } else {
   throw(std::runtime_error("The input file does not exist."));
 }
}



NEVWorkQueue NEVConfig::toWorkQueue() {
  NEVWorkQueue work;

  //Easy case: inputFile is a single file
  if(isSingleFileConfig()) {
      work.push_back(*this);
      return work;
  }

  /*Harder case: inputFile is a directory and we want to process all
      NSx files inside it. We want to extract inputDir/a.ns5 -->
      outputDir/a/, inputDir/b.ns5 --> outputDir/b/, and so on */

  fs::directory_iterator input_iter(_input);
  fs::directory_iterator end_of_dir; //Default ctor --> special "end" value

  for(fs::directory_iterator i(_input); i!=end_of_dir; ++i) {
    fs::path p  = i->path();
    if((fs::is_regular_file(p) || fs::is_symlink(p)) && p.extension() == ".nev") {
      NEVConfig fileConfig(*this);

      fileConfig._input = p.string();

      fs::path out = this->outputPath / p.stem();

      fileConfig._singleFile = true;

      if(fileConfig.outputPrefix().empty()) {
	fs::path p(fileConfig._input);
	std::string f = p.filename().string();      
	auto loc = f.find_last_of(".");      
	fileConfig._outputPrefix = f.substr(0, loc) + "_ch";
      }

      work.push_back(fileConfig);
    }
  }
  
  return work;
}


std::ostream& operator<<(std::ostream &out, const NEVConfig &c) {
  out << "Ripple Event Extraction Configuration: " << std::endl <<
    "\t Input: " << c._input << '\n' <<
    "\t Output Prefix: " << c._outputPrefix << "\n\n";

  auto ev = c.eventFileTypes();
  if(ev.empty()) {
    out << "\t Digital events ignored.\n"; 
  } else {
    out << "\t Digital events extracted to\n";
    for(auto fmt : ev) {
      out << "\t \t* " << c.eventFilename(fmt) << " (" << fmt << ")\n";
    }
  }

  auto stim = c.stimFileTypes();
  if(stim.empty()) {
    out << "\t Stimulation events ignored.\n";
  } else {
    out << "\t Stimulation events extracted to\n";
    for(auto fmt: stim) {
      out << "\t \t* " << c.stimFilename(fmt) << " (" << fmt
	  << (c.includeStimWaves() && fmt != OutputFormat::TEXT ? "#" : "")
	  <<  ")\n";	
    }
  }

  auto spike = c.spikeFileTypes();
   if(spike.empty()) {
    out << "\t Spike events ignored.\n";
  } else {
    out << "\t Spike events extracted to\n";
    for(auto fmt: spike) {
      out << "\t \t* " << c.spikeFilename(fmt) << " (" << fmt
	  << (c.includeSpikeWaves() && fmt != OutputFormat::TEXT ? "#" : "")
	  << ")\n";
    }
  }

   out << "\t (#) Includes waveform data.\n" << std::endl;
      
  return out;
}
  
std::vector<OutputFormat> NEVConfig::eventFileTypes(void) const {
  if(!valid())
    throw(std::runtime_error("Configuration is not valid"));

  return _eventFileTypes;
}

std::vector<OutputFormat> NEVConfig::stimFileTypes(void) const {
  if(!valid())
    throw(std::runtime_error("Configuration is not valid"));

  return _stimFileTypes;
}

std::vector<OutputFormat> NEVConfig::spikeFileTypes(void) const {
  if(!valid())
    throw(std::runtime_error("Configuration is not valid"));

  return _spikeFileTypes;
}

std::vector<OutputFormat> NEVConfig::parseFormatString(const std::string &fmtstr) {


  std::vector<OutputFormat> formats;
  std::stringstream ss(fmtstr);
  std::string token;

  const char DELIM = ',';
  const std::map<std::string, OutputFormat> VALID_FORMATS {
    {"matlab", OutputFormat::MATLAB},
    {"csv",    OutputFormat::CSV},
    {"text", OutputFormat::TEXT}
  };

  while(getline(ss, token, DELIM)) {
    auto res = VALID_FORMATS.find(token);
    if(res == VALID_FORMATS.end())
      throw(std::runtime_error("Unrecognized format " + token + "found"));
    else {
      formats.push_back(res->second);
    }
  }


  return formats;
}

std::string NEVConfig::eventFilename(const OutputFormat type) const {
    
  std::string filename = _outputPrefix + "-events." + exts.find(type)->second;

  return filename;
}

std::string NEVConfig::stimFilename(const OutputFormat type) const {
    
  std::string filename = _outputPrefix + "-microstim." + exts.find(type)->second;

  return filename;
}

std::string NEVConfig::spikeFilename(const OutputFormat type) const {
    
  std::string filename = _outputPrefix + "-online-spikes." + exts.find(type)->second;

  return filename;
}

std::ostream& operator<<(std::ostream &out, OutputFormat of) {
  switch(of) {
  case OutputFormat::TEXT:
    out << "text";
    break;
  case OutputFormat::CSV:
    out << "CSV";
    break;
  case OutputFormat::MATLAB:
    out << "matlab";
    break;
  }
  return out;
}
    
