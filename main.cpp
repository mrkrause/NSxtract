//
//  main.cpp
//  dumpNEV
//
//  Created by Matthew Krause on 2/21/15.
//  Copyright (c) 2015 Matthew Krause. All rights reserved.
//

#include <iostream>
#include <string>

#include "NSxFile.h"

int main(int argc, const char * argv[]) {
      std::string filename = "/Users/mrk/Desktop/m005d.ns5";
  //std::string filename = "/home/mrk/m005d.ns5";
  
    NSxFile f(filename);

    std::int16_t *buffer = 0;
    int maxval;
    while(f.hasMoreData())
        maxval = f.readData(10,buffer);
    
    std::cout << "Read up to " <<  maxval << std::endl;
    for(auto i=0; i<10*96*2; i+=(96*2)) {
        std::cout << buffer[i] << std::endl;
    }
    
    delete [] buffer;
    return 0;
}
