#include "token_auth_metadata_processor.h"

#include <sstream>

// Static Member Initialization
const char TokenAuthMetadataProcessor::kIdentityPropName[] = "x-custom-auth-ticket";

TokenAuthMetadataProcessor::TokenAuthMetadataProcessor(PasswordManagerServer* server) 
: m_Server(server)
{
}

grpc::Status TokenAuthMetadataProcessor::Process(const grpc::AuthMetadataProcessor::InputMetadata& auth_metadata, grpc::AuthContext* context, grpc::AuthMetadataProcessor::OutputMetadata* consumed_auth_metadata, grpc::AuthMetadataProcessor::OutputMetadata* response_metadata)
{
    //return grpc::Status(grpc::StatusCode::UNAUTHENTICATED, grpc::string("Invalid principal"));
    auto authMetadata = auth_metadata.find(kIdentityPropName);
    if(authMetadata == auth_metadata.end())
    {
        std::stringstream ss;
        ss << "Missing " << kIdentityPropName << " property";
        std::cout << ss;
        return grpc::Status(grpc::StatusCode::NOT_FOUND, ss.str());
    }

    // TODO: VALIDATE IF THE TOKEN IS AUTHENTIC
    context->AddProperty(kIdentityPropName, authMetadata->second);
    context->SetPeerIdentityPropertyName(kIdentityPropName);
    return grpc::Status::OK;
}
