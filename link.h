#include <vector>
#include <future>

#include <grpc++/server.h>

#include "linkservice.grpc.pb.h"
#include "queue.h"

extern "C" {
#include "opennsl/port.h"
}

struct linkscan_info {
    int unit;
    opennsl_port_t port;
    opennsl_port_info_t *info;
};

struct linkscan_request {
    grpc::ServerWriter<link::MonitorResponse>* writer;
    Queue<bool> q;
};

class LinkServiceImpl final : public linkservice::Link::Service {
    public:
        grpc::Status Detach(grpc::ServerContext* context, const link::DetachRequest* req, link::DetachResponse* res);
        grpc::Status LinkscanEnableSet(grpc::ServerContext* context, const link::LinkscanEnableSetRequest* req, link::LinkscanEnableSetResponse* res);
        grpc::Status LinkscanEnableGet(grpc::ServerContext* context, const link::LinkscanEnableGetRequest* req, link::LinkscanEnableGetResponse* res);
        grpc::Status LinkscanModeSet(grpc::ServerContext* context, const link::LinkscanModeSetRequest* req, link::LinkscanModeSetResponse* res);
        grpc::Status LinkscanModeGet(grpc::ServerContext* context, const link::LinkscanModeGetRequest* req, link::LinkscanModeGetResponse* res);
        grpc::Status LinkscanModeSetPBM(grpc::ServerContext* context, const link::LinkscanModeSetPBMRequest* req, link::LinkscanModeSetPBMResponse* res);
        grpc::Status Monitor(grpc::ServerContext* context, const link::MonitorRequest* request, grpc::ServerWriter<link::MonitorResponse>* writer);
    private:
        void loop();
        void handle_info(const linkscan_info&);
        std::vector<linkscan_request* > reqs;
        std::mutex mutex_;
        bool monitoring;
        std::thread* th;
};
