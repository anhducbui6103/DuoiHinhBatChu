#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "account.h"
#include "../../lib/protocol.h"
#include "../../lib/database.h"
#include "/opt/lampp/include/mysql.h"

#define USERNAME_SIZE 50
#define PASSWORD_SIZE 50

void freeUser(User *user)
{
    if (user == NULL)
        return;

    user->id = 0;
    memset(user->username, 0, sizeof(user->username));
    user->room_id = 0;
    user->state = 0;
    user->score = 0;
    user->socket_fd = 0;
}

int authenticateUser(Database *db, char *buffer, User *user)
{
    MYSQL *conn = getDatabaseConnection(db);
    MYSQL_RES *res;
    MYSQL_ROW row;
    char username[USERNAME_SIZE], password[PASSWORD_SIZE];
    char *p;
    char query[256];

    memset(username, 0, USERNAME_SIZE);
    memset(password, 0, PASSWORD_SIZE);

    // Tách username và password từ buffer
    p = strtok(buffer, " // ");
    for (int i = 0; i < 2; i++)
    {
        p = strtok(NULL, " // ");
        if (p != NULL)
        {
            if (i == 0)
            {
                strcpy(username, p);
            }
            if (i == 1)
            {
                strcpy(password, p);
            }
        }
    }

    // Truy vấn cơ sở dữ liệu
    snprintf(query, sizeof(query),
             "SELECT id, username FROM users WHERE username='%s' AND password='%s'",
             username, password);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        return LOGIN_FAILURE;
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        fprintf(stderr, "Store result failed: %s\n", mysql_error(conn));
        return LOGIN_FAILURE;
    }

    int auth_status = LOGIN_FAILURE;
    if ((row = mysql_fetch_row(res)) != NULL)
    {
        // Lấy thông tin `id` và `username` từ kết quả
        user->id = atoi(row[0]); // Lấy id từ kết quả truy vấn
        strcpy(user->username, row[1]);

        auth_status = LOGIN_SUCCESS;

        // Cập nhật trường `last_login` trong bảng users
        snprintf(query, sizeof(query),
                 "UPDATE users SET last_login = NOW() WHERE id=%d",
                 user->id);

        if (mysql_query(conn, query))
        {
            fprintf(stderr, "Update last_login failed: %s\n", mysql_error(conn));
            auth_status = LOGIN_FAILURE;
        }
    }

    mysql_free_result(res);
    return auth_status;
}

int userExists(Database *db, const char *username)
{
    MYSQL *conn = getDatabaseConnection(db);
    MYSQL_RES *res;
    char fileUsername[USERNAME_SIZE];
    char query[256];

    snprintf(query, sizeof(query),
             "SELECT * FROM users WHERE username='%s'", username);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Query failed: %s\n", mysql_error(conn));
        return -1; // Lỗi truy vấn
    }

    res = mysql_store_result(conn);
    if (res == NULL)
    {
        fprintf(stderr, "Store result failed: %s\n", mysql_error(conn));
        return -1; // Lỗi xử lý kết quả
    }

    int exists = (mysql_num_rows(res) > 0) ? 1 : 0;
    mysql_free_result(res);
    return exists;
}

int signUpUser(Database *db, char *buffer)
{
    MYSQL *conn = getDatabaseConnection(db);
    char query[256];
    char username[USERNAME_SIZE], password[PASSWORD_SIZE];
    char *p;

    memset(username, 0, USERNAME_SIZE);
    memset(password, 0, PASSWORD_SIZE);

    // Tách username và password từ buffer
    p = strtok(buffer, " // ");
    for (int i = 0; i < 2; i++)
    {
        p = strtok(NULL, " // ");
        if (p != NULL)
        {
            if (i == 0)
            {
                strcpy(username, p);
                printf("%s\n", username);
            }
            if (i == 1)
            {
                strcpy(password, p);
                printf("%s\n", password);
            }
        }
    }

    // Kiểm tra nếu username đã tồn tại
    if (userExists(db, username))
    {
        printf("Username '%s' đã tồn tại\n", username);
        return SIGNUP_FAILURE;
    }

    // Chèn tài khoản mới vào cơ sở dữ liệu
    snprintf(query, sizeof(query),
             "INSERT INTO users (username, password, created_at) VALUES ('%s', '%s', NOW())",
             username, password);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Insert failed: %s\n", mysql_error(conn));
        return SIGNUP_FAILURE;
    }

    printf("Đăng ký thành công cho username '%s'\n", username);
    return SIGNUP_SUCCESS;
}