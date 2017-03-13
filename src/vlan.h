#include <grpc++/server.h>

#include "vlanservice.grpc.pb.h"

class VLANServiceImpl final : public vlanservice::VLAN::Service {
    public:
        grpc::Status Create(::grpc::ServerContext* context, const ::vlan::CreateRequest* request, ::vlan::CreateResponse* response);
        grpc::Status Destroy(::grpc::ServerContext* context, const ::vlan::DestroyRequest* request, ::vlan::DestroyResponse* response);
        grpc::Status DestroyAll(::grpc::ServerContext* context, const ::vlan::DestroyAllRequest* request, ::vlan::DestroyAllResponse* response);
        grpc::Status PortAdd(::grpc::ServerContext* context, const ::vlan::PortAddRequest* request, ::vlan::PortAddResponse* response);
        grpc::Status PortRemove(::grpc::ServerContext* context, const ::vlan::PortRemoveRequest* request, ::vlan::PortRemoveRequest* response);
        grpc::Status GPortAdd(::grpc::ServerContext* context, const ::vlan::GPortAddRequest* request, ::vlan::GPortAddResponse* response);
        grpc::Status GPortDelete(::grpc::ServerContext* context, const ::vlan::GPortDeleteRequest* request, ::vlan::GPortDeleteResponse* response);
        grpc::Status GPortDeleteAll(::grpc::ServerContext* context, const ::vlan::GPortDeleteAllRequest* request, ::vlan::GPortDeleteAllResponse* response);
        grpc::Status List(::grpc::ServerContext* context, const ::vlan::ListRequest* request, ::vlan::ListResponse* response);
        grpc::Status DefaultGet(::grpc::ServerContext* context, const ::vlan::DefaultGetRequest* request, ::vlan::DefaultGetResponse* response);
        grpc::Status DefaultSet(::grpc::ServerContext* context, const ::vlan::DefaultSetRequest* request, ::vlan::DefaultSetResponse* response);
        grpc::Status ControlSet(::grpc::ServerContext* context, const ::vlan::ControlSetRequest* request, ::vlan::ControlSetResponse* response);
        grpc::Status ControlPortSet(::grpc::ServerContext* context, const ::vlan::ControlPortSetRequest* request, ::vlan::ControlPortSetResponse* response);
};
