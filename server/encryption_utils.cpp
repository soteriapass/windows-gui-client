#include "encryption_utils.h"

#include <string>
#include <sstream>
#include <string.h>
#include <random>
#include <chrono>
#include <iostream>
#include <fstream>

#include "hmac.h"
#include "sha1.h"
#include "base32.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

namespace encryption
{
    void PrintLastEncryptionError()
    {
        char * err = reinterpret_cast<char*>(malloc(130));
        ERR_load_crypto_strings();
        ERR_error_string(ERR_get_error(), err);
        std::cout << "ERROR: " << err << std::endl; 
    }

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

    bool GetNewTOTPSecret(std::string& secretKey, std::vector<int>& scratchCodes)
    {
        #define SECRET_BITS               128         // Must be divisible by eight
        #define VERIFICATION_CODE_MODULUS (1000*1000) // Six digits
        #define SCRATCHCODES              5           // Number of initial scratchcodes
        #define SCRATCHCODE_LENGTH        8           // Eight digits per scratchcode
        #define BYTES_PER_SCRATCHCODE     4           // 32bit of randomness is enough
        #define BITS_PER_BASE32_CHAR      5           // Base32 expands space by 8/5

        static const char hotp[]      = "\" HOTP_COUNTER 1\n";
        static const char totp[]      = "\" TOTP_AUTH\n";
        static const char disallow[]  = "\" DISALLOW_REUSE\n";
        static const char step[]      = "\" STEP_SIZE 30\n";
        static const char window[]    = "\" WINDOW_SIZE 17\n";
        static const char ratelimit[] = "\" RATE_LIMIT 3 30\n";
        char secret[(SECRET_BITS + BITS_PER_BASE32_CHAR-1)/BITS_PER_BASE32_CHAR +
                  1 /* newline */ +
                  sizeof(hotp) +  // hotp and totp are mutually exclusive.
                  sizeof(disallow) +
                  sizeof(step) +
                  sizeof(window) +
                  sizeof(ratelimit) + 5 + // NN MMM (total of five digits)
                  SCRATCHCODE_LENGTH*(SCRATCHCODES + 1 /* newline */) +
                  1 /* NUL termination character */];

        uint8_t buf[SECRET_BITS/8 + SCRATCHCODES*BYTES_PER_SCRATCHCODE];
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0) 
        {
            std::cerr << "Failed to open \"/dev/urandom\"";
            return false;
        }
        if (read(fd, buf, sizeof(buf)) != sizeof(buf)) 
        {
            std::cerr << "Failed to read from \"/dev/urandom\"";
            return false;
        }

        base32_encode(buf, SECRET_BITS/8, (uint8_t *)secret, sizeof(secret));
        secretKey = { reinterpret_cast<char*>(secret) };
        
        for (int i = 0; i < SCRATCHCODES; ++i) 
        {
        new_scratch_code:;
            int scratch = 0;
            for (int j = 0; j < BYTES_PER_SCRATCHCODE; ++j) 
            {
                scratch = 256*scratch + buf[SECRET_BITS/8 + BYTES_PER_SCRATCHCODE*i + j];
            }
            int modulus = 1;
            for (int j = 0; j < SCRATCHCODE_LENGTH; j++) 
            {
                modulus *= 10;
            }
            scratch = (scratch & 0x7FFFFFFF) % modulus;
            if (scratch < modulus/10) 
            {
                // Make sure that scratch codes are always exactly eight digits. If they
                // start with a sequence of zeros, just generate a new scratch code.
                if (read(fd, buf + (SECRET_BITS/8 + BYTES_PER_SCRATCHCODE*i),BYTES_PER_SCRATCHCODE) != BYTES_PER_SCRATCHCODE) 
                {
                    std::cerr << "Failed to read from \"/dev/urandom\"";
                    return false;
                }
                goto new_scratch_code;
            }
            /*if (!quiet) 
            {
                printf("  %08d\n", scratch);
            }*/
            snprintf(strrchr(secret, '\000'), sizeof(secret) - strlen(secret), "%08d\n", scratch);
            scratchCodes.push_back(scratch);
        }

        close(fd);
        return true;
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

    bool GenerateEncryptionKey(std::string& out_key, bool generate_public_key, std::string& out_public_key)
    {
        int ret = 0;

        RSA* rsa = nullptr;
        const int bits = 2048;
        const unsigned long e = RSA_F4;

        BIGNUM* bne = nullptr;
        bne = BN_new();
        ret = BN_set_word(bne,e);
        if(ret != 1)
        {
            return false;
        }

        rsa = RSA_new();
        ret = RSA_generate_key_ex(rsa, bits, bne, NULL);
        if(ret != 1)
        {
            BN_free(bne);
            RSA_free(rsa);
            return false;
        }

        if(generate_public_key)
        {
            //BIO* bp_public = BIO_new_file(out_public_file.c_str(), "w+");
            BIO* bp_public = BIO_new(BIO_s_mem());
            ret = PEM_write_bio_RSAPublicKey(bp_public, rsa);

            size_t key_length = BIO_pending(bp_public);
            char* raw_key = reinterpret_cast<char*>(malloc(key_length + 1));
            memset(raw_key, 0, key_length + 1);
            BIO_read(bp_public, raw_key, key_length);
            out_public_key = { raw_key };

            BIO_free_all(bp_public);
            free(raw_key);
            if(ret != 1)
            {
                BN_free(bne);
                RSA_free(rsa);
                return false;
            }
        }

        BIO* bp_private = BIO_new(BIO_s_mem());
        ret = PEM_write_bio_RSAPrivateKey(bp_private, rsa, nullptr, nullptr, 0, nullptr, nullptr);

        size_t key_length = BIO_pending(bp_private);
        char* raw_key = reinterpret_cast<char*>(malloc(key_length + 1));
        memset(raw_key, 0, key_length + 1);
        BIO_read(bp_private, raw_key, key_length);
        out_key = { raw_key };

        BIO_free_all(bp_private);
        BN_free(bne);
        RSA_free(rsa);
  
        if(ret != 1)
        {
            return false;
        }
        return true;
    }

    bool GenerateAndSaveEncryptionKey(const std::string& out_file, bool save_public_key, std::string out_public_file)
    {
        std::string privateKey;
        std::string publicKey;
        if(GenerateEncryptionKey(privateKey, save_public_key, publicKey))
        {
            std::ofstream file;
            file.open(out_file, std::ios::out | std::ios::binary);
            if(file.is_open())
            {
                file << privateKey;
                file.close();
            }
            if(save_public_key)
            {
                file.open(out_public_file, std::ios::out | std::ios::binary);
                if(file.is_open())
                {
                    file << publicKey << std::endl;;
                    file.close();
                }
            }
        }
    }

    RSA* CreateRSA(const std::string& key_file, bool public_key)
    {
        FILE* fp = fopen(key_file.c_str(), "rb");
        if(fp == nullptr)
        {
            return nullptr;
        }
        RSA* rsa = nullptr;
        if(public_key)
            rsa = PEM_read_RSA_PUBKEY(fp, &rsa, nullptr, nullptr);
        else
            rsa = PEM_read_RSAPrivateKey(fp, &rsa, nullptr, nullptr);
        if(rsa == nullptr)
        {
            PrintLastEncryptionError();
            RSA_free(rsa);
            return nullptr;
        }    
    }

    bool EncryptString(const std::string& plain_text, unsigned char* encrypted, const std::string& public_key_file)
    {
        RSA* rsa = CreateRSA(public_key_file, true);

        size_t size = RSA_size(rsa);

        auto len = RSA_public_encrypt(plain_text.size(), reinterpret_cast<const unsigned char*>(plain_text.c_str()), encrypted, rsa, RSA_PKCS1_PADDING);
        if(len == -1)
        {
            RSA_free(rsa);
            PrintLastEncryptionError();
            return false;
        }

        RSA_free(rsa);

        return true;
    }

    bool DecryptString(unsigned char* encrypted, unsigned char* unencrypted, const std::string& private_key_file)
    {
        FILE* fp = fopen(private_key_file.c_str(), "rb");
        if(fp == nullptr)
        {
            return false;
        }
        RSA* rsa = nullptr;
        rsa = PEM_read_RSAPrivateKey(fp, &rsa, nullptr, nullptr);
        if(rsa == nullptr)
        {
            PrintLastEncryptionError();
            RSA_free(rsa);
            return false;
        }

        size_t size = RSA_size(rsa);
        auto len = RSA_private_decrypt(size, encrypted, unencrypted, rsa, RSA_PKCS1_PADDING);
        if(len == -1)
        {
            RSA_free(rsa);
            PrintLastEncryptionError();
            return false;
        }

        RSA_free(rsa);
        
        return true;
    }
}
