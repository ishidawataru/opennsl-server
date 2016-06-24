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
        grpc::Status PortEnableSet(grpc::ServerContext* context, const port::PortEnableSetRequest* req, port::PortEnableSetResponse* res);
        grpc::Status PortEnableGet(grpc::ServerContext* context, const port::PortEnableGetRequest* req, port::PortEnableGetResponse* res);
        grpc::Status PortAdvertSet(grpc::ServerContext* context, const port::PortAdvertSetRequest* req, port::PortAdvertSetResponse* res);
        grpc::Status PortAdvertGet(grpc::ServerContext* context, const port::PortAdvertGetRequest* req, port::PortAdvertGetResponse* res);
        grpc::Status PortAbilityAdvertSet(grpc::ServerContext* context, const port::PortAbilityAdvertSetRequest* req, port::PortAbilityAdvertSetResponse* res);
        grpc::Status PortAbilityAdvertGet(grpc::ServerContext* context, const port::PortAbilityAdvertGetRequest* req, port::PortAbilityAdvertGetResponse* res);
        grpc::Status PortAdvertRemoteGet(grpc::ServerContext* context, const port::PortAdvertRemoteGetRequest* req, port::PortAdvertRemoteGetResponse* res);
        grpc::Status PortAbilityRemoteGet(grpc::ServerContext* context, const port::PortAbilityRemoteGetRequest* req, port::PortAbilityRemoteGetResponse* res);
        grpc::Status PortAbilityGet(grpc::ServerContext* context, const port::PortAbilityGetRequest* req, port::PortAbilityGetResponse* res);
        grpc::Status PortAbilityLocalGet(grpc::ServerContext* context, const port::PortAbilityLocalGetRequest* req, port::PortAbilityLocalGetResponse* res);
};
