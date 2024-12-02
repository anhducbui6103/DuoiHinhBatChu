#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "../../lib/database.h"

typedef struct User
{
    char username[20];
    int room;
    int state;
    int score;
    int socket_fd;
} User;

int authenticateUser(Database *db, char *buffer, User *user);
int userExists(Database *db, const char *username);
int signUpUser(Database *db, char *buffer);

#endif