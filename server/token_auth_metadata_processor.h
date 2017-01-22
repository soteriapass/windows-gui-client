#pragma once

#include <grpc++/grpc++.h>

class PasswordManagerServer;

class TokenAuthMetadataProcessor : public grpc::AuthMetadataProcessor 
{
public:
    TokenAuthMetadataProcessor(PasswordManagerServer* server);

    grpc::Status Process(const grpc::AuthMetadataProcessor::InputMetadata& auth_metadata, grpc::AuthContext* context, grpc::AuthMetadataProcessor::OutputMetadata* consumed_auth_metadata, grpc::AuthMetadataProcessor::OutputMetadata* response_metadata) override;
    
public:
    static const char kIdentityPropName[];

private:
    const PasswordManagerServer* const m_Server;
};
