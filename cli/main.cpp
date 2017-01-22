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
    ACTION_ADDPASSWORD,
    ACTION_AUTHENTICATE,
    ACTION_DELETEPASSWORD,
    ACTION_HELP,
    ACTION_LISTPASSWORDS,
    ACTION_MODIFYPASSWORD,
};

bool add_user(PasswordManagerClient& client, const std::string& new_user)
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

        if(!client.CreateUser(new_user, new_pass))
        {
            std::cerr << "Could not create user" << std::endl;
            std::cerr << client.GetLastError() << std::endl;
            return false;
        }
    }
    return true;
}

bool login(PasswordManagerClient& client, const std::string& user)
{
    std::string pass;
    if(!user.empty())
    {
        std::cout << "password for " << user << ":";
        std::cin >> pass;
    }
    if(!client.Authenticate(user, pass, false))
    {
        std::cerr << client.GetLastError() << std::endl;
        return false;
    }
    std::cout << "Authenticated" << std::endl;
    return true;
}

bool add_password(PasswordManagerClient& client)
{
    std::string account_name;
    std::cout << "account name: ";
    std::cin >> account_name;

    std::string username;
    std::cout << "username: ";
    std::cin >> username;

    std::string password;
    std::cout << "password: ";
    std::cin >> password;

    std::string extra;
    std::cout << "extra: ";
    std::cin >> extra;

    if(!client.AddPassword(account_name, username, password, extra))
    {
        std::cerr << client.GetLastError() << std::endl;
        return false;
    }
    return true;
}

bool list_passwords(PasswordManagerClient& client)
{
    if(!client.ListPasswords())
    {
        return false;
    }
    return true;
}

bool delete_password(PasswordManagerClient& client)
{
    std::string account_name;
    std::cout << "account name: ";
    std::cin >> account_name;

    if(!client.DeletePassword(account_name))
    {
        return false;
    }
    return true;
}

bool modify_password(PasswordManagerClient& client)
{
    std::string account_name;
    std::cout << "account name: ";
    std::cin >> account_name;

    std::string new_account_password;
    std::cout << "password: ";
    std::cin >> new_account_password;

    if(!client.ModifyPassword(account_name, new_account_password))
    {
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
        else if(strcmp(argv[i], "-new_password") == 0)
        {
            actions.push_back(ACTION_ADDPASSWORD);
        }
        else if(strcmp(argv[i], "-list") == 0)
        {
            actions.push_back(ACTION_LISTPASSWORDS);
        }
        else if(strcmp(argv[i], "-delete_password") == 0)
        {
            actions.push_back(ACTION_DELETEPASSWORD);
        }
        else if(strcmp(argv[i], "-modify_password") == 0)
        {
            actions.push_back(ACTION_MODIFYPASSWORD);
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
                if(!add_user(client, new_user))
                {
                    return 1;
                }
                break;
            }
            case ACTION_ADDPASSWORD:
            {
                add_password(client);
                break;
            }
            case ACTION_AUTHENTICATE:
            {
                if(!login(client, user))
                {
                    return 2;
                }
                break;
            }
            case ACTION_HELP:
            {
                std::cout << "Password Manager Command Line Interface" << std::endl;
                std::cout << "v1.0.0.0" << std::endl;
                std::cout << std::endl;
                std::cout << "TO BE COMPLETED" << std::endl;
                break;
            }
            case ACTION_LISTPASSWORDS:
            {
                if(!list_passwords(client))
                {
                    return 3;
                }
            }
        }
    }
}
