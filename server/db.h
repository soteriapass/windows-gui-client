#pragma once

#include "conf.h"

class db
{
public:
    virtual bool Init(conf& conf_file) = 0;
    virtual int GetUserCount() const = 0;
    virtual bool InsertUser(int id, const std::string& username, const std::string& password, const std::string& salt, int iterations, bool admin) = 0;
};
