#include <grpc++/server.h>

#include "portservice.grpc.pb.h"

extern "C" {
#include "opennsl/port.h"
}

opennsl_pbmp_t get_port_config(const google::protobuf::RepeatedField<google::protobuf::uint32>& pbmp);
void set_protobuf_port_config(google::protobuf::RepeatedField<google::protobuf::uint32>* dst, const opennsl_pbmp_t& src);

class PortServiceImpl final : public portservice::Port::Service {
    public:
        grpc::Status Init(grpc::ServerContext* context, const port::InitRequest* req, port::InitResponse* res);
        grpc::Status Clear(grpc::ServerContext* context, const port::ClearRequest* req, port::ClearResponse* res);
        grpc::Status Probe(grpc::ServerContext* context, const port::ProbeRequest* req, port::ProbeResponse* res);
        grpc::Status Detach(grpc::ServerContext* context, const port::DetachRequest* req, port::DetachResponse* res);
        grpc::Status GetConfig(grpc::ServerContext* context, const port::GetConfigRequest* req, port::GetConfigResponse* res);
        grpc::Status GetPortName(grpc::ServerContext* context, const port::GetPortNameRequest* req, port::GetPortNameResponse* res);
};
