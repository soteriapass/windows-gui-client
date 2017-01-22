#include "server.h"

#include <thread>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "encryption_utils.h"
#include "sqlite_db.h"

// Static Member Initialization
PasswordManagerServer* PasswordManagerServer::ms_Instance = nullptr;

PasswordManagerServer::PasswordManagerServer()
: m_IsRunning(false)
, m_Database(nullptr)
{
}

PasswordManagerServer::~PasswordManagerServer()
{
}

grpc::Status PasswordManagerServer::Authenticate(grpc::ServerContext* context, const pswmgr::AuthenticationRequest* request, pswmgr::AuthReply* response)
{
    //First check to see if there are any users
    //I f there are none then we can simply say whatever the resul
    //The authentication has succeeded
    int userCount = m_Database->GetUserCount();
    if(userCount == -1)
    {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "");
    }
    else if(userCount == 0)
    {
        response->set_success(true);
        auto iter = m_AuthTokens.find("no-user");
        if(iter == m_AuthTokens.end())
        {
            auth_token_info* info = new auth_token_info(encryption::GenerateNewAuthToken("no-user"), "no-user");
             
            m_AuthTokens[info->token] = std::shared_ptr<auth_token_info>(info);
            m_AuthTokens["no-user"] = std::shared_ptr<auth_token_info>(info);
        }
        response->set_token(m_AuthTokens["no-user"]->token);
        return grpc::Status::OK;
    }

    std::string salt = m_Database->GetSaltForUser(request->username());
    std::string hashedPassword = encryption::HashAndSalt(request->password().c_str(), reinterpret_cast<const unsigned char*>(salt.c_str()), 10000, 64);
    if(m_Database->ValidPasswordForUser(request->username(), hashedPassword))
    {
        response->set_success(true);
        auto iter = m_AuthTokens.find(request->username());
        if(iter == m_AuthTokens.end())
        {
            auth_token_info* info = new auth_token_info(encryption::GenerateNewAuthToken(request->username()), request->username());

            m_AuthTokens[info->token] = std::shared_ptr<auth_token_info>(info);
            m_AuthTokens[request->username()] = std::shared_ptr<auth_token_info>(info);
        }

        response->set_token(m_AuthTokens[request->username()]->token);
        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid username or password");
}

grpc::Status PasswordManagerServer::CreateUser(grpc::ServerContext* context, const pswmgr::UserCreationRequest* request, pswmgr::SimpleReply* response)
{
    if(context == nullptr || !context->auth_context()->IsPeerAuthenticated())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "");
    }
    std::string salt = encryption::GetNewSalt(16);
    std::string hashedPassword = encryption::HashAndSalt(request->password().c_str(), reinterpret_cast<const unsigned char*>(salt.c_str()), 10000, 64);
    m_Database->InsertUser(m_Database->GetUserCount()+1, request->username(), hashedPassword, salt, 10000, true);
    return grpc::Status::OK;
}

PasswordManagerServer* PasswordManagerServer::Instance()
{
    if(ms_Instance == nullptr)
    {
        ms_Instance = new PasswordManagerServer();
    }
    return ms_Instance;
}

bool PasswordManagerServer::Init(conf& conf_file)
{
    if(m_Database != nullptr)
        return false;

    m_Database = new sqlite_db();
    return m_Database->Init(conf_file);
}

bool PasswordManagerServer::Run(conf& conf_file)
{
    if(m_Database == nullptr)
    {
        std::cerr << "Database has not been opened" << std::endl;
        return false;
    }

    if(m_IsRunning)
    {
        std::cerr << "Server already running" << std::endl;
        return false;
    }

    std::string cert;
    std::string key;
    std::string ca;
    if(!conf_file.get_server_certificate_file().empty())
    {
        std::ifstream file(conf_file.get_server_certificate_file(), std::ifstream::in);
        cert = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    if(!conf_file.get_server_key_file().empty())
    {
        std::ifstream file(conf_file.get_server_key_file(), std::ifstream::in);
        key = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    if(!conf_file.get_server_ca_file().empty())
    {
        std::ifstream file(conf_file.get_server_ca_file(), std::ifstream::in);
        ca = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    m_TokenAuthMetadataProcessor = std::shared_ptr<TokenAuthMetadataProcessor>(new TokenAuthMetadataProcessor(this));

    std::unique_ptr<grpc::Server> authenticationServer;
    {
        const std::string server_address(conf_file.get_authentication_address_and_port());
        grpc::ServerBuilder builder;

        auto credOptions = grpc::SslServerCredentialsOptions();
        credOptions.pem_root_certs = ca;
        credOptions.pem_key_cert_pairs.push_back({ key, cert });

        auto channelCreds = grpc::SslServerCredentials(credOptions);

        builder.AddListeningPort(server_address, channelCreds);
        builder.RegisterService(static_cast<pswmgr::Authentication::Service*>(this));

        authenticationServer = builder.BuildAndStart();
        std::cout << "Authentication service listening on " << server_address << std::endl;
    }

    std::unique_ptr<grpc::Server> passwordServer;
    {
        const std::string server_address(conf_file.get_password_manager_address_and_port());
        grpc::ServerBuilder builder;

        auto credOptions = grpc::SslServerCredentialsOptions();
        credOptions.pem_root_certs = ca;
        credOptions.pem_key_cert_pairs.push_back({ key, cert });

        auto channelCreds = grpc::SslServerCredentials(credOptions);
        channelCreds->SetAuthMetadataProcessor(m_TokenAuthMetadataProcessor);

        builder.AddListeningPort(server_address, channelCreds);
        builder.RegisterService(static_cast<pswmgr::PasswordManager::Service*>(this));

        passwordServer = builder.BuildAndStart();
        std::cout << "Password server listening on " << server_address << std::endl;
    }
    
    std::unique_ptr<grpc::Server> userMgmtServer;
    {
        const std::string server_address(conf_file.get_user_mangement_address_and_port());
        grpc::ServerBuilder builder;

        auto credOptions = grpc::SslServerCredentialsOptions();
        credOptions.pem_root_certs = ca;
        credOptions.pem_key_cert_pairs.push_back({ key, cert });

        auto channelCreds = grpc::SslServerCredentials(credOptions);
        channelCreds->SetAuthMetadataProcessor(m_TokenAuthMetadataProcessor);

        builder.AddListeningPort(server_address, channelCreds);
        builder.RegisterService(static_cast<pswmgr::UserManagement::Service*>(this));

        userMgmtServer = builder.BuildAndStart();
        std::cout << "User management server listening on " << server_address << std::endl;
    }

    m_IsRunning = true;

    authenticationServer->Wait();
    passwordServer->Wait();
    userMgmtServer->Wait();

    return true;
}
