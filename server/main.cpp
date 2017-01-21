#include "pswmgr_server.h"

int main()
{
    if(!PasswordManagerServer::Instance()->Init())
        return -1;

    if(!PasswordManagerServer::Instance()->Run())
        return -2;

    return 0;
}
