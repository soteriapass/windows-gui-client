#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <grpc++/grpc++.h>
#include "client.h"

enum CMD_ACTIONS
{
    ACTION_ADDUSER,
    ACTION_AUTHENTICATE,
    ACTION_HELP,
};

auto get_channel(const std::string& address)
{
    std::ifstream file("/home/mfilion/programming/pswmgr/easy-rsa/keys/ca.crt", std::ifstream::in);
    std::string ca = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    auto credOptions = grpc::SslCredentialsOptions();
    credOptions.pem_root_certs = ca;
    auto sslCreds = grpc::SslCredentials(credOptions);
    auto channel = grpc::CreateChannel(address, sslCreds);
    return channel;
}

bool add_user(const std::string& user, const std::string& pass, const std::string& new_user, const std::string& new_pass)
{
    PasswordManagerClient client( get_channel("localhost:6060") );
    if(!client.Authenticate(user, pass, true))
    {
        std::cerr << client.GetLastError() << std::endl;
        return false;
    }
    if(!client.CreateUser(new_user, new_pass))
    {
        std::cerr << "Could not create user" << std::endl;
        std::cerr << client.GetLastError() << std::endl;
        return false;
    }
    return true;
}

bool login(const std::string& user, const std::string& pass)
{
    PasswordManagerClient client( get_channel("localhost:5050") );
    if(!client.Authenticate(user, pass, false))
    {
        std::cerr << client.GetLastError() << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    std::vector<CMD_ACTIONS> actions;

    std::string user;
    std::string pass;
    std::string new_user;
    std::string new_pass;

    for(int i = 0; i < argc; ++i)
    {
        if(strcmp(argv[i], "-add_user") == 0 || strcmp(argv[i], "-a") == 0) 
        {
            actions.push_back(ACTION_ADDUSER);
        }
        else if(strcmp(argv[i], "-login") == 0 || strcmp(argv[i], "-l") == 0)
        {
            actions.push_back(ACTION_AUTHENTICATE);
        }
        else if(strcmp(argv[i], "-user") == 0 || strcmp(argv[i], "-u") == 0)
        {
            if(i+1 < argc)
                user = argv[i+1];
        }
        else if(strcmp(argv[i], "-pass") == 0 || strcmp(argv[i], "-p") == 0)
        {
            if(i+1 < argc)
                pass = argv[i+1];
        }
        else if(strcmp(argv[i], "-new_user") == 0)
        {
            if(i+1 < argc)
                new_user = argv[i+1];
        }
        else if(strcmp(argv[i], "-new_pass") == 0)
        {
            if(i+1 < argc)
                new_pass = argv[i+1];
        }

    }

    if(actions.empty() && !user.empty() && !pass.empty())
    {
        actions.push_back(ACTION_AUTHENTICATE);
    }
    if(actions.empty())
    {
        actions.push_back(ACTION_HELP);
    }
    for(auto a : actions)
    {
        switch(a)
        {
            case ACTION_ADDUSER:
            {
                if(new_user.empty())
                {
                    std::cerr << "username not specified" << std::endl;
                }
                else if(new_pass.empty())
                {
                    std::cerr << "password not specified" << std::endl;
                }
                else if(!add_user(user, pass, new_user, new_pass))
                {
                    return 1;
                }
                break;
            }
            case ACTION_AUTHENTICATE:
            {
                if(!login(user, pass))
                {
                    return 2;
                }
                std::cout << "Authenticated" << std::endl;
            }
            case ACTION_HELP:
            {
                break;
            }
        }
    }
}
