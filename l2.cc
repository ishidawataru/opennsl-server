#include <cstring>
#include <sstream>

#include <grpc++/server.h>

#include "l2.grpc.pb.h"
#include "l2.h"

extern "C" {
#include "opennsl/error.h"
#include "opennsl/l2.h"
}

opennsl_l2_addr_t get_l2_addr(const l2::Address& addr) {
    opennsl_l2_addr_t ret;
    opennsl_mac_t mac;
    std::memcpy(mac, addr.mac().c_str(), 6);
    opennsl_l2_addr_t_init(&ret, mac, addr.vid());
    ret.flags = addr.flags();
    ret.port = addr.port();
    ret.modid = addr.mod_id();
    ret.tgid = addr.trunk_id();
    return ret;
}

void set_protobuf_l2_address(l2::Address* dst, const opennsl_l2_addr_t& src) {
    dst->set_flags(l2::L2Flag(src.flags));
    dst->set_mac((const void*)src.mac, 6);
    dst->set_vid(src.vid);
    dst->set_port(src.port);
}

grpc::Status L2ServiceImpl::AddAddress(grpc::ServerContext* context, const l2::AddAddressRequest* req, l2::AddAddressResponse* res){
    auto addr = get_l2_addr(req->address());
    auto ret = opennsl_l2_addr_add(req->unit(), &addr);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_l2_addr_add() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status L2ServiceImpl::DeleteAddress(grpc::ServerContext* context, const l2::DeleteAddressRequest* req, l2::DeleteAddressResponse* res){
    opennsl_mac_t mac;
    std::memcpy(mac, req->mac().c_str(), 6);
    auto ret = opennsl_l2_addr_delete(req->unit(), mac, req->vid());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_l2_addr_delete() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status L2ServiceImpl::GetAddress(grpc::ServerContext* context, const l2::GetAddressRequest* req, l2::GetAddressResponse* res){
    opennsl_mac_t mac;
    opennsl_l2_addr_t addr;
    std::memcpy(mac, req->mac().c_str(), 6);
    auto ret = opennsl_l2_addr_get(req->unit(), mac, req->vid(), &addr);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_l2_addr_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    set_protobuf_l2_address(res->mutable_address(), addr);
    return grpc::Status::OK;
}

int trav_fn(int unit, opennsl_l2_addr_t *info, void *user_data) {
    l2::ListResponse* res = static_cast<l2::ListResponse* >(user_data);
    auto size = res->list_size();
    res->add_list();
    set_protobuf_l2_address(res->mutable_list(size), *info);
    return 0;
}

grpc::Status L2ServiceImpl::List(grpc::ServerContext* context, const l2::ListRequest* req, l2::ListResponse* res){
    auto ret = opennsl_l2_traverse(req->unit(), trav_fn, res);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_l2_traverse() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}
