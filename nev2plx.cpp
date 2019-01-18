#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <map>
#include <iomanip>

#include "nev2plx_config.h"
#include "NEVFile.h"

typedef std::map<std::uint16_t, int> IndexMap;
const int EV_COUNT = 21;
const int SLOW_CHANNELS = 0;

void write_file_header(NEVFile &src, std::fstream &dst);
void write_spike_headers(NEVFile &src, std::fstream &dst);
void write_event_headers(std::fstream &dst);

void write_digital(std::shared_ptr<DigitalPacket> packet,
		   std::fstream &plx,
		   std::uint16_t inital_value=0,
		   bool ignore_zeros=true,
		   bool ignore_negatives=true);

void write_spike(std::shared_ptr<SpikePacket> packet,
		 std::fstream &plx,
		 IndexMap &channel_map);


std::map<std::uint16_t, int> channel_to_index(NEVFile &nev);


int main(int argc, char* argv[]) {  
  Config config;
  try {
    config.parse(argc, argv);
  } catch(const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  std::cout << config << std::endl;
  
  NEVFile nev(config.get_input(), config.get_buffer_sz());
  
  auto map = channel_to_index(nev);   
  auto plx = std::fstream(config.get_output(), std::ios::out | std::ios::binary);
  
  write_file_header(nev, plx);
  write_spike_headers(nev, plx);
  write_event_headers(plx);

  auto  count = 0;
  while(!nev.eof()) {
    std::shared_ptr<Packet> packet = nev.readPacket(true, true, true);
    
    if(auto p = std::dynamic_pointer_cast<DigitalPacket>(packet)) {
      write_digital(p, plx, 3840, true, true);
    } else if (auto p = std::dynamic_pointer_cast<SpikePacket>(packet)) {
      write_spike(p, plx, map);
    } else if (auto p = std::dynamic_pointer_cast<StimPacket>(packet)) {
      std::int16_t BLOCK_TYPE = 4; //Event, not segment!
      std::uint16_t hi_time = 0;
      std::int16_t stim_event_channel = 21;
      std::int16_t unit = 0;
      std::int16_t N_WAVEFORMS = 0;
      std::int16_t N_WORDS = 0;
      
      plx.write((char*) &BLOCK_TYPE, sizeof(BLOCK_TYPE));
      plx.write((char*) &hi_time, sizeof(hi_time));
      plx.write((char*) &(p->timestamp), sizeof(std::uint32_t));
      plx.write((char*) &stim_event_channel, sizeof(std::int16_t));
      plx.write((char*) &unit, sizeof(unit));
      plx.write((char*) &N_WAVEFORMS, sizeof(N_WAVEFORMS));
      plx.write((char*) &N_WORDS, sizeof(N_WORDS));
    }
  }
   
  plx.close();  
 
  return 0;
}



void write_file_header(NEVFile &src, std::fstream &dst) {
  const std::uint32_t MAGIC = 0x58454c50;
  const std::int32_t VERSION = 105;             // Wild guess
  const std::int32_t SNIP_LENGTH = 52;          // From Ripple docs
  const std::int32_t PRETHRESH_SNIP_LENGTH = 15;// From Ripple docs
  const std::int32_t FAST_READ = 0;             // No idea!
  const std::uint16_t PREAMP_GAIN = 1;       // Not really relevant here...
  const std::int8_t TRODALNESS = 1;             // No 255-trode for you....
  const char padding[46] = {'\0'};            
  const std::int32_t dummy_ts[130][5] = {0};    // Fill this in later
  const std::int32_t dummy_ev[512] = {0};       // and this....
  
  dst.write((char*) &MAGIC, sizeof(std::uint32_t));
  dst.write((char*) &VERSION, sizeof(std::int32_t));

  auto comment =  src.get_comment();
  comment.resize(128, '\0');
  dst.write(comment.c_str(), sizeof(char) * 128);

  auto ts = src.get_timestampFS();
  dst.write((char*) &ts, sizeof(std::int32_t));

  auto n_channels = int(std::distance(src.spikeChannels_cbegin(),
				      src.spikeChannels_cend()));
  dst.write((char*) &n_channels,    sizeof(n_channels));
  dst.write((char*) &EV_COUNT,      sizeof(EV_COUNT));
  dst.write((char*) &SLOW_CHANNELS, sizeof(SLOW_CHANNELS));

  dst.write((char*) &SNIP_LENGTH,   sizeof(SNIP_LENGTH));
  dst.write((char*) &PRETHRESH_SNIP_LENGTH,   sizeof(PRETHRESH_SNIP_LENGTH));

  auto t0 = src.get_start_sys();
  int year = int(t0.year); int month = int(t0.month); int day = int(t0.day);
  int hour = int(t0.hour); int min =   int(t0.minute); int sec = int(t0.second);
  dst.write((char*) &year,  sizeof(std::int32_t));
  dst.write((char*) &month, sizeof(std::int32_t));
  dst.write((char*) &day,   sizeof(std::int32_t));
  dst.write((char*) &hour,  sizeof(std::int32_t));
  dst.write((char*) &min,   sizeof(std::int32_t));
  dst.write((char*) &sec,   sizeof(std::int32_t));

  dst.write((char*) &FAST_READ, sizeof(FAST_READ));
  auto fs = int(src.get_waveformFS());
  dst.write((char*) &ts, sizeof(fs));

  double dummy_last = 0.0;
  dst.write((char*) &dummy_last, sizeof(double));
  dst.write((char*) &TRODALNESS, sizeof(TRODALNESS)); 
  dst.write((char*) &TRODALNESS, sizeof(TRODALNESS)); //Yes, repeated twice 

  char bps;
  auto begin = src.spikeChannels_cbegin();
  if(src.allWaves16Bit())
    bps = 16;
  else {

    bps = char(begin->second.bytesPerSample * 8);
    for(auto i=begin; i!=src.spikeChannels_cend(); i++)
      if(bps != char(i->second.bytesPerSample*8)) {
	throw("Not good");
      }
  }
  dst.write((char*) &bps, sizeof(char));
  dst.write((char*) &bps, sizeof(char)); // Yes, repeated (for slow)
	    
  unsigned short peak_mv = 8191; // begin->scaleFactor * pow(2,bps - 1); *1e6;
  dst.write((char*) &peak_mv, sizeof(peak_mv));
  dst.write((char*) &peak_mv, sizeof(peak_mv));

  dst.write((char*) &PREAMP_GAIN, sizeof(PREAMP_GAIN));
  dst.write((char*) padding, sizeof(char)*46); //Per docs

  dst.write((char*) dummy_ts, sizeof(std::int32_t)*130*5);
  dst.write((char*) dummy_ts, sizeof(std::int32_t)*130*5);
  dst.write((char*) dummy_ev, sizeof(std::int32_t)*512);
	   
  return;
}

std::map<std::uint16_t, int> channel_to_index(NEVFile &nev) {
  int index = 0;
  std::map<std::uint16_t, int> m;
  for(auto i=nev.spikeChannels_cbegin(); i!=nev.spikeChannels_cend(); i++) {
    m[i->first]=++index;
  }
  return m;
}

void write_spike_headers(NEVFile &src, std::fstream &dst) {
  const std::int32_t WF_RATE = 10;
  const std::int32_t REF_CHANNEL = 0;
  const std::int32_t GAIN = 32;
  const std::int32_t FILTER = 1;
  const std::int32_t THRESHOLD = -634; //Can query header for this?
  const std::int32_t METHOD = 2;
  const std::int32_t n_units_dummy = 0; // Need to fill this in at the end
  const std::int16_t TEMPLATE[5][64] = {0};    // Is garbage okay here?
  const std::int32_t FIT[5] = {50,50,50,50,50};               // Or here?
  const std::int32_t SORT_WIDTH = 52; //docs
  const std::int16_t BOXES[5][2][4] = {0};
  const std::int32_t SORT_BEG = 1;
  const int PADDING[11] = {0};
  std::string comment = "Trellis online sorting";
  comment.resize(128, '\0');
  const char* comment_cstr = comment.c_str();
  
  int ix=0;
  for(auto i = src.spikeChannels_cbegin(); i!=src.spikeChannels_cend(); i++) {
    std::ostringstream ss;
    ss << std::setw(3) << std::setfill('0') << i->first;
    std::string s2 = "sig" + ss.str();
    s2.resize(32, '\0');

    dst.write(s2.c_str(), sizeof(char)*32);
    dst.write(s2.c_str(), sizeof(char)*32); // Repeated intentionally

    ix++;

    dst.write((char*) &ix, sizeof(ix));
    dst.write((char*) &WF_RATE, sizeof(WF_RATE));    
    dst.write((char*) &ix, sizeof(ix)); //reusing as sig channel
    dst.write((char*) &REF_CHANNEL, sizeof(REF_CHANNEL));
    dst.write((char*) &GAIN, sizeof(GAIN));

    dst.write((char*) &FILTER, sizeof(FILTER));
    dst.write((char*) &THRESHOLD, sizeof(THRESHOLD));
    dst.write((char*) &METHOD, sizeof(METHOD));
    dst.write((char*) &n_units_dummy, sizeof(n_units_dummy));
    dst.write((char*) TEMPLATE, sizeof(std::int16_t)*5*64);
    dst.write((char*) &FIT, sizeof(FIT[0])*5);
    dst.write((char*) &SORT_WIDTH, sizeof(SORT_WIDTH));

    dst.write((char*) BOXES, sizeof(std::int16_t)*5*2*4);
    dst.write((char*) &SORT_BEG, sizeof(SORT_BEG));

    dst.write(comment_cstr, sizeof(char)*128);
    dst.write((char*) PADDING, sizeof(int)*11);
  }    
} 
    
void write_event_headers(std::fstream &dst) {
  const int PADDING[33] = {0};

  // Create one event channel for the parallel port
  for(int i=1; i<=16; i++) {
     std::ostringstream name_ss;
     name_ss << "Event" << std::setw(3) << std::setfill('0') << i;
     std::string name =  name_ss.str();
     name.resize(32, '\0');
     dst.write(name.c_str(), sizeof(char)*32);

     dst.write((char*) &i, sizeof(std::int32_t));

     std::ostringstream comment_ss;
     comment_ss << "Parallel bit " << i;
     std::string comment = comment_ss.str();
     comment.resize(128, '\0');
     dst.write(comment.c_str(), sizeof(char)*128);
     
     dst.write((char*) PADDING, sizeof(std::int32_t)*33);
  }

  // Create one event channel per SMA channel
  for(int i=17; i<20; i++) {
     std::ostringstream name_ss;
     name_ss << "Event" << std::setw(3) << std::setfill('0') << i;
     std::string name =  name_ss.str();
     name.resize(32, '\0');
     dst.write(name.c_str(), sizeof(char)*32);

     dst.write((char*) &i, sizeof(std::int32_t));

     std::ostringstream comment_ss;
     comment_ss << "SMA " << i;
     std::string comment = comment_ss.str();
     comment.resize(128, '\0');
     dst.write(comment.c_str(), sizeof(char)*128);
     
     dst.write((char*) PADDING, sizeof(std::int32_t)*33);
  }

  {
    int i = 21;
    std::string name("Microstimulation");
    name.resize(32, '\0');

    dst.write(name.c_str(), sizeof(char) * 32);
    dst.write((char*) &i, sizeof(std::int32_t));

    std::string comment("All channels");
    comment.resize(128, '\0');
    dst.write(comment.c_str(), sizeof(char)*128);
		        
    dst.write((char*) PADDING, sizeof(std::int32_t)*33);
  }
}

 void write_digital(std::shared_ptr<DigitalPacket> packet,
		    std::fstream &plx,
		    std::uint16_t inital_value,
		    bool ignore_zeros,
		    bool ignore_negatives) {

   static bool initalized = false;
   static std::uint16_t last_value;
   const int N_PARALLEL_EVENTS = 16;
   
   const std::int16_t BLOCK_TYPE = 4;
   const std::uint16_t hi_time = 0;
   const std::int16_t UNIT_ID = 0;
   const std::int16_t N_WAVEFORMS = 0;
   const std::int16_t N_WORDS = 0;
   int new_value;
   std::int16_t channel;
   
   switch(packet->reason) {
     case DigitalReason::PARALLEL:
       if(!initalized) {
	 last_value = inital_value;
	 initalized = true;
       }

       new_value = packet->parallel - last_value;
       channel = std::int16_t(log2(abs(new_value)));
       last_value = packet->parallel;
       if((ignore_negatives && new_value<0) || (ignore_zeros && new_value==0)) {
	 return;
       }
     break;
   case DigitalReason::SMA1:
     channel = N_PARALLEL_EVENTS + 1;
     break;     
   case DigitalReason::SMA2:
     channel = N_PARALLEL_EVENTS + 2;
     break;
   case DigitalReason::SMA3:
     channel = N_PARALLEL_EVENTS + 3;
     break;
   case DigitalReason::SMA4:
     channel = N_PARALLEL_EVENTS + 4;
     break;
   }
   
   plx.write((char*) &BLOCK_TYPE, sizeof(BLOCK_TYPE));
   plx.write((char*) &hi_time, sizeof(hi_time));
   plx.write((char*) &(packet->timestamp), sizeof(std::uint32_t));
   plx.write((char*) &channel, sizeof(std::int16_t));
   plx.write((char*) &UNIT_ID, sizeof(UNIT_ID));
   plx.write((char*) &N_WAVEFORMS, sizeof(N_WAVEFORMS));
   plx.write((char*) &N_WORDS, sizeof(N_WORDS));
   return;
 }


void write_spike(std::shared_ptr<SpikePacket> packet,
		 std::fstream &plx, IndexMap &channel_map){
  
  static std::int16_t BLOCK_TYPE = 1;
  static std::uint16_t hi_time = 0;
  static std::int16_t N_WAVEFORMS = 1;
  static std::int16_t N_WORDS=52;
  short channel = short(channel_map[packet->electrodeID]);
  
  plx.write((char*) &BLOCK_TYPE, sizeof(BLOCK_TYPE));
  plx.write((char*) &hi_time,    sizeof(hi_time));
  plx.write((char*) &(packet->timestamp), sizeof(std::uint32_t));
  plx.write((char*) &channel, sizeof(std::uint16_t));
  plx.write((char*) &(packet->unit), sizeof(std::uint16_t));
  plx.write((char*) &N_WAVEFORMS, sizeof(N_WAVEFORMS));
  plx.write((char*) &(N_WORDS), sizeof(N_WORDS));
  plx.write((char*) packet->waveform, sizeof(char) * packet->len);  
}  


  
