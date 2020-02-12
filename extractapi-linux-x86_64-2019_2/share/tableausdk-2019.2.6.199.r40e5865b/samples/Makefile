#
# Build and run C/C++ samples.
#
# Requires g++ 5.0 or later.

LIBROOT = /usr/lib64
RELLIBROOT = ../../../lib64
FLAGS  = -I$(LIBROOT)/../include -I$(RELLIBROOT)/../include
CFLAGS = $(FLAGS) -std=c99
CXXFLAGS = $(FLAGS) -std=c++0x
LDFLAGS = -Wl,-rpath,$(LIBROOT)/tableausdk:$(RELLIBROOT)/tableausdk

LIBS = -L$(LIBROOT)/tableausdk -L$(RELLIBROOT)/tableausdk -lTableauCommon -lTableauHyperExtract -l:libstdc++.so.6

usage :
	@echo
	@echo "usage: make [target]"
	@echo
	@echo "Top-level targets:"
	@echo "  usage (default)        Generate this message"
	@echo "  clean                  Remove all executables, symbols, and Tableau extracts in the current directory"
	@echo "  build-c                Build the C sample"
	@echo "  build-cpp              Build the C++ sample"
	@echo "  build-both             Build the C sample and C++ sample"
	@echo "  run-c ARGS="..."       Build the C sample and run it with ARGS"
	@echo "  run-cpp ARGS="..."     Build the C++ sample and run it with ARGS"
	@echo
	@echo "For details, please see the Tableau SDK documentation:"
	@echo "https://onlinehelp.tableau.com/current/api/sdk/en-us/help.htm"

clean :
	rm -f DataExtract.log TableauSDK*.log \
        TableauSDKSample-c TableauSDKSample-cpp order-c.hyper order-cpp.hyper \

build-c : TableauSDKSample.c
	$(CC) $(CFLAGS) $(LDFLAGS) TableauSDKSample.c $(LIBS) -o TableauSDKSample-c

build-cpp : TableauSDKSample.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) TableauSDKSample.cpp $(LIBS) -o TableauSDKSample-cpp

build-both : build-c build-cpp

run-c : build-c
	./TableauSDKSample-c $(ARGS)

run-cpp : build-cpp
	./TableauSDKSample-cpp $(ARGS)
