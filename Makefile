CC=g++
CFLAGS= -O3 -Wall -Werror -std=gnu++11
LIBS=-lFLAC++ -lboost_program_options -lboost_filesystem -lboost_system
TARGET = rippleToFlac
DEPS = NSxFile.h Config.h
OBJ = Config.o NSxFile.o NSxChannel.o NSxHeader.o rippleToFlac.o


%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rippleToFlac: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *~ core
