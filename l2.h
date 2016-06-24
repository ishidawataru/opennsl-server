#include <grpc++/server.h>

#include "l2service.grpc.pb.h"

extern "C" {
#include "opennsl/l2.h"
}

class L2ServiceImpl final : public l2service::L2::Service {
    public:
        grpc::Status AddAddress(grpc::ServerContext* context, const l2::AddAddressRequest* req, l2::AddAddressResponse* res);
        grpc::Status DeleteAddress(grpc::ServerContext* context, const l2::DeleteAddressRequest* req, l2::DeleteAddressResponse* res);
        grpc::Status GetAddress(grpc::ServerContext* context, const l2::GetAddressRequest* req, l2::GetAddressResponse* res);
        grpc::Status List(grpc::ServerContext* context, const l2::ListRequest* req, l2::ListResponse* res);
};
