#include <vector>
#include <future>

#include <grpc++/server.h>

#include "knetservice.grpc.pb.h"

extern "C" {
#include "opennsl/knet.h"
}

class KNETServiceImpl final : public knetservice::KNET::Service {
    public:
        grpc::Status InitKNET(grpc::ServerContext* context, const knet::InitKNETRequest* req, knet::InitKNETResponse* res);
        grpc::Status AddKNET(grpc::ServerContext* context, const knet::AddKNETRequest* req, knet::AddKNETResponse* res);
        grpc::Status DeleteKNET(grpc::ServerContext* context, const knet::DeleteKNETRequest* req, knet::DeleteKNETResponse* res);
        grpc::Status ListKNET(grpc::ServerContext* context, const knet::ListKNETRequest* req, knet::ListKNETResponse* res);
};
