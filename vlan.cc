#include <sstream>

#include <grpc++/server.h>

#include "vlanservice.grpc.pb.h"
#include "vlan.h"
#include "port.h"

extern "C" {
#include "opennsl/error.h"
#include "opennsl/vlan.h"
}

grpc::Status VLANServiceImpl::Create(::grpc::ServerContext* context, const ::vlan::CreateRequest* req, ::vlan::CreateResponse* res){
    auto ret = opennsl_vlan_create(req->unit(), req->vid());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_vlan_create() failed");
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::Destroy(::grpc::ServerContext* context, const ::vlan::DestroyRequest* req, ::vlan::DestroyResponse* res){
    auto ret = opennsl_vlan_destroy(req->unit(), req->vid());
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_destroy() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::DestroyAll(::grpc::ServerContext* context, const ::vlan::DestroyAllRequest* req, ::vlan::DestroyAllResponse* res){
    auto ret = opennsl_vlan_destroy_all(req->unit());
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_destroy_all() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::PortAdd(::grpc::ServerContext* context, const ::vlan::PortAddRequest* req, ::vlan::PortAddResponse* res){
    auto pbmp = get_port_config(req->pbmp());
    auto ubmp = get_port_config(req->ut_pbmp());
    auto ret = opennsl_vlan_port_add(req->unit(), req->vid(), pbmp, ubmp);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_port_add() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::PortRemove(::grpc::ServerContext* context, const ::vlan::PortRemoveRequest* req, ::vlan::PortRemoveRequest* res){
    auto pbmp = get_port_config(req->pbmp());
    auto ret = opennsl_vlan_port_remove(req->unit(), req->vid(), pbmp);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_port_remove() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::GPortAdd(::grpc::ServerContext* context, const ::vlan::GPortAddRequest* req, ::vlan::GPortAddResponse* res){
    auto ret = opennsl_vlan_gport_add(req->unit(), req->vid(), req->port(), req->flags());
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_gport_add() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::GPortDelete(::grpc::ServerContext* context, const ::vlan::GPortDeleteRequest* req, ::vlan::GPortDeleteResponse* res){
    auto ret = opennsl_vlan_gport_delete(req->unit(), req->vid(), req->port());
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_gport_delete() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::GPortDeleteAll(::grpc::ServerContext* context, const ::vlan::GPortDeleteAllRequest* req, ::vlan::GPortDeleteAllResponse* res){
    auto ret = opennsl_vlan_gport_delete_all(req->unit(), req->vid());
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_gport_delete_all() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::List(::grpc::ServerContext* context, const ::vlan::ListRequest* req, ::vlan::ListResponse* res){
    opennsl_vlan_data_t *p;
    int count;
    auto ret = opennsl_vlan_list(req->unit(), &p, &count);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_list() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    for (auto i = 0 ; i < count; i++) {
        res->add_list();
        auto data = res->mutable_list(i);
        data->set_vid(p[i].vlan_tag);
        set_protobuf_port_config(data->mutable_pbmp(), p[i].port_bitmap);
        set_protobuf_port_config(data->mutable_ut_pbmp(), p[i].ut_port_bitmap);
    }
    ret = opennsl_vlan_list_destroy(req->unit(), p, count);
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_vlan_list_destroy() failed");
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::DefaultGet(::grpc::ServerContext* context, const ::vlan::DefaultGetRequest* req, ::vlan::DefaultGetResponse* res){
    opennsl_vlan_t vid;
    auto ret = opennsl_vlan_default_get(req->unit(), &vid);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_default_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_vid(vid);
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::DefaultSet(::grpc::ServerContext* context, const ::vlan::DefaultSetRequest* req, ::vlan::DefaultSetResponse* res){
    auto ret = opennsl_vlan_default_set(req->unit(), req->vid());
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_vlan_default_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::ControlSet(::grpc::ServerContext* context, const ::vlan::ControlSetRequest* req, ::vlan::ControlSetResponse* res){
    return grpc::Status::OK;
}

grpc::Status VLANServiceImpl::ControlPortSet(::grpc::ServerContext* context, const ::vlan::ControlPortSetRequest* req, ::vlan::ControlPortSetResponse* res){
    return grpc::Status::OK;
}
