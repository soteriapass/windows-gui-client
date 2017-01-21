#include "pswmgr_server.h"

// Static Member Initialization
PasswordManagerServerImpl* PasswordManagerServerImpl::ms_Instance = nullptr;

PasswordManagerServerImpl::PasswordManagerServerImpl()
: m_IsRunning(false)
{
}

PasswordManagerServerImpl::~PasswordManagerServerImpl()
{
}

grpc::Status PasswordManagerServerImpl::Authenticate(grpc::ServerContext* context, const pswmgr::AuthenticationRequest* request, pswmgr::AuthenticationReply* response)
{
    return grpc::Status::OK;
}

bool PasswordManagerServerImpl::Run()
{
    if(m_IsRunning)
        return false;

    const std::string server_address("0.0.0.0:50051");
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(this);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    m_IsRunning = true;

    server->Wait();
    return true;
}
