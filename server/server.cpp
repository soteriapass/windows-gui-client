#include "server.h"

#include <thread>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include "encryption_utils.h"
#include "log.h"
#include "sqlite_db.h"

// Static Member Initialization
PasswordManagerServer* PasswordManagerServer::ms_Instance = nullptr;

PasswordManagerServer::PasswordManagerServer()
: m_IsRunning(false)
, m_Database(nullptr)
{
    logging::log("PasswordManagerServer::PasswordManagerServer", true);
}

PasswordManagerServer::~PasswordManagerServer()
{
    logging::log("PasswordManagerServer::~PasswordManagerServer", true);
}

grpc::Status PasswordManagerServer::Authenticate(grpc::ServerContext* context, const pswmgr::AuthenticationRequest* request, pswmgr::AuthReply* response)
{
    logging::log("PasswordManagerServer::Authenticate", true);
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
        auto iter = m_AuthTokens.find(request->username());
        if(iter == m_AuthTokens.end())
        {
            int userId = m_Database->GetUserId(request->username());
            std::string secretToken;
            if(m_Database->Get2FA(userId, secretToken))
            {
                if(request->tfa_token() == 0)
                {
                    logging::log("No 2FA Token received", true);
                    response->set_token_needed_for_2fa(true);
                    return grpc::Status(grpc::StatusCode::OK, "Need 2FA Token");
                }
                else if(request->tfa_token() != 0 && !encryption::CheckTimebasedCode(secretToken, request->tfa_token()))
                {
                    logging::log(secretToken, true);
                    std::cout << request->tfa_token() << std::endl;
                    logging::log("Invalid 2FA Token received", true);
                    response->set_token_needed_for_2fa(true);
                    return grpc::Status(grpc::StatusCode::OK, "Invalid 2FA Token");
                }
            }

            response->set_success(true);
            auth_token_info* info = new auth_token_info(encryption::GenerateNewAuthToken(request->username()), request->username());

            m_AuthTokens[info->token] = std::shared_ptr<auth_token_info>(info);
            m_AuthTokens[request->username()] = std::shared_ptr<auth_token_info>(info);
        }

        response->set_token(m_AuthTokens[request->username()]->token);
        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Invalid username or password");
}

grpc::Status PasswordManagerServer::ListPasswords(grpc::ServerContext* context, const pswmgr::SimpleRequest* request, pswmgr::PasswordList* response)
{
    logging::log("PasswordManagerServer::ListPasswords", true);
    if(context == nullptr || !context->auth_context()->IsPeerAuthenticated())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "");
    }

    auto propertyValues = context->auth_context()->FindPropertyValues(context->auth_context()->GetPeerIdentityPropertyName());
    if(propertyValues.empty())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Didn't find x-custom-auth-ticket in the call credentials");
    }
    std::string peerName = { (*propertyValues.begin()).data() };

    auto authTokenInfo = m_AuthTokens[peerName];
    int userId = m_Database->GetUserId(authTokenInfo->username);

    struct callback_func_cookie_t
    {
        pswmgr::PasswordList* response;
        std::string* private_key_file;
    };

    auto callbackFunc = [](char* account_name, char* username, char* password, char* extra, void* cookie) -> void
    {
        callback_func_cookie_t* callback_cookie = reinterpret_cast<callback_func_cookie_t*>(cookie);
        pswmgr::PasswordList* response = callback_cookie->response;
        pswmgr::PasswordEntry* entry = response->add_passwords();
        entry->set_account_name({ account_name });
        entry->set_username({ username });

        unsigned char decrypted[4098] = { };
        encryption::DecryptString(reinterpret_cast<unsigned char*>(password), decrypted, *callback_cookie->private_key_file);

        entry->set_password({ reinterpret_cast<char*>(decrypted) });
        entry->set_extra({ extra });
    };

    callback_func_cookie_t cookie;
    cookie.response = response;
    cookie.private_key_file = &m_PrivateKey;

    if(!m_Database->ListPasswords(userId, callbackFunc, &cookie))
    {
        return grpc::Status(grpc::StatusCode::UNAVAILABLE, "");
    }

    return grpc::Status::OK;
}

grpc::Status PasswordManagerServer::AddPassword(grpc::ServerContext* context, const pswmgr::PasswordEntry* request, pswmgr::SimpleReply* response)
{
    logging::log("PasswordManagerServer::AddPassword", true);
    if(context == nullptr || !context->auth_context()->IsPeerAuthenticated())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "");
    }

    auto propertyValues = context->auth_context()->FindPropertyValues(context->auth_context()->GetPeerIdentityPropertyName());
    if(propertyValues.empty())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Didn't find x-custom-auth-ticket in the call credentials");
    }
    std::string peerName = { (*propertyValues.begin()).data() };

    auto authTokenInfo = m_AuthTokens[peerName];
    int userId = m_Database->GetUserId(authTokenInfo->username);

    unsigned char encrypted[4098] = { };
    int enc_len;
    encryption::EncryptString(request->password(), encrypted, enc_len, m_PublicKey);

    if(!m_Database->AddPassword(userId, request->account_name(), request->username(), reinterpret_cast<char*>(encrypted), enc_len, request->extra()))
    {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "");
    }
    response->set_success(true);
    return grpc::Status::OK;
}

grpc::Status PasswordManagerServer::DeletePassword(grpc::ServerContext* context, const pswmgr::PasswordEntry* request, pswmgr::SimpleReply* response)
{
    logging::log("PasswordManagerServer::DeletePassword", true);
    if(context == nullptr || !context->auth_context()->IsPeerAuthenticated())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "");
    }

    auto propertyValues = context->auth_context()->FindPropertyValues(context->auth_context()->GetPeerIdentityPropertyName());
    if(propertyValues.empty())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Didn't find x-custom-auth-ticket in the call credentials");
    }
    std::string peerName = { (*propertyValues.begin()).data() };

    auto authTokenInfo = m_AuthTokens[peerName];
    int userId = m_Database->GetUserId(authTokenInfo->username);

    if(!m_Database->DeletePassword(userId, request->account_name()))
    {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error");
    }

    return grpc::Status::OK;
}

grpc::Status PasswordManagerServer::ModifyPassword(grpc::ServerContext* context, const pswmgr::PasswordEntry* request, pswmgr::SimpleReply* response)
{
    logging::log("PasswordManagerServer::ModifyPassword", true);
    if(context == nullptr || !context->auth_context()->IsPeerAuthenticated())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "");
    }

    auto propertyValues = context->auth_context()->FindPropertyValues(context->auth_context()->GetPeerIdentityPropertyName());
    if(propertyValues.empty())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "Didn't find x-custom-auth-ticket in the call credentials");
    }
    std::string peerName = { (*propertyValues.begin()).data() };

    auto authTokenInfo = m_AuthTokens[peerName];
    int userId = m_Database->GetUserId(authTokenInfo->username);

    unsigned char encrypted[4098] = { };
    int enc_len;
    encryption::EncryptString(request->password(), encrypted, enc_len, m_PublicKey);

    if(!m_Database->ModifyPassword(userId, request->account_name(), { reinterpret_cast<char*>(encrypted) }))
    {
        return grpc::Status(grpc::StatusCode::UNKNOWN, "Unknown error");
    }

    return grpc::Status::OK;
}

grpc::Status PasswordManagerServer::CreateUser(grpc::ServerContext* context, const pswmgr::UserCreationRequest* request, pswmgr::UserCreationReply* response)
{
    logging::log("PasswordManagerServer::CreateUser", true);
    if(context == nullptr || !context->auth_context()->IsPeerAuthenticated())
    {
        return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, "");
    }

    std::string salt = encryption::GetNewSalt(16);
    std::string hashedPassword = encryption::HashAndSalt(request->password().c_str(), reinterpret_cast<const unsigned char*>(salt.c_str()), 10000, 64);
    const int userId = m_Database->GetUserCount() + 1;
    if(m_Database->InsertUser(userId, request->username(), hashedPassword, salt, 10000, true))
    {
        response->set_success(true);
        if(request->add_2fa())
        {
            std::string tfaSecret;
            std::vector<int> scratchCodes;
            if(!encryption::GetNewTOTPSecret(tfaSecret, scratchCodes))
            {
                return grpc::Status(grpc::StatusCode::UNKNOWN, "totp generation error");
            }
            if(!m_Database->Insert2FA(userId, tfaSecret, scratchCodes))
            {
                response->set_success(false);
                return grpc::Status(grpc::StatusCode::UNKNOWN, "Database error");
            }
            response->set_secret(tfaSecret);
            for(int i = 0; i < scratchCodes.size(); ++i)
            {
                response->add_scratch_codes(scratchCodes[i]);
            }
            //Generate the qrcode with qrencode
            std::stringstream ss;
            ss << "qrencode -d 300 \"otpauth://totp/pswmgr(mfilion)?secret=" << tfaSecret << "\" -o qrcode.png";
            system(ss.str().c_str());

            std::ifstream qrcode;
            qrcode.open("qrcode.png", std::ios::in | std::ios::binary);
            if(qrcode.is_open())
            {
                qrcode.seekg (0, std::ios::end);
                auto size = qrcode.tellg();
                char* memblock = new char [size];
                qrcode.seekg (0, std::ios::beg);
                qrcode.read (memblock, size);
                qrcode.close();
                response->set_qrcode(memblock, size);
            }
            else
            {
                logging::log("Was unable to open qrcode.png", true);
            }
        }
        return grpc::Status::OK;
    }

    return grpc::Status(grpc::StatusCode::UNKNOWN, "Database error");
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

    m_PublicKey = conf_file.get_public_key_filename();
    m_PrivateKey = conf_file.get_private_key_filename();

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

    if(!conf_file.get_user_server_certificate_file().empty())
    {
        std::ifstream file(conf_file.get_user_server_certificate_file(), std::ifstream::in);
        cert = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    if(!conf_file.get_server_key_file().empty())
    {
        std::ifstream file(conf_file.get_user_server_key_file(), std::ifstream::in);
        key = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
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
