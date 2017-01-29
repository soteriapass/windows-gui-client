#include "server.h"
#include "conf.h"
#include "encryption_utils.h"
#include "log.h"

#include <sstream>

#include <chrono>
#include "hmac.h"
#include "sha1.h"
#include "base32.h"

void test()
{
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
 
    auto intervals = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() / 30;

    char key[] = "PL27LSK6PRNZHKX5VMKVFQZTYM";
    int window = 17;
    int base32Len = strlen(key);
    int secretLen = (base32Len*5+7)/8;
    uint8_t* key_base32 = (uint8_t*)malloc(base32Len + 1);
    memcpy(key_base32, key, base32Len);
    key_base32[base32Len] = '\000';
    secretLen = base32_decode(key_base32, key_base32, base32Len);
 
    for (int i = -((window-1)/2); i <= window/2; ++i)
    {
        std::cout << "HASH (BASE32):" << encryption::ComputeCode(key_base32, secretLen, intervals + i ) << std::endl;
    }
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
