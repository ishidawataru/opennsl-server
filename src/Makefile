
CXX = g++
export OPENNSL_ROOT := ../../OpenNSL
platform := as5712
CPPFLAGS += -I/usr/local/include -I/usr/include -I$(OPENNSL_ROOT)/include -I.
CXXFLAGS += -std=c++11 -Wall


LDFLAGS += -L/usr/local/lib -L/usr/lib -lshellish -lopennsl -L$(OPENNSL_ROOT)/bin/$(platform)
	
EXAMPLE_UTIL_SRCS = $(abspath $(OPENNSL_ROOT)/examples/util.c)
EXAMPLE_UTIL_OBJS = $(abspath $(OPENNSL_ROOT)/examples/util.o)

exutils:
	$(CC) ${CFLAGS} -c $(EXAMPLE_UTIL_SRCS)


osh:
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(CPPFLAGS) main.cpp -Wl,-Bstatic $^ $(EXAMPLE_UTIL_OBJS) -Wl,-Bdynamic -lc $(LDFLAGS) -o $@
	
all: exutils osh