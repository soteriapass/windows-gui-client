#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "pswmgr.grpc.pb.h"

class PasswordManagerServerImpl final : public pswmgr::PasswordManagerServer::Service
{
public:
    static PasswordManagerServerImpl* Instance();
    bool Run();

protected:
    PasswordManagerServerImpl();
    ~PasswordManagerServerImpl();

private:
    virtual grpc::Status Authenticate(grpc::ServerContext* context, const pswmgr::AuthenticationRequest* request, pswmgr::AuthenticationReply* response) override;

private:
    static PasswordManagerServerImpl* ms_Instance;

    bool m_IsRunning;
};
