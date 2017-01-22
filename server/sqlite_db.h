#pragma once

#include "db.h"

#include <sqlite3.h>

class sqlite_db final : public db
{
public:
    bool Init(conf& conf_file) override;
    int GetUserCount() const override;
    bool InsertUser(int id, const std::string& username, const std::string& password, const std::string& salt, int iterations, bool admin) override;
    bool ValidPasswordForUser(const std::string& username, const std::string& password) override;
    std::string GetSaltForUser(const std::string& username) override;
private:
    sqlite3* m_Database;
};
