MATLAB_ROOT=/usr/local/MATLAB/R2018a
CC=$(MATLAB_ROOT)/bin/mex
CFLAGS=-v GCC=/usr/bin/gcc-5 -client engine -g -DMAT_FILE_SUPPORT
LIBS=-lFLAC -lFLAC++ -lboost_program_options -lboost_filesystem -lboost_system
TARGET = rippleToFlac
DEPS = NSxFile.h NSxConfig.h MatFile.h
OBJ = NSxConfig.o NSxFile.o NSxChannel.o NSxHeader.o nsx2mat.o nsx2txt.o rippleToFlac.o
COMMON_OBJ = typeHelper.o MatFile.o


%.o: %.cpp $(DEPS)
	$(CC) -c $@ $< $(CFLAGS)

rippleToFlac: $(COMMON_OBJ) NSxConfig.o NSxFile.o NSxChannel.o NSxHeader.o nsx2mat.o nsx2txt.o rippleToFlac.o
	$(CC) -output $@ $^ $(CFLAGS) $(LIBS)

NEVExtract: $(COMMON_OBJ) datapacket.o NEVConfig.o NEVFile.o extheader.o NEVExtract.o saveNEVEvents.o
	$(CC) -output $@ $^ $(CFLAGS) $(LIBS)
	
.PHONY: clean
clean:
	rm -f *.o *~ core
