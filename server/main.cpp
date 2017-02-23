#include "server.h"
#include "conf.h"
#include "encryption_utils.h"
#include "log.h"

#include <sstream>
#include <signal.h>

#include <chrono>
#include "hmac.h"
#include "sha1.h"
#include "base32.h"

#include "sqlite_db.h"

void signal_callback_handler(int signum)
{
    std::stringstream ss;
    ss << "Caught signal " << signum;
    logging::log(ss.str(), false);

    PasswordManagerServer::Instance()->Destroy();

    exit(signum);
}

int main(int argc, char** argv)
{
    signal(SIGINT, signal_callback_handler);

    if(argc > 1 && strcmp(argv[1], "-v") == 0)
    {
        logging::set_verbose(true);
        logging::log("Verbose mode activated", true);
    }

    std::string conf_path = "pswmgrd.conf";
    if(argc == 2)
    {
    	conf_path = argv[1];
        logging::log(conf_path, true);
    }

    conf conf_file(conf_path);
    if(!logging::init(conf_file))
        return false;

    if(!PasswordManagerServer::Instance()->Init(conf_file))
        return -1;

    if(!PasswordManagerServer::Instance()->Run(conf_file))
        return -2;

    return 0;
}
