#include "client.h"

PasswordManagerClient::PasswordManagerClient(conf& conf_file, std::shared_ptr<grpc::Channel> channel)
: m_AuthStub(pswmgr::Authentication::NewStub(channel)) 
, m_TokenAuth(nullptr)
, m_Conf(conf_file)
{
}

bool PasswordManagerClient::Authenticate(const std::string& user, const std::string& pass, bool createUser)
{
    grpc::ClientContext context;
    pswmgr::AuthenticationRequest request;
    request.set_username(user);
    request.set_password(pass);

    pswmgr::AuthReply response;

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

    m_TokenAuth = new TokenAuthenticator(response.token());

    auto callCreds = grpc::MetadataCredentialsFromPlugin(std::unique_ptr<grpc::MetadataCredentialsPlugin>(m_TokenAuth));
    m_PassMgrStub = pswmgr::PasswordManager::NewStub(GetChannel(m_Conf, m_Conf.get_password_manager_address_and_port(), callCreds));
    m_UserMgrStub = pswmgr::UserManagement::NewStub(GetChannel(m_Conf, m_Conf.get_user_mangement_address_and_port(), callCreds));

    return true;
}

bool PasswordManagerClient::CreateUser(const std::string& user, const std::string& pass)
{
    grpc::ClientContext context;
    pswmgr::UserCreationRequest request;
    request.set_username(user);
    request.set_password(pass);

    pswmgr::SimpleReply response;

    grpc::Status status = m_UserMgrStub->CreateUser(&context, request, &response);
    if(!status.ok())
    {
        m_LastError = "Could not connect to the server";
        return false;
    }

    if(!response.success())
    {
        return false;
    }

    return true;
}
