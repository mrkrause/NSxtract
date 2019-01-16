# NSxtract - 

This repository contains C++ classes and programs for working with the NSx and NEV data files produced by 
[Ripple Neuro](https://rippleneuro.com/)'s Grapevine Neural Interface Processor. 

We use the following programs to convert those data into formats that are more amenable to analysis:

* NSxToFlac: Extract continuous wideband signals from an .NSx file to a collection of losslessly-compressed FLAC files. Metadata, in the form of Matlab .MAT (HDF5) file and a human-readable text file, is also extracted, as are analog channels. This reduces the size of the data by 2-5x, and speeds up I/O for analyses that consider only a few channels at a time. It is also easier to share a set of 50-50Mb files than one giant 300 Gb monstronsity. 

* NEVExtract: Extract digital events, spike snippets, and/or microstimulation trains from NEV files. These can be exported as Matlab .MAT (HDF5), human-readable text, or comma-separated value files. This is particularly useful if spike snippets were saved during data acquisition, because the resulting files can be annoyingly large.


### Building the programs

This code is written in C++14, and uses
* [Boost](https://www.boost.org/)
* [LibFLAC++](https://xiph.org/flac/)

Matlab files are currently written via the Matlab C API, via a wrapper class (MatFile.cpp). This requires building the code with mex and its C++ compiler. Doing so may require that you match the Boost and LibFLAC versions with those included in your matlab install and/or build them using the same compiler that mex uses (which may not be your system compiler!).\

### About the classes

The class organization matches the NEV/NSx spec fairly closely. See NEVspec_2_2_vNN.pdf in the Trellis documentation. 




