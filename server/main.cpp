#include "server.h"
#include "conf.h"

int main()
{
    conf conf_file("pswmgrd.conf");
    if(!PasswordManagerServer::Instance()->Init(conf_file))
        return -1;

    if(!PasswordManagerServer::Instance()->Run(conf_file))
        return -2;

    return 0;
}
