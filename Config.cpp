#include "Config.h"


Config::Config(void) : valid(false), desc("Convert a Ripple NSx file to FLAC") {
  desc.add_options()
    ("help", "Show this help message")
    ("input-file,i", 
         opts::value<std::string>(), 
         "NSx file to convert")
    ("output-dir,o", 
         opts::value<std::string>()->default_value("./"), 
         "Place the converted FLAC files in this directory")
    ("output-prefix", 
         opts::value<std::string>()->default_value("ch"),
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
  ;

  
  pos.add("input-file", 1);
  pos.add("output-dir", 2);
  pos.add("output-prefix", 3);
}

unsigned int Config::nThreads(void) const {
  if(valid) 
    return _nThreads;
  else
    throw(std::runtime_error("Options not initalized"));
}

unsigned int Config::readSize(void) const {
  if(valid)
    return _readSize;
  else
    throw(std::runtime_error("Options not initalized"));
}

unsigned int Config::flacCompression(void) const {
  if(valid)
    return _flacCompression;
  else
    throw(std::runtime_error("Options not initalized"));
}

std::string Config::inputFile(void) const {
  if(valid)
    return _inputFile;
  else
    throw(std::runtime_error("Options not initalized"));
}

std::string Config::outputDir(void) const {
  if(valid)
    return _outputDir;
  else
    throw(std::runtime_error("Options not initalized"));
}

std::string Config::outputPrefix(void) const {
  if(valid)
    return _outputPrefix;
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

  // Check input file and return it if valid
  _inputFile = getInputFilename(vm);

  // Check output directory and return it if valid
  _outputDir = getOutputDir(vm);

  _outputPrefix = vm["output-prefix"].as<std::string>();
  _nThreads = vm["threads"].as<unsigned>();
  _readSize = vm["read-size"].as<unsigned>();
  _flacCompression = vm["flac-compression"].as<unsigned>();
  valid = true;
}


std::string Config::getInputFilename(const opts::variables_map& vm) {

if(!vm.count("input-file")) {    
    throw(std::runtime_error("No input file found!"));
  } else {

    std::string filename = vm["input-file"].as<std::string>();
    fs::path inputPath(filename);
    if(fs::exists(inputPath)) {
      if(fs::is_regular_file(inputPath) || fs::is_symlink(inputPath)) {
	return filename;
      } else {
	throw(std::runtime_error("The input file is not a regular file"));
      }
    } else {
      throw(std::runtime_error("The input file does not exist"));
    }
  }
}

std::string Config::getOutputDir(const opts::variables_map& vm) {
  std::string filename = vm["output-dir"].as<std::string>();
  outputPath = fs::path(filename);

  if(fs::exists(outputPath) && fs::is_regular_file(outputPath))
    throw(std::runtime_error("Cannot create a directory with the same name as a file!"));

  fs::create_directories(outputPath); //Throws on failure, does nothing if exists
  return filename;
}

std::string Config::outputFilename(std::uint16_t electrode) const {
  std::ostringstream str;
  
  str << outputPrefix() << std::setfill('0') << std::setw(3) << electrode;
  str << ".flac";
  std::string outfile = (outputPath / str.str()).string();

  return(outfile);
}


 
      
    


  
