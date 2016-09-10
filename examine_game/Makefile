USE_SDL := 1

# This will likely need to be changed to suit your installation.
ALE := $(CURDIR)/../ALE-montezuma-modified

FLAGS := -g --std=c++11 -I$(ALE)/src -I$(ALE)/src/controllers -I$(ALE)/src/os_dependent -I$(ALE)/src/environment -I$(ALE)/src/external -L$(ALE) -DTFG_DIR="\"$(HOME)/Dropbox/TFG\""
CXX := g++
FILE := memory_record
LDFLAGS := -lale -lz -lSDL

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    FLAGS += -Wl,-rpath=$(ALE)
endif
ifeq ($(UNAME_S),Darwin)
    FLAGS += -framework Cocoa
endif

ifeq ($(strip $(USE_SDL)), 1)
  DEFINES += -D__USE_SDL -DSOUND_SUPPORT
  FLAGS += $(shell sdl-config --cflags)
  LDFLAGS += $(shell sdl-config --libs)
endif

all: memory_record heuristic_stop

memory_record: memory_record.cpp
	$(CXX) $(DEFINES) $(FLAGS) memory_record.cpp $(LDFLAGS) -o memory_record

heuristic_stop: heuristic_stop.cpp
	$(CXX) $(DEFINES) $(FLAGS) heuristic_stop.cpp $(LDFLAGS) -o heuristic_stop

clean:
	rm -rf memory_record heuristic_stop *.o
