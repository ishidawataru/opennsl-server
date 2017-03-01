#include <cstring>
#include <sstream>
#include <future>

#include <grpc++/server.h>

#include "knet.grpc.pb.h"
#include "knet.h"

extern "C" {
#include "opennsl/error.h"
#include "opennsl/knet.h"
}

opennsl_knet_netif_t get_netif (const knet::Interface& req) {
    opennsl_knet_netif_t ret;
    opennsl_knet_netif_t_init(&ret);
    unsigned char baseMac[6] = { 0x02, 0x10, 0x18, 0x00, 0x00, 0x01 };
    ret.type = OPENNSL_KNET_NETIF_T_TX_LOCAL_PORT;
    ret.port = req.port();
    strcpy(ret.name, req.name());
    memcpy(ret.mac_addr, baseMac, 6);
    return ret;
}

opennsl_knet_filter_t get_filter (const knet::Interface& req, const opennsl_knet_netif_t& netif) {
    opennsl_knet_filter_t ret;
    opennsl_knet_filter_t_init(&ret);
    ret.type = OPENNSL_KNET_FILTER_T_RX_PKT;
    ret.flags = OPENNSL_KNET_FILTER_F_STRIP_TAG;
    ret.dest_type = OPENNSL_KNET_DEST_T_NETIF;
    ret.dest_id = netif.id;
    ret.match_flags = OPENNSL_KNET_FILTER_M_INGPORT;
    ret.m_ingport = netif.port;
    return ret;
}

grpc::Status KNETServiceImpl::InitKNET(grpc::ServerContext* context, const knet::InitRequest* req, knet::InitResponse* res){
    auto ret = opennsl_knet_netif_init(req->unit());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_knet_netif_init() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status KNETServiceImpl::AddKNET(grpc::ServerContext* context, const knet::AddRequest* req, knet::AddResponse* res){
    auto netif = get_netif(req->netif());
    auto ret = opennsl_knet_netif_create(req->unit(), &netif);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_knet_netif_create() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    auto filter = get_filter(req->netif(), netif);
    auto ret = opennsl_knet_filter_create(req->unit(), &filter, &netif);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_knet_filter_create() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status KNETServiceImpl::DeleteKNET(grpc::ServerContext* context, const knet::DeleteRequest* req, knet::DeleteResponse* res){
    // task here
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_knet_netif_destroy() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status KNETServiceImpl::ListKNET(grpc::ServerContext* context, const knet::ListRequest* req, knet::ListResponse* res){
    // task here
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_knet_netif_traverse() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}
