#include <vector>
#include <future>

#include <grpc++/server.h>

#include "l2service.grpc.pb.h"
#include "queue.h"

extern "C" {
#include "opennsl/l2.h"
}

struct l2_info {
    int unit;
    opennsl_l2_addr_t *l2addr;
    int operation;
    void *userdata;
};

struct l2_request {
    grpc::ServerWriter<l2::MonitorResponse>* writer;
    Queue<bool> q;
};

class L2ServiceImpl final : public l2service::L2::Service {
    public:
        grpc::Status AddAddress(grpc::ServerContext* context, const l2::AddAddressRequest* req, l2::AddAddressResponse* res);
        grpc::Status DeleteAddress(grpc::ServerContext* context, const l2::DeleteAddressRequest* req, l2::DeleteAddressResponse* res);
        grpc::Status GetAddress(grpc::ServerContext* context, const l2::GetAddressRequest* req, l2::GetAddressResponse* res);
        grpc::Status List(grpc::ServerContext* context, const l2::ListRequest* req, l2::ListResponse* res);
        grpc::Status Monitor(grpc::ServerContext* context, const l2::MonitorRequest* req, grpc::ServerWriter< l2::MonitorResponse>* writer);
    private:
        void loop();
        void handle_info(const l2_info&);
        std::vector<l2_request* > reqs;
        std::mutex mutex_;
        bool monitoring;
        std::thread* th;
        Queue<l2_info>* info_q;
};
