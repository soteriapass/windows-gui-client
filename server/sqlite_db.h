#pragma once

#include "db.h"

#include <sqlite3.h>

class sqlite_db final : public db
{
public:
    bool Init(conf& conf_file) override;
    int GetUserCount() const override;
    bool InsertUser(int id, const std::string& username, const std::string& password, const std::string& salt, int iterations, bool admin) override;
    bool Insert2FA(int id, const std::string& secret, std::vector<int> scratchCodes) override;
    bool ValidPasswordForUser(const std::string& username, const std::string& password) override;
    std::string GetSaltForUser(const std::string& username) override;
    int GetUserId(const std::string& username) override;
    bool Get2FA(int userId, std::string& token) override;
    bool AddPassword(int userId, const std::string& accountName, const std::string& username, const std::string& password, const std::string& extra) override;
    bool AddPassword(int userId, const std::string& accountName, const std::string& username, const char* enc_password, int enc_length, const std::string& extra) override;
    bool DeletePassword(int userId, const std::string& accountName) override;
    bool ModifyPassword(int userId, const std::string& accountName, const std::string& password) override;
    bool ListPasswords(int userId, void (*add_callback)(char*,char*,char*,char*,void*), void* cookie) override;
private:
   int GetPasswordEntryCount() const;
private:
    sqlite3* m_Database;
};
