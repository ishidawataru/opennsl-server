#include <grpc++/server.h>

#include "statservice.grpc.pb.h"
#include "stat.h"

extern "C" {
#include "opennsl/error.h"
#include "opennsl/port.h"
}

grpc::Status StatServiceImpl::Init(grpc::ServerContext* context, const stat::InitRequest* req, stat::InitResponse* res) {
    auto ret = opennsl_stat_init(req->unit());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_stat_init() failed");
    }
    return grpc::Status::OK;
}

grpc::Status StatServiceImpl::Clear(grpc::ServerContext* context, const stat::ClearRequest* req, stat::ClearResponse* res) {
    auto ret = opennsl_stat_clear(req->unit(), req->port());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_stat_clear() failed");
    }
    return grpc::Status::OK;
}

grpc::Status StatServiceImpl::Sync(grpc::ServerContext* context, const stat::SyncRequest* req, stat::SyncResponse* res) {
    auto ret = opennsl_stat_sync(req->unit());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_stat_sync() failed");
    }
    return grpc::Status::OK;
}

grpc::Status StatServiceImpl::Get(grpc::ServerContext* context, const stat::GetRequest* req, stat::GetResponse* res) {
    uint64 value;
    auto ret = opennsl_stat_get(req->unit(), req->port(), opennsl_stat_val_t(req->type()), &value);
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_stat_get() failed");
    }
    res->set_value(value);
    return grpc::Status::OK;
}

//grpc::Status StatServiceImpl::MultiGet(grpc::ServerContext* context, const stat::MultiGetRequest* req, stat::MultiGetResponse* res) {
//    uint64 value;
//    auto ret = opennsl_stat_get(req->unit(), req->port(), req->type(), &value);
//    if (ret != OPENNSL_E_NONE) {
//        return grpc::Status(grpc::UNAVAILABLE, "opennsl_stat_sync() failed");
//    }
//    res->set_value(value);
//    return grpc::Status::OK;
//}
