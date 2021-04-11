
struct EventSOA;

#include <string>
#include "NEVConfig.h"
#include "NEVFile.h"
#include "MatFile.h"
#include "eventsoa.h"

void saveEventsCSV(const NEVConfig &config, const NEVFile &file, const EventSOA &ev) {

  std::string filename = config.eventFilename(OutputFormat::CSV);
  std::ofstream out(filename);
  if(!out) {
    throw(std::runtime_error("Unable to open " + filename + " for writing"));
  }

  double stampToSec =  1.0 / static_cast<double>(file.get_timestampFS());
  
  out << "timestamp,tic,reasoncode,parallel,sma1,sma2,sma3,sma4\n";
  
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
  /* Pack event data into a MAT file. Most of this is a 1-1 mapping from EventSOA,
     except that the SMA fields are condensed into Nx4 logical arrays.
  */
  
  std::string filename = config.eventFilename(OutputFormat::MATLAB);  
  MATFile m(filename, "wz");  


  /* Write a basic header */
  const char* header_fieldnames[] = {
    "dataFile",       // 0
    "captureMode",    // 1
    "comment",        // 2
    "creator",        // 3
    "timeResolution", //4
    "start_time_sys", //5
    "start_time_proc" //6
  };

  MW::mwSize header_dim[2] = {1, 1};
  MW::mxArray* header = MW::mxCreateStructArray(2, header_dim, 7, header_fieldnames);
  MW::mxSetFieldByNumber(header, 0, 0, MW::mxCreateString(config.input().c_str()));
  MW::mxSetFieldByNumber(header, 0, 1, MW::mxCreateString(f.get_digital_mode() ?
							  "parallel" : "serial"));
  MW::mxSetFieldByNumber(header, 0, 2, MW::mxCreateString(f.get_comment().c_str()));
  MW::mxSetFieldByNumber(header, 0, 3, MW::mxCreateString(f.get_creator().c_str()));
  MW::mxSetFieldByNumber(header, 0, 4, MW::mxCreateDoubleScalar(static_cast<double>(f.get_timestampFS())));
  MW::mxSetFieldByNumber(header, 0, 5, MW::mxCreateString(f.get_start_sys().str().c_str()));
  MW::mxSetFieldByNumber(header, 0, 6, MW::mxCreateDoubleScalar(static_cast<double>(f.get_start_proc())));

  m.putScalar("header", header);
  MW::mxDestroyArray(header);
			 
  
  MW::mwSize ndims = static_cast<MW::mwSize>(2);  
  MW::mwSize dim1x[2] = { static_cast<MW::mwSize>(ev.ts.size()), 1}; // dims for Nx1 matrix
  MW::mwSize dim4x[2] = { static_cast<MW::mwSize>(ev.ts.size()), 4}; // dims for Nx4 matrix (SMA)

  // Write times in seconds and clock ticks
  std::vector<double> time(ev.ts.size());
  const double ticToSec = 1.0 / static_cast<double>(f.get_timestampFS());
  for(size_t i=0; i<ev.ts.size(); i++) {
    time[i] = double(ev.ts[i]) * ticToSec;
  }
  m.putArray("ts",  time.data(),   ndims, dim1x);
  m.putArray("tic", ev.ts.data(),  ndims, dim1x);
  
  /* Cannot use a std::vector<bool> here because it doesn't 
     provide access to ::data(). We over-allocate so that
     we can reuse this buffer to put all 4 SMA into one matrix.
 */
  auto bufSize = ev.reason.size() * 4;
  bool* boolBuffer = new bool[bufSize];
  
  const std::string reason_str[8] = {
    "is_parallel_change",
    "is_sma1_change", "is_sma2_change", "is_sma3_change", "is_sma4_change", // This row are placeholders (skipped below)
    "is_output_change", "is_periodic_sample", "is_serial_change"};
  
  for(int reason_id = 0; reason_id < 8; reason_id++) {
    // Skip the SMA reasons and package them up separately later
    if(reason_id > 0 && reason_id < 5)
      continue;
      
    for(auto i=0; i < ev.reason.size(); i++) {
      if(ev.reason[i] & (1 << reason_id))
	boolBuffer[i] = true;
      else
	boolBuffer[i] = false;
    }

    m.putArray(reason_str[reason_id], boolBuffer, ndims, dim1x);
  }
  
  /* Pack the SMA changes into a single matrix */
  std::fill(boolBuffer, boolBuffer+bufSize, false);
  for(int sma_id=0; sma_id<4; sma_id++) {
    auto offset = ev.reason.size() * sma_id;
    std::transform(ev.reason.begin(), ev.reason.end(),
		   boolBuffer + offset, 
      [sma_id](DigitalReason r) -> bool {return static_cast<bool>(r & (1 << (1 + sma_id)));});
  }

  m.putArray("is_sma_change", boolBuffer, ndims, dim4x);   
    
  m.putArray("parallel_value", ev.parallel.data(), ndims, dim1x);

  /* Pack SMA values into a single matrix */
  std::fill(boolBuffer, boolBuffer+bufSize, false);
  std::transform(ev.sma1.begin(), ev.sma1.end(), boolBuffer,
		 [](std::uint16_t s) -> bool { return s !=0;});
  std::transform(ev.sma2.begin(), ev.sma2.end(), boolBuffer + ev.sma1.size(),
		 [](std::uint16_t s) -> bool { return s !=0;});
  std::transform(ev.sma3.begin(), ev.sma3.end(), boolBuffer + 2*ev.sma1.size(),
		 [](std::uint16_t s) -> bool { return s !=0;});
  std::transform(ev.sma4.begin(), ev.sma4.end(), boolBuffer + 3*ev.sma1.size(),
		 [](std::uint16_t s) -> bool { return s !=0;});

  m.putArray("sma_value", boolBuffer, ndims, dim4x);

  delete [] boolBuffer;
}


void saveEventsText(const NEVConfig &config, const NEVFile &file, const EventSOA &ev) {

  const double stampToSec =  1.0 / static_cast<double>(file.get_timestampFS());
  const static std::string reason_str[8] = {
    "Parallel",
    "SMA #1", "SMA #2", "SMA #3", "SMA #4",
    "Output", "Periodic", "Serial"
  };
  
  std::string filename = config.eventFilename(OutputFormat::TEXT);

  
  std::ofstream out(filename);
  if(!out) {
    throw(std::runtime_error("Unable to open " + filename + " for writing."));
  }
  
  out << "Digital events from " << config.input() << "\n"
      << "Capture mode: " << file.get_digital_mode() << "\n"
      << "Timestamp Resolution: " << file.get_timestampFS() << "\n"
      << "System time: " << file.get_start_sys() << "\n"
      << "Processor time: " << file.get_start_proc() << "\n"
      << "Comment: " << file.get_comment() << "\n"
      << "Creator: " << file.get_creator() << "\n"
      << "\n\n";          
    
  for(unsigned i=0; i<ev.ts.size(); i++) {
    out << "Event at " << (double) ev.ts[i] * stampToSec
	<< " (tic " << ev.ts[i] << ")\n";

    // Print reason as a code and text  
    bool first = true;
    for(int reason_id=0; reason_id < 8; reason_id++) {
      if((int)ev.reason[i] & (1 << reason_id)) {
	out << (first ? "\tReason: " : " & ") << reason_str[reason_id];

	if(first)
	  first = false;

      }
    }
    out << "\n";

    // Actual channel values
    out << "\tParallel: " << ev.parallel[i] << "\n";
    out << "\tSMA: " << ev.sma1[i] << " " << ev.sma2[i] << " "
	             << ev.sma3[i] << " " << ev.sma4[i] << "\n";
    out << "\n";
  }

  out.close();
}
