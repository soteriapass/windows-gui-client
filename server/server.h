#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include <memory>

#include <grpc++/grpc++.h>
#include <sqlite3.h>

#include "pswmgr.grpc.pb.h"

#include "conf.h"
#include "auth_token_info.h"

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
    sqlite3* m_Database;
    std::map<std::string, std::shared_ptr<auth_token_info>> m_AuthTokens;
};
