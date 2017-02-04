#include "server.h"
#include "conf.h"
#include "encryption_utils.h"
#include "log.h"

#include <sstream>

#include <chrono>
#include "hmac.h"
#include "sha1.h"
#include "base32.h"

#include "sqlite_db.h"

int main(int argc, char** argv)
{
    if(argc > 1 && strcmp(argv[1], "-v") == 0)
    {
        logging::set_verbose(true);
        logging::log("Verbose mode activated", true);
    }

    conf conf_file("pswmgrd.conf");

    char plainText[2048/8] = "Hello this is Ravi";
    unsigned char encrypted[4098] = { };
    unsigned char decrypted[4098] = { };
    encryption::EncryptString(plainText, encrypted, conf_file.get_public_key_filename());

    auto database = new sqlite_db();
    database->Init(conf_file);
 
//    database->AddPassword(1, "test", "user", { reinterpret_cast<char*>(encrypted) }, "123");
    auto add_callback = [](char* a, char* b, char* c, char* d, void*)
    {
    //    std::cout << a << std::endl;
    //    std::cout << b << std::endl;
//        std::cout << c << std::endl;
    //    std::cout << d << std::endl;
        unsigned char decrypted[4098] = { };
        encryption::DecryptString(reinterpret_cast<unsigned char*>(c), decrypted, "/home/mfilion/programming/pswmgr/keys/private.pem");
       std::cout << decrypted << std::endl;
    };
    database->ListPasswords(1, add_callback, nullptr); 

    //encryption::DecryptString(encrypted, decrypted, conf_file.get_private_key_filename());
 //   std::cout << decrypted << std::endl;
return 0;

    if(!PasswordManagerServer::Instance()->Init(conf_file))
        return -1;

    if(!PasswordManagerServer::Instance()->Run(conf_file))
        return -2;

    return 0;
}
