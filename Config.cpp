#include "Config.h"


Config::Config(void) : _valid(false), desc("Convert a Ripple NSx file to losslessly-compressed FLAC files") {
  desc.add_options()
    ("help", "Show this help message")
    ("input,i", 
         opts::value<std::string>(), 
         "NSx file to convert. If a directory, convert all NSx files in the directory.")
    ("output-dir,o", 
         opts::value<std::string>()->default_value("./"), 
         "Place the converted FLAC files in this directory")
    ("output-prefix", 
         opts::value<std::string>()->default_value(""),
         "The converted FLAC filenames start with this, followed by the channel number")
    ("threads", 
         opts::value<unsigned>()->default_value(1), 
         "Number of threads to use for compression")
    ("read-size", 
         opts::value<unsigned>()->default_value(60000),
         "Maximum number of samples to read at once")
    ("flac-compression", 
         opts::value<unsigned>()->default_value(8), 
         "FLAC compression level")
    ("matlab-header",
         opts::value<bool>()->default_value(true),
         "Write header/metadata as a Matlab file?")
    ("text-header",
         opts::value<bool>()->default_value(true),
         "Write header/metadata as a text file?")
    ("compress-data",
         opts::value<bool>()->default_value(true),
         "Compress the data in the NSx file?")
  ;

  pos.add("input", 1);
  pos.add("output-dir", 2);
  pos.add("output-prefix", 3);
}


unsigned int Config::nThreads(void) const {
  if(_valid) 
    return _nThreads;
  else
    throw(std::runtime_error("Options not initalized"));
}


unsigned int Config::readSize(void) const {
  if(_valid)
    return _readSize;
  else
    throw(std::runtime_error("Options not initalized"));
}


unsigned int Config::flacCompression(void) const {
  if(_valid)
    return _flacCompression;
  else
    throw(std::runtime_error("Options not initalized"));
}


std::string Config::input(void) const {
  if(_valid)
    return _input;
  else
    throw(std::runtime_error("Options not initalized"));
}


std::string Config::outputDir(void) const {
  if(_valid)
    return _outputDir;
  else
    throw(std::runtime_error("Options not initalized"));
}


std::string Config::outputPrefix(void) const {
    if(!_valid)
        throw(std::runtime_error("Options not initalized"));
    
    return _outputPrefix;
}


bool Config::matlabHeader(void) const {
    if(_valid)
        return _matlabHeader;
    else
        throw(std::runtime_error("Options not initalized"));
}


bool Config::textHeader(void) const {
    if(_valid)
        return _textHeader;
    else
        throw(std::runtime_error("Options not initalized"));
}

bool Config::compressData(void) const {
    if(_valid)
        return _compressData;
    else
        throw(std::runtime_error("Options not initalized"));
}

void Config::parse(int argc, char* argv[]) {

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

  // Check output directory and return it if valid
  setOutputDir(vm);

  _outputPrefix = vm["output-prefix"].as<std::string>();
  if(_singleFile && _outputPrefix.empty()) {
      fs::path p(_input);
      std::string f = p.filename().string();
      
      auto loc = f.find_last_of(".");
      
      _outputPrefix = f.substr(0, loc) + "_ch";
  }
    
    
    
  _nThreads = vm["threads"].as<unsigned>();
  _readSize = vm["read-size"].as<unsigned>();
  _flacCompression = vm["flac-compression"].as<unsigned>();
  _matlabHeader = vm["matlab-header"].as<bool>();
  _textHeader = vm["text-header"].as<bool>();
  _compressData = vm["compress-data"].as<bool>();
    
  _valid = true;
}

void Config::setInput(const opts::variables_map& vm) {

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



void Config::setOutputDir(const opts::variables_map& vm) {
  this->_outputDir = vm["output-dir"].as<std::string>();
  this->outputPath = fs::path(this->_outputDir);

  if(fs::exists(this->outputPath) && fs::is_regular_file(this->outputPath))
    throw(std::runtime_error("Cannot create a directory with the same name as a file!"));

  fs::create_directories(this->outputPath); //Throws on failure, does nothing if exists
  return;
}

void Config::setOutputDir(const fs::path &p) {
  this->outputPath = p;
  this->_outputDir = p.string();

  if(fs::exists(this->outputPath) && fs::is_regular_file(this->outputPath))
    throw(std::runtime_error("Cannot create a directory with the same name as a file!"));

  fs::create_directories(this->outputPath); //Throws on failure, does nothing if exists
  return;
}

  
std::string Config::outputFilename(std::uint16_t electrode, bool withPath) const {
  std::ostringstream str;
  
  str << outputPrefix() << std::setfill('0') << std::setw(3) << electrode;
  str << ".flac";
    
  if(withPath)
      return(outputPath / str.str()).string();
  else
      return str.str();
}


std::string Config::matlabHeaderFilename() const {
    std::string filename = outputPrefix();
    auto startAt = filename.find_last_of("_");
    
    if(startAt == std::string::npos) {
        return (outputPath / "header.mat").string();
    } else {
        filename.replace(startAt, std::string::npos, ".mat");
        return (outputPath / filename).string();
    }
}


std::string Config::textHeaderFilename() const {
    std::string filename = outputPrefix();
    auto startAt = filename.find_last_of("_");
    
    if(startAt == std::string::npos) {
        return (outputPath / "header.txt").string();
    } else {
        filename.replace(startAt, std::string::npos, ".txt");
        return (outputPath / filename).string();
    }
}


WorkQueue Config::toWorkQueue() {
  WorkQueue work;

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
    if((fs::is_regular_file(p) || fs::is_symlink(p)) && p.extension() == ".ns5") {
      Config fileConfig(*this);

      fileConfig._input = p.string();


      fs::path out = this->outputPath / p.stem();
      fileConfig.setOutputDir(out);

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


std::ostream& operator<<(std::ostream &out, const Config &c) {
  out << "Ripple-To-FLac Conversation Configuration: " << std::endl <<
    "\t Input: " << c._input << std::endl <<
    "\t Output Directory: " << c._outputDir << std::endl <<
    "\t Running in " << (c._singleFile ? "single file mode" : "directory mode") << std::endl <<
    std::endl <<
    "\t Writing Matlab header: " << (c._matlabHeader ? "Yes" : "No") << std::endl <<
    "\t Writing text header: " << (c._textHeader ? "Yes" : "No") << std::endl <<
    "\t Writing compressed data: " << (c._compressData ? "Yes": "No") << std::endl <<
    std::endl <<
    "\t Output Prefix: " << c._outputPrefix << std::endl <<
    "\t Compression level: " << c._flacCompression << std::endl <<
    "\t # of threads: " <<  c._nThreads << std::endl <<
    "\t I/O Block Size: " << c._readSize << std::endl <<
    std::endl;

  if(c._singleFile) {
    out << "\t First file will be called: " << c.outputFilename(1, true) <<
      std::endl;
  } else {
    out << "\t No files generated. Unpack this into per-file configuration." <<
      std::endl;
  }
  return out;
}
  
  

  
    

  
