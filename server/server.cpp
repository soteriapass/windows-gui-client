#include "pswmgr_server.h"

#include <thread>
#include <string>
#include <sstream>

// Static Member Initialization
PasswordManagerServer* PasswordManagerServer::ms_Instance = nullptr;

// Static Fucntions
static int callback(void*, int argc, char** argv, char** azColName)
{
    for(int i = 0; i < argc; ++i)
    {
        std::string param = argv[i] ? argv[i] : "NULL";
        std::cerr << azColName[i] << " = " << param << std::endl;
    }
    std::cerr << std::endl;
    return 0;
}

PasswordManagerServer::PasswordManagerServer()
: m_IsRunning(false)
, m_Database(nullptr)
{
}

PasswordManagerServer::~PasswordManagerServer()
{
}

grpc::Status PasswordManagerServer::Authenticate(grpc::ServerContext* context, const pswmgr::AuthenticationRequest* request, pswmgr::SimpleReply* response)
{
    //First check to see if there are any users
    //I f there are none then we can simply say whatever the resul
    //The authentication has succeeded
    {
        int userCount = 0;
        auto callbackFunc = [](void* unknown, int argc, char** argv, char** azColName) -> int
        {
            if(unknown != nullptr)
            {
                *reinterpret_cast<int*>(unknown) = argc;
            }
            return 0;
        };
        std::string sql = "SELECT * FROM USERS";
        const char* data = "Callback function called";
        char* err = nullptr;
        int rc = sqlite3_exec(m_Database, sql.c_str(), callbackFunc, (void*)userCount, &err);
        if( rc != SQLITE_OK )
        {
            std::cerr << "sql error: " << err << std::endl;
            return grpc::Status::OK;
        }
        if(userCount == 0)
        {
            response->set_sucess(true);
            return grpc::Status::OK;
        }
    }

    {
        std::stringstream sql;
        sql << "SELECT ID, USERNAME, PASSWORD, ADMIN WHERE USERNAME == \"" << request->username() << "\"";
    }
    return grpc::Status::OK;
}

grpc::Status PasswordManagerServer::CreateUser(grpc::ServerContext* context, const pswmgr::UserCreationRequest* request, pswmgr::SimpleReply* response)
{
    return grpc::Status::OK;
}

PasswordManagerServer* PasswordManagerServer::Instance()
{
    if(ms_Instance == nullptr)
    {
        ms_Instance = new PasswordManagerServer();
    }
    return ms_Instance;
}

bool PasswordManagerServer::Init(conf& conf_file)
{
    if(m_Database != nullptr)
        return false;

    char* err = nullptr;
    int rc = sqlite3_open("pswmgr.db", &m_Database);
    if(rc != SQLITE_OK)
    {
        std::cerr << "Can't open database: " << sqlite3_errmsg(m_Database) << std::endl;
        return false;
    }

    const std::string create_sql = "CREATE TABLE IF NOT EXISTS USERS(ID INT PRIMARY KEY NOT NULL, USERNAME TEXT NOT NULL, PASSWORD CHAR(256) NOT NULL, BOOLEAN ADMIN NOT NULL);";
    rc = sqlite3_exec(m_Database, create_sql.c_str(), callback, nullptr, &err);
    if(rc != SQLITE_OK)
    {
        std::cerr << "Can't create table: " << err << std::endl;
        return false;
    }
    return true;
} 

bool PasswordManagerServer::Run(conf& conf_file)
{
    if(m_Database == nullptr)
    {
        std::cerr << "Database has not been opened" << std::endl;
        return false;
    }

    if(m_IsRunning)
    {
        std::cerr << "Server already running" << std::endl;
        return false;
    }

    std::unique_ptr<grpc::Server> passwordServer;
    {
        const std::string server_address(conf_file.get_password_manager_address_and_port());
        grpc::ServerBuilder builder;

        auto credOptions = grpc::SslServerCredentialsOptions();
        auto channelCreds = grpc::SslServerCredentials(credOptions);

        builder.AddListeningPort(server_address, channelCreds);
        builder.RegisterService(static_cast<pswmgr::PasswordManager::Service*>(this));

        passwordServer = builder.BuildAndStart();
        std::cout << "Password server listening on " << server_address << std::endl;
    }
    
    std::unique_ptr<grpc::Server> userMgmtServer;
    {
        const std::string server_address(conf_file.get_user_mangement_address_and_port());
        grpc::ServerBuilder builder;

        auto credOptions = grpc::SslServerCredentialsOptions();
        auto channelCreds = grpc::SslServerCredentials(credOptions);

        builder.AddListeningPort(server_address, channelCreds);
        builder.RegisterService(static_cast<pswmgr::UserManagement::Service*>(this));

        userMgmtServer = builder.BuildAndStart();
        std::cout << "User management server listening on " << server_address << std::endl;
    }

    m_IsRunning = true;

    passwordServer->Wait();
    userMgmtServer->Wait();

    return true;
}
