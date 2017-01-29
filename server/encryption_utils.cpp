#include "encryption_utils.h"

#include <string>
#include <sstream>
#include <string.h>
#include <random>
#include <chrono>

#include "hmac.h"
#include "sha1.h"
#include "base32.h"

namespace encryption
{
    std::random_device rd;
    std::mt19937 e{rd()};;
    std::uniform_int_distribution<unsigned char> dist{1, std::numeric_limits<unsigned char>::max()};
    std::uniform_int_distribution<unsigned int> distScratch{10000000, 99999999};

    std::string HashAndSalt(const char* pass, const unsigned char *salt, int iterations, int outputBytes)
    {
        unsigned char out[outputBytes+1];
        memset(out, 0, outputBytes+1);
        PKCS5_PBKDF2_HMAC(pass, strlen(pass), salt, strlen(reinterpret_cast<const char*>(salt)), iterations, EVP_sha512(), outputBytes, out);

        char hexResult[outputBytes+1];
        for (int i = 0; i < outputBytes; i++)
        {
             sprintf(hexResult + (i * 2), "%02x", 255 & out[i]);
        }

        return { hexResult };
    }

    std::string StringToHex(const std::string& input)
    {
        static const char* const lut = "0123456789ABCDEF";
        size_t len = input.length();

        std::string output;
        output.reserve(2 * len);
        for (size_t i = 0; i < len; ++i)
        {
            const unsigned char c = input[i];
            output.push_back(lut[c >> 4]);
            output.push_back(lut[c & 15]);
        }
        return { output };
    }

    std::string GetNewSalt(int outputBytes)
    {
        std::stringstream ss;
        for(int i = 0; i < outputBytes/2; ++i)
        {
            ss << dist(e);
        }
        return StringToHex(ss.str());
    }

    std::string GenerateNewAuthToken(const std::string& reference)
    {
        return HashAndSalt(reference.c_str(), reinterpret_cast<const unsigned char*>(GetNewSalt(8).c_str()), 1, 8);
    }

    int ComputeCode(const uint8_t *secret, int secretLen, unsigned long value)
    {
        uint8_t val[8];
        for (int i = 8; i--; value >>= 8)
        {
            val[i] = value;
        }
        uint8_t hash[SHA1_DIGEST_LENGTH];
        hmac_sha1(secret, secretLen, val, 8, hash, SHA1_DIGEST_LENGTH);
        memset(val, 0, sizeof(val));
        int offset = hash[SHA1_DIGEST_LENGTH - 1] & 0xF;
        unsigned int truncatedHash = 0;
        for (int i = 0; i < 4; ++i)
        {
            truncatedHash <<= 8;
            truncatedHash  |= hash[offset + i];
        }
        memset(hash, 0, sizeof(hash));
        truncatedHash &= 0x7FFFFFFF;
        truncatedHash %= 1000000;
        return truncatedHash;
    }

    // Code originally taken from pam_google_authenticate.c 
    // (https://github.com/google/google-authenticator-libpam)
    //
    // Copyright 2010 Google Inc.
    // Author: Markus Gutschke
    //
    // Licensed under the Apache License, Version 2.0 (the "License");
    // you may not use this file except in compliance with the License.
    // You may obtain a copy of the License at
    //
    //      http://www.apache.org/licenses/LICENSE-2.0
    //
    // Unless required by applicable law or agreed to in writing, software
    // distributed under the License is distributed on an "AS IS" BASIS,
    // WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    // See the License for the specific language governing permissions and
    // limitations under the License.
    bool CheckTimebasedCode(const std::string& key, const int code, int window)
    {
        // All time based verification codes are no longer than six digits.
        if (code < 0 || code >= 1000000)
        {
            return false;
        }

        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

        auto intervals = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() / 30;

        int base32Len = key.size();
        int secretLen = (base32Len*5+7)/8;
        uint8_t* key_base32 = (uint8_t*)malloc(base32Len + 1);
        memcpy(key_base32, key.c_str(), base32Len);
        key_base32[base32Len] = '\000';
        secretLen = base32_decode(key_base32, key_base32, base32Len);

        for (int i = -((window-1)/2); i <= window/2; ++i)
        {
            auto computedCode = ComputeCode(key_base32, secretLen, intervals + i );
            if(computedCode == code)
            {
                return true;
            }
        }

        return false;
    }

    int GenerateScratchCode()
    {
        return distScratch(e);
    }
}
