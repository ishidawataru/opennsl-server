#include <grpc++/server.h>

#include "statservice.grpc.pb.h"

class StatServiceImpl final : public statservice::Stat::Service {
    public:
        grpc::Status Init(grpc::ServerContext* context, const stat::InitRequest* req, stat::InitResponse* res);
        grpc::Status Clear(grpc::ServerContext* context, const stat::ClearRequest* req, stat::ClearResponse* res);
        grpc::Status Sync(grpc::ServerContext* context, const stat::SyncRequest* req, stat::SyncResponse* res);
        grpc::Status Get(grpc::ServerContext* context, const stat::GetRequest* req, stat::GetResponse* res);
//        grpc::Status MultiGet(grpc::ServerContext* context, const stat::MultiGetRequest* req, stat::MultiGetResponse* res);
};
