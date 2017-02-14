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

    std::string conf_path = "pswmgrd.conf";
    if(argc == 2)
    {
    	conf_path = argv[1];
        std::cout << conf_path << std::endl;
    }

    conf conf_file(conf_path);

    if(!PasswordManagerServer::Instance()->Init(conf_file))
        return -1;

    if(!PasswordManagerServer::Instance()->Run(conf_file))
        return -2;

    return 0;
}
