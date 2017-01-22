#pragma once

#include <string>
#include <chrono>

struct auth_token_info
{
    auth_token_info(const std::string& i_token, const std::string& i_username)
    : token(i_token)
    , username(i_username)
    , creation(std::chrono::system_clock::now())
    , renewal(std::chrono::system_clock::now())
    , expiration(std::chrono::system_clock::now() + std::chrono::minutes(10))  
    {
    }
    const std::string token;
    const std::string username;
    const std::chrono::time_point<std::chrono::system_clock> creation;
    const std::chrono::time_point<std::chrono::system_clock> renewal;
    std::chrono::time_point<std::chrono::system_clock> expiration;
};
