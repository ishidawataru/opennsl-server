#include <vector>
#include <future>

#include <grpc++/server.h>

#include "knetservice.grpc.pb.h"

extern "C" {
#include "opennsl/knet.h"
}

class KNETServiceImpl final : public knetservice::KNET::Service {
    public:
        grpc::Status InitKNET(grpc::ServerContext* context, const knet::InitRequest* req, knet::InitResponse* res);
        grpc::Status AddKNET(grpc::ServerContext* context, const knet::AddRequest* req, knet::AddResponse* res);
        grpc::Status DeleteKNET(grpc::ServerContext* context, const knet::DeleteRequest* req, knet::DeleteResponse* res);
        grpc::Status ListKNET(grpc::ServerContext* context, const knet::ListRequest* req, knet::ListResponse* res);
};
