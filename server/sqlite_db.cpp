#include "sqlite_db.h"

#include <iostream>
#include <sstream>
#include <string.h>

#include "log.h"

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
    logging::log("sqlite_db::Init", true);
    if(m_Database != nullptr)
        return false;

    char* err = nullptr;
    int rc = sqlite3_open(conf_file.get_database_file().c_str(), &m_Database);
    if(rc != SQLITE_OK)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(m_Database) << std::endl;
        return false;
    }

    const std::string create_user_table_sql = "CREATE TABLE IF NOT EXISTS USERS(ID INT PRIMARY KEY NOT NULL, USERNAME TEXT NOT NULL, PASSWORD CHAR(64) NOT NULL, SALT CHAR(16) NOT NULL, ITERATIONS INT NOT NULL, ADMIN BOOLEAN NOT NULL);";
    rc = sqlite3_exec(m_Database, create_user_table_sql.c_str(), GenericCallback, nullptr, &err);
    if(rc != SQLITE_OK)
    {
        std::cerr << "Can't create users table: " << err << std::endl;
        return false;
    }

    const std::string create_pswd_table_sql = "CREATE TABLE IF NOT EXISTS PASSWORDS(ID INT PRIMARY KEY NOT NULL, USER_ID INT NOT NULL, ACCOUNT_NAME VARCHAR(512) NOT NULL, USERNAME VARCHAR(512) NOT NULL, PASSWORD VARCHAR(512) NOT NULL, EXTRA VARCHAR(512) NOT NULL);";
    rc = sqlite3_exec(m_Database, create_pswd_table_sql.c_str(), GenericCallback, nullptr, &err);
    if(rc != SQLITE_OK)
    {
        std::cerr << "Can't create passwords table: " << err << std::endl;
        return false;
    }

    return true;
}

int sqlite_db::GetUserCount() const
{
    logging::log("sqlite_db::GetUserCount", true);
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
    logging::log("sqlite_db::InsertUser", true);
    std::stringstream insert_sql;
    insert_sql << "INSERT INTO USERS(ID, USERNAME, PASSWORD, SALT, ITERATIONS, ADMIN)";
    insert_sql << "VALUES (" << id << ",'" << username << "','" << password << "','" << salt << "'," << iterations << "," << admin << ")";

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
    logging::log("sqlite_db::ValidPasswordForUser", true);
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
    logging::log("sqlite_db::GetSaltForUser", true);
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

int sqlite_db::GetUserId(const std::string& username)
{
    logging::log("sqlite_db::GetUserId", true);
    std::stringstream sql;
    sql << "SELECT ID FROM USERS WHERE  USERNAME == '" << username << "'";

    auto callbackFunc = [](void* unknown, int argc, char** argv, char** azColName) -> int
    {
        if(unknown != nullptr && argc > 0)
        {
            int* buffer = reinterpret_cast<int*>(unknown);
            *buffer = std::stoi({ argv[0] });
        }
        return 0;
    };

    char* err = nullptr;
    int data = 0;
    int rc = sqlite3_exec(m_Database, sql.str().c_str(), callbackFunc, reinterpret_cast<void*>(&data), &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "(sqlite error) " << err << std::endl;
        sqlite3_free(err);
        return 0;
    }

    return data;
}

bool sqlite_db::AddPassword(int userId, const std::string& accountName, const std::string& username, const std::string& password, const std::string& extra)
{
    logging::log("sqlite_db::AddPassword", true);
    int id = GetPasswordEntryCount()+1;
    std::stringstream insert_sql;
    insert_sql << "INSERT INTO PASSWORDS(ID, USER_ID, ACCOUNT_NAME, USERNAME, PASSWORD, EXTRA) ";
    insert_sql << "VALUES (" << id << ","<< userId << ",'" << accountName << "','" << username << "','" << password << "','" << extra << "')";

    logging::log("sql: " + insert_sql.str(), true);

    char* err = nullptr;
    int data = 0;
    int rc = sqlite3_exec(m_Database, insert_sql.str().c_str(), nullptr, nullptr, &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "(sqlite error) " << err << std::endl;
        sqlite3_free(err);
        return false;
    }

    return true;
}

int sqlite_db::GetPasswordEntryCount() const
{
    logging::log("sqlite_db::GetPasswordEntryCount", true);
    int userCount = 0;
    auto callbackFunc = [](void* unknown, int argc, char** argv, char** azColName) -> int
    {
        if(unknown != nullptr)
        {
            *reinterpret_cast<int*>(unknown) += 1;
        }
        return 0;
    };
    std::string sql = "SELECT * FROM PASSWORDS";
    char* err = nullptr;
    int rc = sqlite3_exec(m_Database, sql.c_str(), callbackFunc, reinterpret_cast<void*>(&userCount), &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "sql error: " << err << std::endl;
        sqlite3_free(err);
        return -1;
    }
    return userCount;
}

bool sqlite_db::DeletePassword(int userId, const std::string& accountName)
{
    std::stringstream update_sql;
    update_sql << "UPDATE PASSWORDS set USER_ID = 0, ACCOUNT_NAME = '', USERNAME = '', PASSWORD = '', EXTRA = '' WHERE USER_ID == " << userId << " AND ACCOUNT_NAME == '" << accountName << "';";
    char* err = nullptr;
    int rc = sqlite3_exec(m_Database, update_sql.str().c_str(), nullptr, nullptr, &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "sql error: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool sqlite_db::ModifyPassword(int userId, const std::string& accountName, const std::string& password)
{
    std::stringstream update_sql;
    update_sql << "UPDATE PASSWORDS set PASSWORD = '" << password << "' WHERE USER_ID == " << userId << " AND ACCOUNT_NAME == '" << accountName << "';";
    char* err = nullptr;
    int rc = sqlite3_exec(m_Database, update_sql.str().c_str(), nullptr, nullptr, &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "sql error: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}

bool sqlite_db::ListPasswords(int userId, void (*add_callback)(char*,char*,char*,char*,void*), void* cookie)
{
    struct list_callback_info_t 
    {
        void (*add_callback)(char*,char*,char*,char*,void*);
        void* cookie;
    };

    auto callbackFunc = [](void* unknown, int argc, char** argv, char** azColName) -> int
    {
        if(unknown != nullptr && argc == 4)
        {
            list_callback_info_t* callbackInfo = reinterpret_cast<list_callback_info_t*>(unknown);
            callbackInfo->add_callback(argv[0], argv[1], argv[2], argv[3], callbackInfo->cookie);
        }
        return 0;
    };

    list_callback_info_t callbackInfo;
    callbackInfo.add_callback = add_callback;
    callbackInfo.cookie = cookie;

    std::stringstream sql;
    sql << "SELECT ACCOUNT_NAME, USERNAME, PASSWORD, EXTRA FROM PASSWORDS WHERE USER_ID == " << userId << ";";

    char* err = nullptr;
    int rc = sqlite3_exec(m_Database, sql.str().c_str(), callbackFunc, &callbackInfo, &err);
    if( rc != SQLITE_OK )
    {
        std::cerr << "sql error: " << err << std::endl;
        sqlite3_free(err);
        return false;
    }
    return true;
}
