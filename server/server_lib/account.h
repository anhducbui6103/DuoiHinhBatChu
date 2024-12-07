#ifndef ACCOUNT_H
#define ACCOUNT_H

#include "../../lib/database.h"

typedef struct User
{
    int id;
    char username[20];
    int room_id;
    int state;
    int score;
    int socket_fd;
} User;

void freeUser(User *user);
int authenticateUser(Database *db, char *buffer, User *user);
int userExists(Database *db, const char *username);
int signUpUser(Database *db, char *buffer);

#endif