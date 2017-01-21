#include <stdio.h>
#include <sqlite3.h> 
#include <string>
#include <string.h>

sqlite3 *db;
char *zErrMsg = 0;

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
    printf("callback\n");
    for(int i = 0; i < argc; ++i)
    {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}

bool open_db()
{
    int rc = sqlite3_open("test.db", &db);
    if(rc == SQLITE_OK)
    {
        fprintf(stderr, "Opened database successfully\n");
    }
    else
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return false;
    }

    return true;
}

bool create_table()
{
    const std::string sql = "CREATE TABLE IF NOT EXISTS COMPANY(ID INT PRIMARY KEY NOT NULL, NAME TEXT NOT NULL, AGE INT NOT NULL, ADDRESS CHAR(50), SALARY REAL);";
    int rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        fprintf(stdout, "Table created successfully\n");
        return true;
    }
}

void query_data()
{
    const std::string sql = "SELECT * from COMPANY";
    const char* data = "Callback function called";
    int rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK )
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    else
    {
        fprintf(stdout, "Operation done successfully\n");
    }
}

bool insert_data()
{
    const std::string sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

    int rc = sqlite3_exec(db, sql.c_str(), callback, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        return false;
    }
    else
    {
        fprintf(stdout, "Records created successfully\n");
    }

    return true;
}

int main(int argc, char* argv[])
{
    char *zErrMsg = 0;
    if(!open_db())
    {
        return -1;
    }

    if(!create_table())
    {
        return -2;
    }

    if(argc == 2 && strcmp(argv[1], "-i") == 0)
    {
        fprintf(stdout, "Insert data\n");
        insert_data();
    }
    if(argc == 2 && strcmp(argv[1], "-o") == 0)
    {
        fprintf(stdout, "Printing data\n");
        query_data();
    }

    sqlite3_close(db);
    return 0;
}
