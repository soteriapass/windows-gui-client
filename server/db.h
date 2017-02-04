#pragma once

#include "conf.h"
#include <vector>

class db
{
public:
    //Basic intialization
    virtual bool Init(conf& conf_file) = 0;

    //User functions
    virtual int GetUserCount() const = 0;
    virtual bool InsertUser(int id, const std::string& username, const std::string& password, const std::string& salt, int iterations, bool admin) = 0;
    virtual bool Insert2FA(int id, const std::string& secret, std::vector<int> scratchCodes) = 0;
    virtual bool ValidPasswordForUser(const std::string& username, const std::string& password) = 0;
    virtual std::string GetSaltForUser(const std::string& username) = 0;
    virtual int GetUserId(const std::string& username) = 0;
    virtual bool Get2FA(int userId, std::string& token) = 0;
    virtual bool UpdateUserPassword(int userId, const std::string& hashedPassword) = 0;

    //Password access
    virtual bool AddPassword(int userId, const std::string& accountName, const std::string& username, const std::string& password, const std::string& extra) = 0;
    virtual bool AddPassword(int userId, const std::string& accountName, const std::string& username, const char* enc_password, int enc_length, const std::string& extra) = 0;
    virtual bool DeletePassword(int userId, const std::string& accountName) = 0;
    virtual bool ModifyPassword(int userId, const std::string& accountName, const std::string& username, const std::string& password, const std::string& extra) = 0;
    virtual bool ModifyPassword(int userId, const std::string& accountName, const std::string& username, const char* enc_password, int enc_length, const std::string& extra) = 0;
    virtual bool ListPasswords(int userId, void (*add_callback)(char*,char*,char*,char*,void*), void* cookie) = 0;

};
