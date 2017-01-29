#include "server.h"
#include "conf.h"
#include "encryption_utils.h"
#include "log.h"

//#include <security/pam_appl.h>
//#include <security/pam_modules.h>

#include <openssl/hmac.h>
#include <sstream>

#include <chrono>
#include "hmac.h"
#include "sha1.h"
#include "base32.h"

int compute_code(const uint8_t *secret, int secretLen, unsigned long value) {
  uint8_t val[8];
  for (int i = 8; i--; value >>= 8) {
    val[i] = value;
  }
  uint8_t hash[SHA1_DIGEST_LENGTH];
  hmac_sha1(secret, secretLen, val, 8, hash, SHA1_DIGEST_LENGTH);
  memset(val, 0, sizeof(val));
  int offset = hash[SHA1_DIGEST_LENGTH - 1] & 0xF;
  unsigned int truncatedHash = 0;
  for (int i = 0; i < 4; ++i) {
    truncatedHash <<= 8;
    truncatedHash  |= hash[offset + i];
  }
  memset(hash, 0, sizeof(hash));
  truncatedHash &= 0x7FFFFFFF;
  truncatedHash %= 1000000;
  return truncatedHash;
}

#ifdef TESTING
static time_t current_time;
void set_time(time_t t) __attribute__((visibility("default")));
void set_time(time_t t) {
  current_time = t;
}

static time_t get_time(void) {
  return current_time;
}
#else
static time_t get_time(void) {
  return time(NULL);
}
#endif

static uint8_t *get_shared_secret(const char *buf, int *secretLen) {
  // Decode secret key
  int base32Len = strcspn(buf, "\n");
  *secretLen = (base32Len*5 + 7)/8;
  uint8_t *secret = reinterpret_cast<uint8_t*>(malloc(base32Len + 1));
  if (secret == NULL) {
    *secretLen = 0;
    return NULL;
  }
  memcpy(secret, buf, base32Len);
  secret[base32Len] = '\000';
  if ((*secretLen = base32_decode(secret, secret, base32Len)) < 1) {
    memset(secret, 0, base32Len);
    free(secret);
    return NULL;
  }
  memset(secret + *secretLen, 0, base32Len + 1 - *secretLen);

  return secret;
}

static char oom;

static char *get_cfg_value(const char *key,
                           const char *buf) {
  size_t key_len = strlen(key);
  for (const char *line = buf; *line; ) {
    const char *ptr;
    if (line[0] == '"' && line[1] == ' ' && !memcmp(line+2, key, key_len) &&
        (!*(ptr = line+2+key_len) || *ptr == ' ' || *ptr == '\t' ||
         *ptr == '\r' || *ptr == '\n')) {
      ptr += strspn(ptr, " \t");
      size_t val_len = strcspn(ptr, "\r\n");
      char *val = reinterpret_cast<char*>(malloc(val_len + 1));
      if (!val) {
        return &oom;
      } else {
        memcpy(val, ptr, val_len);
        val[val_len] = '\000';
        return val;
      }
    } else {
      line += strcspn(line, "\r\n");
      line += strspn(line, "\r\n");
    }
  }
  return NULL;
}

#include <iostream>
#include <fstream>

void test()
{
    /*std::ifstream file;
    file.open("/home/mfilion/.google_authenticator", std::ios::in);
    if(!file.is_open())
    {
        std::cout << "Didn't work" << std::endl;
        return;
    }

    std::stringstream ss;
    std::string line;
    while(getline(file, line))
    {
        ss << line << '\n';
    }

    int secretLen = 0;
    auto secret = get_shared_secret(ss.str().c_str(), &secretLen);

    std::cout << "secret: " << secret << std::endl;
    std::cout << "length: " << secretLen << std::endl;

    auto now = std::chrono::system_clock::now();
    auto tm = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() / 30;
    int window = 17;

    //TIME_SKEW
    const char *skew_str = get_cfg_value("TIME_SKEW", ss.str().c_str());
    int skew = 0;
    if (skew_str) {
        skew = (int)strtol(skew_str, NULL, 10);
    }
    free((void *)skew_str);

    for (int i = -((window-1)/2); i <= window/2; ++i)
    {
        std::cout << "HASH:" << compute_code(secret, secretLen, tm + skew + i) << std::endl;
    }

return;*/
//    pam_sm_authenticate(nullptr, 0, 0, nullptr);
//    pam_start(nullptr, nullptr, nullptr, nullptr); add -lpam to MAkefile

    std::chrono::time_point<std::chrono::system_clock> p1, p2, p3;
 
    p2 = std::chrono::system_clock::now();
    p3 = p2 - std::chrono::hours(24);
 
    std::time_t epoch_time = std::chrono::system_clock::to_time_t(p1);
    std::cout << "epoch: " << std::ctime(&epoch_time) << std::endl;

    auto C = std::chrono::duration_cast<std::chrono::seconds>(p2.time_since_epoch()).count() / 30;
    std::cout << "seconds since epoch: " << C << std::endl;
    std::cout << "time_since_epoch: " << (C/30) << "(" << (C%30) << ")" << std::endl;

    char key[] = "PL27LSK6PRNZHKX5VMKVFQZTYM";
    std::cout << "Key: " << key << std::endl;
    int window = 17;

    for (int i = -((window-1)/2); i <= window/2; ++i)
    {
        std::cout << "HASH:" << compute_code((uint8_t*)key, strlen(key), C + i ) << std::endl;
    }

    //Try with Base32
    int base32Len = strlen(key);
    int secretLen = (base32Len*5+7)/8;
    uint8_t* key_base32 = (uint8_t*)malloc(base32Len + 1);
    memcpy(key_base32, key, base32Len);
    key_base32[base32Len] = '\000';
    secretLen = base32_decode(key_base32, key_base32, base32Len);
    std::cout << "Key (Base32): " << key_base32 << std::endl;
 
    for (int i = -((window-1)/2); i <= window/2; ++i)
    {
        std::cout << "HASH (BASE32):" << compute_code(key_base32, secretLen, C + i ) << std::endl;
    }


/*
    //int64_t C;
    std::stringstream ssC;
    ssC << C;

    auto now = std::chrono::system_clock::now();


    //char key[] = "4WZU6J7X5UVFTY765T2AIP7GOQ";
    char* data = nullptr;
    auto H = HMAC(EVP_sha1(), key, strlen(key), (unsigned char*)ssC.str().c_str(), ssC.str().size(), NULL, NULL);
    std::cout << "HAS2: " << H << std::endl;*/
}

int main(int argc, char** argv)
{
    test();
return 0;
    if(argc > 1 && strcmp(argv[1], "-v") == 0)
    {
        logging::set_verbose(true);
        logging::log("Verbose mode activated", true);
    }

    conf conf_file("pswmgrd.conf");
    if(!PasswordManagerServer::Instance()->Init(conf_file))
        return -1;

    if(!PasswordManagerServer::Instance()->Run(conf_file))
        return -2;

    return 0;
}
