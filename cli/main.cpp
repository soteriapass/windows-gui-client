#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <grpc++/grpc++.h>

#include "client.h"
#include "conf.h"

enum CMD_ACTIONS
{
    ACTION_ADDUSER,
    ACTION_AUTHENTICATE,
    ACTION_HELP,
};

bool add_user(PasswordManagerClient& client, const std::string& new_user, const std::string& new_pass)
{
    if(!client.CreateUser(new_user, new_pass))
    {
        std::cerr << "Could not create user" << std::endl;
        std::cerr << client.GetLastError() << std::endl;
        return false;
    }
    return true;
}

bool login(PasswordManagerClient& client, const std::string& user, const std::string& pass)
{
    if(!client.Authenticate(user, pass, false))
    {
        std::cerr << client.GetLastError() << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char** argv)
{
    conf conf_file("pswmgr.conf");

    std::vector<CMD_ACTIONS> actions;

    std::string user;
    std::string new_user;

    PasswordManagerClient client(conf_file, PasswordManagerClient::GetChannel(conf_file, conf_file.get_authentication_address_and_port()));

    for(int i = 0; i < argc; ++i)
    {
        if(strcmp(argv[i], "-add_user") == 0 || strcmp(argv[i], "-a") == 0) 
        {
            actions.push_back(ACTION_AUTHENTICATE);
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
        else if(strcmp(argv[i], "-new_user") == 0)
        {
            if(i+1 < argc)
                new_user = argv[i+1];
        }
    }

    if(actions.empty() && !user.empty())
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
                else
                {
                    std::string new_pass;
                    std::string new_pass_confirm;
                    while(new_pass.empty() || new_pass != new_pass_confirm)
                    {
                        std::cout << "password for " << new_user << ":";
                        std::cin >> new_pass;
 
                        std::cout << "confirm password for " << new_user << ":";
                        std::cin >> new_pass_confirm;
                    };
                    if(!add_user(client, new_user, new_pass))
                    {
                        return 1;
                    }
                }
                break;
            }
            case ACTION_AUTHENTICATE:
            {
                std::string pass;
                if(!user.empty())
                {
                    std::cout << "password for " << user << ":";
                    std::cin >> pass;
                }
                if(!login(client, user, pass))
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
