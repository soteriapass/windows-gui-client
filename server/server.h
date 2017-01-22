#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <memory>

#include <grpc++/grpc++.h>

#include "pswmgr.grpc.pb.h"

#include "conf.h"
#include "db.h"
#include "auth_token_info.h"
#include "token_auth_metadata_processor.h"

class PasswordManagerServer final : public pswmgr::PasswordManager::Service, public pswmgr::UserManagement::Service, public pswmgr::Authentication::Service
{
public:
    static PasswordManagerServer* Instance();
    bool Init(conf& conf_file);
    bool Run(conf& conf_file);

protected:
    PasswordManagerServer();
    ~PasswordManagerServer();

private:
    virtual grpc::Status Authenticate(grpc::ServerContext* context, const pswmgr::AuthenticationRequest* request, pswmgr::AuthReply* response) override;
    virtual grpc::Status CreateUser(grpc::ServerContext* context, const pswmgr::UserCreationRequest* request, pswmgr::SimpleReply* response) override;

private:
    static PasswordManagerServer* ms_Instance;

    bool m_IsRunning;
    db* m_Database;
    std::map<std::string, std::shared_ptr<auth_token_info>> m_AuthTokens;
    
    std::shared_ptr<TokenAuthMetadataProcessor> m_TokenAuthMetadataProcessor;
};
