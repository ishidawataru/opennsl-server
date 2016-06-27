#include <cstring>
#include <sstream>

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
        std::ostringstream err;
        err << "opennsl_port_init() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::Clear(grpc::ServerContext* context, const port::ClearRequest* req, port::ClearResponse* res) {
    auto ret = opennsl_port_clear(req->unit());
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_port_clear() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
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
        std::ostringstream err;
        err << "opennsl_port_probe() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    set_protobuf_port_config(res->mutable_pbmp(), okay_pbmp);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::Detach(grpc::ServerContext* context, const port::DetachRequest* req, port::DetachResponse* res) {
    opennsl_pbmp_t okay_pbmp;
    opennsl_pbmp_t pbmp = get_port_config(req->pbmp());
    auto ret = opennsl_port_detach(req->unit(), pbmp, &okay_pbmp);
    if (ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_port_probe() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
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
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_port(config.port.pbits[i]);
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_cpu(config.cpu.pbits[i]);
    }
    for (int i = 0; i < _SHR_PBMP_WORD_MAX; i++) {
        res->mutable_config()->add_all(config.all.pbits[i]);
    }
    return grpc::Status::OK;
};

grpc::Status PortServiceImpl::GetPortName(grpc::ServerContext* context, const port::GetPortNameRequest* req, port::GetPortNameResponse* res) {
    res->set_name(opennsl_port_name(req->unit(), req->port()));
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortEnableSet(grpc::ServerContext* context, const port::PortEnableSetRequest* req, port::PortEnableSetResponse* res){
    auto ret = opennsl_port_enable_set(req->unit(), req->port(), req->enable());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_enable_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortEnableGet(grpc::ServerContext* context, const port::PortEnableGetRequest* req, port::PortEnableGetResponse* res){
    int enable;
    auto ret = opennsl_port_enable_get(req->unit(), req->port(), &enable);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_enable_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_enable(enable);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAdvertSet(grpc::ServerContext* context, const port::PortAdvertSetRequest* req, port::PortAdvertSetResponse* res){
    auto ret = opennsl_port_advert_set(req->unit(), req->port(), req->ability());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_advert_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAdvertGet(grpc::ServerContext* context, const port::PortAdvertGetRequest* req, port::PortAdvertGetResponse* res){
    opennsl_port_abil_t abil;
    auto ret = opennsl_port_advert_get(req->unit(), req->port(), &abil);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_advert_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_ability(abil);
    return grpc::Status::OK;
}

opennsl_port_ability_t get_ability(const port::Ability& src) {
    opennsl_port_ability_t dst;
    dst.speed_half_duplex = src.speed_half_duplex();
    dst.speed_full_duplex = src.speed_full_duplex();
    dst.pause = src.pause();
    dst.interface = src.interface();
    dst.medium = src.medium();
    dst.loopback = src.loopback();
    dst.flags = src.flags();
    dst.eee = src.eee();
    dst.fcmap = src.fcmap();
    dst.fec = src.fec();
    return dst;
}

void set_protobuf_ability(port::Ability* dst, const opennsl_port_ability_t& src) {
    dst->set_speed_half_duplex(src.speed_half_duplex);
    dst->set_speed_full_duplex(src.speed_full_duplex);
    dst->set_pause(src.pause);
    dst->set_interface(src.interface);
    dst->set_medium(src.medium);
    dst->set_loopback(src.loopback);
    dst->set_flags(src.flags);
    dst->set_eee(src.eee);
    dst->set_fcmap(src.fcmap);
    dst->set_fec(src.fec);
}

grpc::Status PortServiceImpl::PortAbilityAdvertSet(grpc::ServerContext* context, const port::PortAbilityAdvertSetRequest* req, port::PortAbilityAdvertSetResponse* res){
    auto ability = get_ability(req->ability());
    auto ret = opennsl_port_ability_advert_set(req->unit(), req->port(), &ability);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_ability_advert_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAbilityAdvertGet(grpc::ServerContext* context, const port::PortAbilityAdvertGetRequest* req, port::PortAbilityAdvertGetResponse* res){
    opennsl_port_ability_t ability;
    auto ret = opennsl_port_ability_advert_get(req->unit(), req->port(), &ability);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_ability_advert_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    set_protobuf_ability(res->mutable_ability(), ability);
    return grpc::Status::OK;
}


grpc::Status PortServiceImpl::PortAdvertRemoteGet(grpc::ServerContext* context, const port::PortAdvertRemoteGetRequest* req, port::PortAdvertRemoteGetResponse* res){
    opennsl_port_abil_t ability;
    auto ret = opennsl_port_advert_remote_get(req->unit(), req->port(), &ability);
    if ( ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_port_advert_remote_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_ability(ability);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAbilityRemoteGet(grpc::ServerContext* context, const port::PortAbilityRemoteGetRequest* req, port::PortAbilityRemoteGetResponse* res){
    opennsl_port_ability_t ability;
    auto ret = opennsl_port_ability_remote_get(req->unit(), req->port(), &ability);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_ability_remote_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    set_protobuf_ability(res->mutable_ability(), ability);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAbilityGet(grpc::ServerContext* context, const port::PortAbilityGetRequest* req, port::PortAbilityGetResponse* res){
    opennsl_port_abil_t ability;
    auto ret = opennsl_port_ability_get(req->unit(), req->port(), &ability);
    if ( ret != OPENNSL_E_NONE) {
        std::ostringstream err;
        err << "opennsl_port_ability_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_ability(ability);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAbilityLocalGet(grpc::ServerContext* context, const port::PortAbilityLocalGetRequest* req, port::PortAbilityLocalGetResponse* res){
    opennsl_port_ability_t ability;
    auto ret = opennsl_port_ability_local_get(req->unit(), req->port(), &ability);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_ability_local_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    set_protobuf_ability(res->mutable_ability(), ability);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortLinkscanSet(grpc::ServerContext* context, const port::PortLinkscanSetRequest* req, port::PortLinkscanSetResponse* res){
    auto ret = opennsl_port_linkscan_set(req->unit(), req->port(), req->linkscan());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_linkscan_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortLinkscanGet(grpc::ServerContext* context, const port::PortLinkscanGetRequest* req, port::PortLinkscanGetResponse* res){
    int linkscan;
    auto ret = opennsl_port_linkscan_get(req->unit(), req->port(), &linkscan);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_linkscan_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_linkscan(linkscan);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAutonegSet(grpc::ServerContext* context, const port::PortAutonegSetRequest* req, port::PortAutonegSetResponse* res){
    auto ret = opennsl_port_autoneg_set(req->unit(), req->port(), req->enable());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_autoneg_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortAutonegGet(grpc::ServerContext* context, const port::PortAutonegGetRequest* req, port::PortAutonegGetResponse* res){
    int enabled;
    auto ret = opennsl_port_autoneg_get(req->unit(), req->port(), &enabled);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_autoneg_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_enabled(enabled);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortSpeedMAX(grpc::ServerContext* context, const port::PortSpeedMAXRequest* req, port::PortSpeedMAXResponse* res){
    int speed;
    auto ret = opennsl_port_speed_max(req->unit(), req->port(), &speed);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_speed_max() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_speed(speed);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortSpeedSet(grpc::ServerContext* context, const port::PortSpeedSetRequest* req, port::PortSpeedSetResponse* res){
    auto ret = opennsl_port_speed_set(req->unit(), req->port(), req->speed());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_speed_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortSpeedGet(grpc::ServerContext* context, const port::PortSpeedGetRequest* req, port::PortSpeedGetResponse* res){
    int speed;
    auto ret = opennsl_port_speed_get(req->unit(), req->port(), &speed);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_speed_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_speed(speed);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortInterfaceSet(grpc::ServerContext* context, const port::PortInterfaceSetRequest* req, port::PortInterfaceSetResponse* res){
    auto ret = opennsl_port_interface_set(req->unit(), req->port(), static_cast<opennsl_port_if_t>(req->type()));
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_interface_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortInterfaceGet(grpc::ServerContext* context, const port::PortInterfaceGetRequest* req, port::PortInterfaceGetResponse* res){
    opennsl_port_if_t type;
    auto ret = opennsl_port_interface_get(req->unit(), req->port(), &type);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_interface_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_type(static_cast<port::InterfaceType>(type));
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortLinkStatusGet(::grpc::ServerContext* context, const ::port::PortLinkStatusGetRequest* req, ::port::PortLinkStatusGetResponse* res){
    int status;
    auto ret = opennsl_port_link_status_get(req->unit(), req->port(), &status);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_link_status_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    if ( status > 0 ) {
        res->set_up(true);
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortLinkFailedClear(::grpc::ServerContext* context, const ::port::PortLinkFailedClearRequest* req, ::port::PortLinkFailedClearResponse* res){
    auto ret = opennsl_port_link_failed_clear(req->unit(), req->port());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_link_failed_clear() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortControlSet(::grpc::ServerContext* context, const ::port::PortControlSetRequest* req, ::port::PortControlSetResponse* res){
    auto ret = opennsl_port_control_set(req->unit(), req->port(), static_cast<opennsl_port_control_t>(req->type()), req->value());
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_control_set() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortControlGet(::grpc::ServerContext* context, const ::port::PortControlGetRequest* req, ::port::PortControlGetResponse* res){
    int value;
    auto ret = opennsl_port_control_get(req->unit(), req->port(), static_cast<opennsl_port_control_t>(req->type()), &value);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_control_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_value(value);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortGportGet(::grpc::ServerContext* context, const ::port::PortGportGetRequest* req, ::port::PortGportGetResponse* res){
    opennsl_gport_t gport;
    auto ret = opennsl_port_gport_get(req->unit(), req->port(), &gport);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_gport_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_gport(gport);
    return grpc::Status::OK;
}

grpc::Status PortServiceImpl::PortLocalGet(::grpc::ServerContext* context, const ::port::PortLocalGetRequest* req, ::port::PortLocalGetResponse* res){
    opennsl_port_t port;
    auto ret = opennsl_port_local_get(req->unit(), req->gport(), &port);
    if ( ret != OPENNSL_E_NONE ) {
        std::ostringstream err;
        err << "opennsl_port_local_get() failed " << opennsl_errmsg(ret);
        return grpc::Status(grpc::UNAVAILABLE, err.str());
    }
    res->set_port(port);
    return grpc::Status::OK;
}
