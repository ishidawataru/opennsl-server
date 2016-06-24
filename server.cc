#include <string>

#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>

#include "driverservice.grpc.pb.h"
#include "driver.grpc.pb.h"
#include "l2service.grpc.pb.h"
#include "port.h"
#include "stat.h"
#include "link.h"
#include "vlan.h"
#include "l2.h"

extern "C" {
#include "sal/driver.h"
#include "sal/version.h"
#include "opennsl/error.h"
}

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using grpc::ServerWriter;
using grpc::Status;

class DriverServiceImpl final : public driverservice::Driver::Service {
    public:
        Status Init(ServerContext* context, const driver::InitRequest* req, driver::InitResponse* res) {
            int rv = 0;
            rv = opennsl_driver_init((opennsl_init_t *) NULL);
            if(rv == OPENNSL_E_NONE){
                return Status::OK;
            }
            return Status(grpc::UNAVAILABLE, "");
        }
        Status GetVersion(ServerContext* context, const driver::GetVersionRequest* req, driver::GetVersionResponse* res) {
            res->set_version(opennsl_version_get());
            return Status::OK;
        }
};

int main(int argc, char** argv) {
    std::string server_address("0.0.0.0:50051");
    DriverServiceImpl driverservice;
    PortServiceImpl portservice;
    StatServiceImpl statservice;
    LinkServiceImpl linkservice;
    VLANServiceImpl vlanservice;
    L2ServiceImpl l2service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&driverservice);
    builder.RegisterService(&portservice);
    builder.RegisterService(&statservice);
    builder.RegisterService(&linkservice);
    builder.RegisterService(&vlanservice);
    builder.RegisterService(&l2service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();

    return 0;
}
