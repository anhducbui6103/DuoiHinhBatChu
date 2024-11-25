#ifndef DATABASE_H
#define DATABASE_H

#include "/opt/lampp/include/mysql.h"

// Cấu trúc để lưu kết nối database
typedef struct
{
    MYSQL *connection;
} Database;

// Hàm mở kết nối
void connectToDatabase(Database *db, const char *host, const char *user, const char *password, const char *dbname, unsigned int port);

// Hàm đóng kết nối
void disconnectDatabase(Database *db);

// Truy xuất kết nối cho các module khác
MYSQL *getDatabaseConnection(Database *db);

#endif // DATABASE_H
