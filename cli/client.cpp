#include "client.h"

PasswordManagerClient::PasswordManagerClient(std::shared_ptr<grpc::Channel> channel)
: m_AuthStub(pswmgr::Authentication::NewStub(channel)) 
{
}

bool PasswordManagerClient::Authenticate(const std::string& user, const std::string& pass, bool createUser)
{
    grpc::ClientContext context;
    pswmgr::AuthenticationRequest request;
    request.set_username(user);
    request.set_password(pass);

    pswmgr::AuthReply response;

    //std::cout << "IsPeerAuthenticated: " << context.auth_context()->IsPeerAuthenticated() << std::endl;

    grpc::Status status = m_AuthStub->Authenticate(&context, request, &response);
    if(!status.ok())
    {
        m_LastError = "Could not connect to server";
        return false;
    }

    if(!response.success())
    {
        m_LastError = "Authentication error";
        return false;
    }

    std::cout << "IsPeerAuthenticated: " << context.auth_context()->IsPeerAuthenticated() << std::endl;
    std::cout << "PeerIdentity: " << context.auth_context()->GetPeerIdentity()[0] << std::endl;
    std::cout << "GetPeerIdentityPropertyName: " << context.auth_context()->GetPeerIdentityPropertyName() << std::endl;

    std::cout << "Token: " << response.token() << std::endl;

    return true;
}

bool PasswordManagerClient::CreateUser(const std::string& user, const std::string& pass)
{
// virtual ::grpc::Status CreateUser(::grpc::ClientContext* context, const ::pswmgr::UserCreationRequest& request, ::pswmgr::SimpleReply* response)

}
