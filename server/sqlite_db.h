#pragma once

#include "db.h"

#include <sqlite3.h>

class sqlite_db : public db
{
public:
    virtual bool Init(conf& conf_file) override;
    virtual int GetUserCount() const override;
    virtual bool InsertUser(int id, const std::string& username, const std::string& password, const std::string& salt, int iterations, bool admin) override;
private:
    sqlite3* m_Database;
};
