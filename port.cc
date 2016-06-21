#include <iostream>

#include <grpc++/server.h>

#include "portservice.grpc.pb.h"
#include "port.h"

extern "C" {
#include "opennsl/error.h"
#include "opennsl/port.h"
}


grpc::Status PortServiceImpl::Init(grpc::ServerContext* context, const port::InitRequest* req, port::InitResponse* res) {
    auto ret = opennsl_port_init(req->unit());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_port_init() failed");
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::Clear(grpc::ServerContext* context, const port::ClearRequest* req, port::ClearResponse* res) {
    auto ret = opennsl_port_clear(req->unit());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_port_clear() failed");
    }
    return grpc::Status::OK;
}

opennsl_pbmp_t get_port_config(const google::protobuf::RepeatedField<google::protobuf::uint32>& pbmp) {
    opennsl_pbmp_t ret;
    int i = 0;
    for(auto b : pbmp) {
        ret.pbits[i++] = b;
    }
    return ret;
}

void set_protobuf_port_config(google::protobuf::RepeatedField<google::protobuf::uint32>* dst, const opennsl_pbmp_t& src) {
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        dst->Add(src.pbits[i]);
    }
    return;
}

grpc::Status PortServiceImpl::Probe(grpc::ServerContext* context, const port::ProbeRequest* req, port::ProbeResponse* res) {
    opennsl_pbmp_t okay_pbmp;
    opennsl_pbmp_t pbmp = get_port_config(req->pbmp());
    auto ret = opennsl_port_probe(req->unit(), pbmp, &okay_pbmp);
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_port_probe() failed");
    }
    set_protobuf_port_config(res->mutable_pbmp(), okay_pbmp);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::Detach(grpc::ServerContext* context, const port::DetachRequest* req, port::DetachResponse* res) {
    opennsl_pbmp_t okay_pbmp;
    opennsl_pbmp_t pbmp = get_port_config(req->pbmp());
    auto ret = opennsl_port_detach(req->unit(), pbmp, &okay_pbmp);
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_port_probe() failed");
    }
    set_protobuf_port_config(res->mutable_pbmp(), okay_pbmp);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::GetConfig(grpc::ServerContext* context, const port::GetConfigRequest* req, port::GetConfigResponse* res) {
    opennsl_port_config_t config;
    auto ret = opennsl_port_config_get(req->unit(), &config);
    if (ret != OPENNSL_E_NONE ) {
        return grpc::Status(grpc::UNAVAILABLE, "");
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_fe(config.fe.pbits[i]);
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_ge(config.ge.pbits[i]);
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_xe(config.xe.pbits[i]);
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_ce(config.ce.pbits[i]);
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_e(config.e.pbits[i]);
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_hg(config.hg.pbits[i]);
    }
    return grpc::Status::OK;
};

grpc::Status PortServiceImpl::GetPortName(grpc::ServerContext* context, const port::GetPortNameRequest* req, port::GetPortNameResponse* res) {
    res->set_name(opennsl_port_name(req->unit(), req->port()));
    return grpc::Status::OK;
}
