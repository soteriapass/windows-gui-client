#include "sqlite_db.h"

#include <iostream>
#include <sstream>
#include <string.h>

static int GenericCallback(void*, int argc, char** argv, char** azColName)
{
    for(int i = 0; i < argc; ++i)
    {
        std::string param = argv[i] ? argv[i] : "NULL";
        std::cerr << azColName[i] << " = " << param << std::endl;
    }
    std::cerr << std::endl;
    return 0;
}

bool sqlite_db::Init(conf& conf_file)
{
    if(m_Database != nullptr)
        return false;

    char* err = nullptr;
    int rc = sqlite3_open(conf_file.get_database_file().c_str(), &m_Database);
    if(rc != SQLITE_OK)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(m_Database) << std::endl;
        return false;
    }

    const std::string create_sql = "CREATE TABLE IF NOT EXISTS USERS(ID INT PRIMARY KEY NOT NULL, USERNAME TEXT NOT NULL, PASSWORD CHAR(64) NOT NULL, SALT CHAR(16) NOT NULL, ITERATIONS INT NOT NULL, ADMIN BOOLEAN NOT NULL);";
    rc = sqlite3_exec(m_Database, create_sql.c_str(), GenericCallback, nullptr, &err);
    if(rc != SQLITE_OK)
    {
        std::cerr << "Can't create table: " << err << std::endl;
        return false;
    }

    return true;
}

int sqlite_db::GetUserCount() const
{
    int userCount = 0;
    auto callbackFunc = [](void* unknown, int argc, char** argv, char** azColName) -> int
    {
        if(unknown != nullptr)
        {
            *reinterpret_cast<int*>(unknown) += 1;
        }
        return 0;
    };
    std::string sql = "SELECT * FROM USERS";
    const char* data = "Callback function called";
    char* err = nullptr;
    int rc = sqlite3_exec(m_Database, sql.c_str(), callbackFunc, reinterpret_cast<void*>(&userCount), &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "sql error: " << err << std::endl;
        return -1;
    }
    return userCount;
}

bool sqlite_db::InsertUser(int id, const std::string& username, const std::string& password, const std::string& salt, int iterations, bool admin)
{
    std::stringstream insert_sql;
    insert_sql << "INSERT INTO USERS(ID, USERNAME, PASSWORD, SALT, ITERATIONS, ADMIN)";
    insert_sql << "VALUES (" << id << ",'" << username << "','" << password << "','" << salt << "'," << iterations << "," << admin << ")";

    std::cout << "sql: " << insert_sql.str() << std::endl;

    char* err = nullptr;
    int rc = sqlite3_exec(m_Database, insert_sql.str().c_str(), GenericCallback, 0, &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "(sqlite3 error) " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool sqlite_db::ValidPasswordForUser(const std::string& username, const std::string& password)
{
    std::stringstream sql;
    sql << "SELECT PASSWORD FROM USERS WHERE USERNAME == '" << username << "'";

    auto callbackFunc = [](void* unknown, int argc, char** argv, char** azColName) -> int
    {
        if(unknown != nullptr && argc > 0)
        {
            char* buffer = reinterpret_cast<char*>(unknown);
            memcpy(buffer, argv[0], strlen(argv[0]));
        }
        return 0;
    };

    char* err = nullptr;
    char buffer[129];
    memset(buffer, 0, 129);
    int rc = sqlite3_exec(m_Database, sql.str().c_str(), callbackFunc, reinterpret_cast<void*>(buffer), &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "(sqlite error) " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return strcmp(buffer, password.c_str()) == 0;
}

std::string sqlite_db::GetSaltForUser(const std::string& username)
{
    std::stringstream sql;
    sql << "SELECT SALT FROM USERS WHERE USERNAME == '" << username << "'";

    auto callbackFunc = [](void* unknown, int argc, char** argv, char** azColName) -> int
    {
        if(unknown != nullptr && argc > 0)
        {
            char* buffer = reinterpret_cast<char*>(unknown);
            memcpy(buffer, argv[0], strlen(argv[0]));
        }
        return 0;
    };

    char* err = nullptr;
    char buffer[17];
    memset(buffer, 0, 17);
    int rc = sqlite3_exec(m_Database, sql.str().c_str(), callbackFunc, reinterpret_cast<void*>(buffer), &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "(sqlite error) " << err << std::endl;
        sqlite3_free(err);
        return {};
    }

    return { buffer };
}
