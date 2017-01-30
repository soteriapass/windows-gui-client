#pragma once

#include <string>
#include <stdint.h>
#include <vector>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

namespace encryption
{
    std::string HashAndSalt(const char* pass, const unsigned char *salt, int iterations, int outputBytes);
    std::string GetNewSalt(int outputBytes);
    bool GetNewTOTPSecret(std::string& secretKey, std::vector<int>& scratchCodes);
    std::string GenerateNewAuthToken(const std::string& reference);
    int ComputeCode(const uint8_t* secret, int secretLen, unsigned long value);
    bool CheckTimebasedCode(const std::string& secretKey, const int code, int window = 17);
    int GenerateScratchCode();
}
