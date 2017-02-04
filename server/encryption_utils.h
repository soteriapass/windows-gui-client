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
    bool GenerateEncryptionKey(std::string& out_key, bool generate_public_key, std::string& out_public_key);
    bool GenerateAndSaveEncryptionKey(const std::string& out_file, bool save_public_key = false, std::string out_public_file = "");

    bool EncryptString(const std::string& unencrypted, unsigned char* encrypted, int& enc_size, const std::string& public_key_file);
    bool DecryptString(unsigned char* encrypted, unsigned char* unencrypted, const std::string& private_key_file);
}
