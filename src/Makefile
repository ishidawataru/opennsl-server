all: opennsl-server

CXX = g++
CPPFLAGS += -I/usr/local/include -I. -pthread
CXXFLAGS += -std=c++11
LDFLAGS += -L/usr/local/lib -L. `pkg-config --libs grpc++` -lprotobuf -lpthread -ldl -lopennsl
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

PROTOS_PATH = proto

vpath %.proto $(PROTOS_PATH)


opennsl-server: driver.pb.o driver.grpc.pb.o driverservice.pb.o driverservice.grpc.pb.o \
    init.pb.o init.grpc.pb.o initservice.pb.o initservice.grpc.pb.o \
    l2.pb.o l2.grpc.pb.o l2service.pb.o l2service.grpc.pb.o \
    port.pb.o port.grpc.pb.o portservice.pb.o portservice.grpc.pb.o \
    stat.pb.o stat.grpc.pb.o statservice.pb.o statservice.grpc.pb.o \
    link.pb.o link.grpc.pb.o linkservice.pb.o linkservice.grpc.pb.o \
    vlan.pb.o vlan.grpc.pb.o vlanservice.pb.o vlanservice.grpc.pb.o \
    knet.pb.o knet.grpc.pb.o knetservice.pb.o knetservice.grpc.pb.o \
    vlan.o link.o stat.o port.o l2.o knet.o server.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ $(LDFLAGS) -o $@

%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h opennsl_server
