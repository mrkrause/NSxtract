
struct EventSOA;

#include <string>
#include "NEVConfig.h"
#include "NEVFile.h"
#include "Matfile.h"
#include "eventsoa.h"

void saveEventsCSV(const NEVConfig &config, const NEVFile &file, const EventSOA &ev) {

  std::string filename = config.eventFilename(OutputFormat::CSV);
  std::ofstream out(filename);
  if(!out) {
    throw(std::runtime_error("Unable to open " + filename + " for writing"));
  }

  double stampToSec =  1.0 / static_cast<double>(file.timestampFS());
  
  out << "timestamp,tic,reason,parallel,sma1,sma2,sma3,sma4\n";
  
  for(unsigned i=0; i<ev.ts.size(); i++) {
    out << ev.ts[i] * stampToSec << ','
	<< ev.ts[i] << ','
	<< (int) ev.reason[i]    << ','
	<< ev.parallel[i]  << ','
	<< ev.sma1[i]      << ','
	<< ev.sma2[i]      << ','
	<< ev.sma3[i]      << ','
	<< ev.sma4[i]      << '\n';
  }
	
  out.close();
}


void saveEventsMatlab(const NEVConfig &config, const NEVFile &f, const EventSOA &ev) {
  std::string filename = config.eventFilename(OutputFormat::MATLAB);
  std::cout << "   Writing events to matlab file as " << filename << std::endl;
  MATFile m(filename, "wz");

  MW::mwSize ndims = static_cast<MW::mwSize>(2);
  MW::mwSize dim[2] = { static_cast<MW::mwSize>(ev.ts.size()), 1};

  std::vector<double> time(ev.ts.size());
  const double ticToSec = 1.0 / static_cast<double>(f.timestampFS());
  for(size_t i=0; i<ev.ts.size(); i++) {
    time[i] = double(ev.ts[i]) * ticToSec;
  }

  m.putArray("ts",   time.data(),   ndims, dim);
  m.putArray("tic", ev.ts.data(),  ndims, dim);

  
  // If you change the underlying type for DigitalReason, you
  // *must* adjust the line below to match. Sorry for breaking the encapsulation!
  m.putArray("reason", reinterpret_cast<const std::uint8_t*>(ev.reason.data()), ndims, dim);
  
  m.putArray("parallel", ev.parallel.data(), ndims, dim);
  m.putArray("sma1", ev.sma1.data(), ndims, dim);
  m.putArray("sma2", ev.sma2.data(), ndims, dim);
  m.putArray("sma3", ev.sma3.data(), ndims, dim);
  m.putArray("sma4", ev.sma4.data(), ndims, dim);
}


void saveEventsText(const NEVConfig &config, const NEVFile &file, const EventSOA &ev) {

  const double stampToSec =  1.0 / static_cast<double>(file.timestampFS());
  
  std::string filename = config.eventFilename(OutputFormat::TEXT);
  std::ofstream out(filename);
  if(!out) {
    throw(std::runtime_error("Unable to open " + filename + " for writing."));
  }
  
  out << "Digital events from " << config.input() << "\n\n";
  for(unsigned i=0; i<ev.ts.size(); i++) {
    out << "Event at " << (double) ev.ts[i] * stampToSec << " (tic " << ev.ts[i] << ")\n";
    out << "\tReason: " << (int) ev.reason[i] << "\n";
    out << "\tParallel: " << ev.parallel[i] << "\n";
    out << "\tSMA: " << ev.sma1[i] << " " << ev.sma2[i] << " "
	             << ev.sma3[i] << " " << ev.sma4[i] << "\n";
    out << "\n";
  }

  out.close();
}
