MATLAB_ROOT=/usr/local/MATLAB/R2015a/
CC=$(MATLAB_ROOT)/bin/mex
CFLAGS=-client engine -g -DMAT_FILE_SUPPORT
LIBS=-lFLAC -lFLAC++ -lboost_program_options -lboost_filesystem -lboost_system
TARGET = rippleToFlac
DEPS = NSxFile.h Config.h MatFile.h
OBJ = typeHelper.o MatFile.o Config.o NSxFile.o NSxChannel.o NSxHeader.o nsx2mat.o nsx2txt.o rippleToFlac.o


%.o: %.cpp $(DEPS)
	$(CC) -c $@ $< $(CFLAGS)

rippleToFlac: $(OBJ)
	$(CC) -output $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *~ core
