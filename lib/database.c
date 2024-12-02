#include "database.h"
#include <stdio.h>
#include <stdlib.h>

// Hàm mở kết nối đến cơ sở dữ liệu
void connectToDatabase(Database *db, const char *host, const char *user, const char *password, const char *dbname, unsigned int port)
{
    db->connection = mysql_init(NULL);
    if (db->connection == NULL)
    {
        fprintf(stderr, "mysql_init() failed\n");
        exit(EXIT_FAILURE);
    }

    if (mysql_real_connect(db->connection, host, user, password, dbname, port, NULL, 0) == NULL)
    {
        fprintf(stderr, "mysql_real_connect() failed\nError: %s\n", mysql_error(db->connection));
        mysql_close(db->connection);
        exit(EXIT_FAILURE);
    }

    printf("Database connected successfully!\n");
}

// Hàm đóng kết nối
void disconnectDatabase(Database *db)
{
    if (db->connection != NULL)
    {
        mysql_close(db->connection);
        printf("Database connection closed.\n");
    }
}

// Hàm truy xuất kết nối cho các module khác
MYSQL *getDatabaseConnection(Database *db)
{
    return db->connection;
}
