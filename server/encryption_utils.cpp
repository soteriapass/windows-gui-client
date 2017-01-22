#include "encryption_utils.h"

#include <string>
#include <sstream>
#include <string.h>
#include <random>

namespace encryption
{
    std::random_device rd;
    std::mt19937 e{rd()};;
    std::uniform_int_distribution<unsigned char> dist{1, std::numeric_limits<unsigned char>::max()};

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
}
