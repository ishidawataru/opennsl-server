#include <future>

#include <grpc++/server.h>
#include <grpc++/impl/codegen/call.h>

#include "linkservice.grpc.pb.h"
#include "link.h"

extern "C" {
#include "opennsl/error.h"
#include "opennsl/link.h"
}

Queue<linkscan_info> q;

grpc::Status LinkServiceImpl::Detach(grpc::ServerContext* context, const link::DetachRequest* req, link::DetachResponse* res) {
    auto ret = opennsl_linkscan_detach(req->unit());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_linkscan_detach() failed");
    }
    return grpc::Status::OK;
}

grpc::Status LinkServiceImpl::LinkscanEnableSet(grpc::ServerContext* context, const link::LinkscanEnableSetRequest* req, link::LinkscanEnableSetResponse* res) {
    auto ret = opennsl_linkscan_enable_set(req->unit(), req->interval());
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_linkscan_enable_set() failed");
    }
    return grpc::Status::OK;
}

grpc::Status LinkServiceImpl::LinkscanEnableGet(grpc::ServerContext* context, const link::LinkscanEnableGetRequest* req, link::LinkscanEnableGetResponse* res) {
    int interval;
    auto ret = opennsl_linkscan_enable_get(req->unit(), &interval);
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_linkscan_enable_set() failed");
    }
    res->set_interval(interval);
    return grpc::Status::OK;
}

grpc::Status LinkServiceImpl::LinkscanModeSet(grpc::ServerContext* context, const link::LinkscanModeSetRequest* req, link::LinkscanModeSetResponse* res) {
    auto ret = opennsl_linkscan_mode_set(req->unit(), req->port(), int(req->mode()));
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_linkscan_mode_set() failed");
    }
    return grpc::Status::OK;
}

grpc::Status LinkServiceImpl::LinkscanModeGet(grpc::ServerContext* context, const link::LinkscanModeGetRequest* req, link::LinkscanModeGetResponse* res) {
    int mode;
    auto ret = opennsl_linkscan_mode_get(req->unit(), req->port(), &mode);
    if (ret != OPENNSL_E_NONE) {
        return grpc::Status(grpc::UNAVAILABLE, "opennsl_linkscan_mode_get() failed");
    }
    res->set_mode(link::LinkScanMode(mode));
    return grpc::Status::OK;
}

grpc::Status LinkServiceImpl::LinkscanModeSetPBM(grpc::ServerContext* context, const link::LinkscanModeSetPBMRequest* req, link::LinkscanModeSetPBMResponse* res) {
}

void LinkServiceImpl::handle_info(const linkscan_info& info) {
    grpc::WriteOptions option;
    link::MonitorResponse res;
    res.set_unit(info.unit);
    res.set_port(info.port);
    std::vector<linkscan_request*> _reqs;
    std::unique_lock<std::mutex> mlock(mutex_);
    for ( auto req : reqs ) {
        if ( req->writer->Write(res, option) ) {
            _reqs.push_back(req);
        } else {
            req->q.push(true);
        }
    }
    reqs = _reqs;
}

void LinkServiceImpl::loop() {
    while (true) {
        handle_info(q.pop());
    }
}

void linkscan_handler(int unit, opennsl_port_t port, opennsl_port_info_t *info) {
    linkscan_info i{unit, port, info};
    q.push(i);
}

grpc::Status LinkServiceImpl::Monitor(grpc::ServerContext* context, const link::MonitorRequest* req, grpc::ServerWriter<link::MonitorResponse>* writer) {
    if ( !monitoring ) {
        th = new std::thread(&LinkServiceImpl::loop, this);
        auto ret = opennsl_linkscan_register(req->unit(), linkscan_handler);
        if ( ret != OPENNSL_E_NONE ) {
            return grpc::Status(grpc::UNAVAILABLE, "opennsl_linkscan_register() failed");
        }
        monitoring = true;
    }
    auto request = new linkscan_request();
    request->writer = writer;

    {
        std::unique_lock<std::mutex> mlock(mutex_);
        reqs.push_back(request);
    } // unlock mutex

    // wait
    request->q.pop();
    return grpc::Status::OK;
}
