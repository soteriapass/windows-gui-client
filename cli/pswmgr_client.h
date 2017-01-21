#pragma once

#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include "pswmgr.grpc.pb.h"

class PasswordManagerClient final
{
public:
    PasswordManagerClient(std::shared_ptr<grpc::Channel> channel);

    bool Authenticate(const std::string& user, const std::string& pass, bool create = false);
    bool CreateUser(const std::string& user, const std::string& pass);
    const std::string& GetLastError() const { return m_LastError; }
private:
    std::unique_ptr<pswmgr::PasswordManager::Stub> m_PassMgrStub;
    std::unique_ptr<pswmgr::UserManagement::Stub> m_UserMgrStub;
    std::string m_LastError;
};
