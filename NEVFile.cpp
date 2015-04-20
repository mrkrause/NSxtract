NEVFile::NEVFile(std::string filename) {
  
    file.open(filename, std::ios_base::binary | std::ios_base::binary);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    if(!file) {
      throw(std::runtime_error("Cannot open file for reading"));
    }

    /* Read the magic word to ensure this is the right kind of file*/      
    std::string typeString = "NEURALEV";
    file.read


