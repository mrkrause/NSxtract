NEVFile::NEVFile(std::string filename) {
  
    file.open(filename, std::ios_base::binary | std::ios_base::binary);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    if(!file) {
      throw(std::runtime_error("Cannot open file for reading"));
    }

    /* Read the magic word to ensure this is the right kind of file*/      
    const std::string typeString = "NEURALEV";
    char magic[8];
    file.read(reinterpret_cast<char*>(&magic), length(typeString));
    

    file.read(reinterpret_cast<char*>(&majorVersion), sizeof(majorVersion));
    file.read(reinterpret_cast<char*>(&minorVersion), sizeof(minorVersion));
    file.read(reinterpret_cast<char*>(&flags), sizeof(flags));

    file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
    file.read(reinterpret_cast<char*>(&packetSize), sizeof(packetSize));

    file.read(reinterpret_cast<char*>(&tsResolution), sizeof(tsResolution));
    file.read(reinterpret_cast<char*>(&sampResolution), sizeof(sampResolution));
    file.read(reinterpret_cast<char*>(&time), sizeof(time));

    file.read(reinterpret_cast<char*>(&creator), sizeof(char)*32);
    file.read(reinterpret_cast<char*>(&comment), sizeof(char)*256);

    file.read(reinterpret_cast<char*>(&nHeaders), sizeof(nHeaders));

}

std::ostream& operator<<(std::ostream& out, const NEVFile& f) {

    out << "NEV Header (file format " << int(f.majorVersion) << '.' << int(f.minorVersion) <<  ')' << std::endl;
    out << "Data collection began at " << f.time << std::endl;
    out << "Contents: " << f.label << ", beginning at offset " << int(f.offset) << std::endl;
    out << h.channelCount << " channels, sampled at " << h.getSamplingFreq() << "Hz. Time resolution " << h.timeResolution << " Hz." << std::endl;
    
    out << "Comments: " << h.comment << std::endl;
    
    return out;
}


	      
      
	      
	      


