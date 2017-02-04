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

    if(!PasswordManagerServer::Instance()->Init(conf_file))
        return -1;

    if(!PasswordManagerServer::Instance()->Run(conf_file))
        return -2;

    return 0;
}
