#include <string>
#include <cstdint>

#include <FLAC++/metadata.h>
#include <FLAC++/encoder.h>
#include <memory>


#include "Config.h"
#include "NSxFile.h"

#ifdef WINDOWS
#include "mingw.thread.h"
#endif

#include <thread>

typedef std::vector<std::unique_ptr<FLAC::Encoder::File> > EncoderBank;

struct ThreadData {
  /* This structure is for farming out FLAC encoding to separate threads. 
     It neither creates nor destroys any of these things! It's just a passthrough*/
  ThreadData(std::int16_t* _bulkBuffer, EncoderBank *_e, unsigned _nChannels) {
    bulkBuffer = _bulkBuffer;
    e = _e;
    nChannels = _nChannels;
  }

  std::int16_t* bulkBuffer;
  FLAC__int32* channelBuffer;
  EncoderBank* e;
  unsigned nChannels;

  unsigned datalen;

  unsigned start;
  unsigned stop;
};
  
void process_singleThreaded(NSxFile &f, const Config &c, EncoderBank &e);
void process_multiThreaded(NSxFile &f, const Config &config, EncoderBank &encoders);
void doEncode(ThreadData d);


int main(int argc, char *argv[]) {
  
    /* Parse input options */
    Config config;
    try {
        config.parse(argc, argv);
    } catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    NSxFile f(config.inputFile());
  
    if(config.matlabHeader()) {
        f.writeMatHeader(config);
    }
    
    if(config.textHeader()) {
        f.writeTxtHeader(config);
    }
    
    if(config.compressData()) {
        EncoderBank encoders;
        encoders.reserve(f.getChannelCount());

        unsigned i = 0;
        for(auto ch=f.channelBegin(); ch!=f.channelEnd(); i++, ch++) {
            encoders.push_back(std::unique_ptr<FLAC::Encoder::File>(new FLAC::Encoder::File));

            bool ok = true;
            ok &= encoders[i]->set_channels(1);
            ok &= encoders[i]->set_bits_per_sample(16); //fixed by Ripple hardware
            ok &= encoders[i]->set_compression_level(config.flacCompression());
            ok &= encoders[i]->set_sample_rate(f.getSamplingFreq());

            if(!ok) {
                throw(std::runtime_error("Unable to configure FLAC encoder"));
            }
    
            std::string filename = config.outputFilename((*ch).getNumericID());
            encoders[i]->init(filename.c_str());
        }

        try {
            if(config.nThreads() == 1)
                process_singleThreaded(f, config, encoders);
            else
                process_multiThreaded(f, config, encoders);
            
            } catch (std::runtime_error &e) {
                std::cerr << "Caught an exception: " << e.what() << std::endl;
                return -1;
        }
    }
    
    return 0;
}

void process_singleThreaded(NSxFile &f, const Config &config, EncoderBank &encoders) {
    
  std::int16_t* bulkBuffer = nullptr; // Allocated by f.readData; deleted below

  FLAC__int32* channelBuffer = new FLAC__int32[config.readSize()];
  const FLAC__int32* c = channelBuffer;

  // Read in a chunk of data, extract each electrode's "column", and encode it
  auto nChannels = f.getChannelCount();
  while(f.hasMoreData()) {      
    auto datalen = f.readData(config.readSize(), bulkBuffer);

    for(auto chan = 0U; chan < nChannels; chan++) {
      for(auto i=chan, j=0U; i<datalen*nChannels; i+=nChannels, j++) {
	channelBuffer[j] = FLAC__int32(bulkBuffer[i]);
      }
      
      encoders[chan]->process(&c, datalen);
    }
  }

  // Finish off the compression.
  for(auto e = encoders.begin(); e!=encoders.end(); e++)
    (*e)->finish();
  
    
  delete[] bulkBuffer;
  delete[] channelBuffer;
}

void process_multiThreaded(NSxFile &f, const Config &config, EncoderBank &encoders) {

  /* After watching a few runs, it looks like this program is almost always 
     CPU-bound (surprisingly little I/O waiting). So...let's get some more CPUs! */

  std::int16_t* bulkBuffer = nullptr; //Will be alloced by NSxFile.readData()
  FLAC__int32** channelBuffers = new FLAC__int32*[config.nThreads()];

  // Pack stuff into a struct for easier transfer and allocate buffers for each thread
  ThreadData td(bulkBuffer, &encoders, f.getChannelCount());
  unsigned stride = unsigned(std::ceil(double(f.getChannelCount()) / double(config.nThreads())));

  for(auto i=0U; i<config.nThreads(); i++) {
    channelBuffers[i] = new FLAC__int32[config.readSize()];
  }
  
  while(f.hasMoreData()) {
    td.datalen = f.readData(config.readSize(), td.bulkBuffer);

    std::vector<std::unique_ptr<std::thread> > threads;
    for(auto i = 0U; i<config.nThreads(); i++) {
      td.start = stride * i;
      td.stop = std::min(stride*(i+1), f.getChannelCount()) ;     
      
      td.channelBuffer = channelBuffers[i];

      threads.push_back(std::unique_ptr<std::thread>(new std::thread(doEncode, td)));
    }

    /*Rejoin after processing this block*/
    for(auto &t: threads) {
      t->join();
    }
  }
  
  for(auto e = encoders.begin(); e!=encoders.end(); e++)
    (*e)->finish();
  
  for(auto i=0U; i<config.nThreads(); i++) {
    delete[] channelBuffers[i];
  }

  delete[] channelBuffers;  
  delete[] bulkBuffer;    
}

void doEncode(ThreadData d)  {
  /* This takes the data and encodes it. It's meant to be called by a std::thread*/
  for(auto chan = d.start; chan < d.stop; chan++) {
    for(auto i=chan, j=0U; i<d.datalen*d.nChannels; i+=d.nChannels, j++) {
      d.channelBuffer[j] = FLAC__int32(d.bulkBuffer[i]);
    }
    
    const FLAC__int32* c = d.channelBuffer;
    (*(d.e))[chan]->process(&c, d.datalen);
  }   
}

  




					      
  
