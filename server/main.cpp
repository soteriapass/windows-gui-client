#include "pswmgr_server.h"

int main()
{
    return PasswordManagerServerImpl::Instance()->Run() ? 0 : 1;
}
