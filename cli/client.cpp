#include "pswmgr_client.h"

PasswordManagerClient::PasswordManagerClient(std::shared_ptr<grpc::Channel> channel)
: m_PassMgrStub(pswmgr::PasswordManager::NewStub(channel)) 
, m_UserMgrStub(pswmgr::UserManagement::NewStub(channel))
{
}

bool PasswordManagerClient::Authenticate(const std::string& user, const std::string& pass, bool createUser)
{
    grpc::ClientContext context;
    pswmgr::AuthenticationRequest request;
    request.set_username(user);
    request.set_password(pass);

    pswmgr::SimpleReply response;

    grpc::Status status = createUser 
                              ? m_UserMgrStub->Authenticate(&context, request, &response) 
                              : m_PassMgrStub->Authenticate(&context, request, &response);
    if(!status.ok())
    {
        m_LastError = "Could not connect to server";
        return false;
    }

    if(!response.sucess())
    {
        m_LastError = "Authentication error";
        return false;
    }

    return true;
}

bool PasswordManagerClient::CreateUser(const std::string& user, const std::string& pass)
{
// virtual ::grpc::Status CreateUser(::grpc::ClientContext* context, const ::pswmgr::UserCreationRequest& request, ::pswmgr::SimpleReply* response)

}
