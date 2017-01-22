#pragma once

#include <string>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

namespace encryption
{
    std::string HashAndSalt(const char* pass, const unsigned char *salt, int iterations, int outputBytes);
    std::string GetNewSalt(int outputBytes);
    std::string GenerateNewAuthToken(const std::string& reference);
}
