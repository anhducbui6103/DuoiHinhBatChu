#ifndef ROOM_H
#define ROOM_H

#include "../../lib/database.h"
#include "account.h"
#include "../../lib/protocol.h"

#define MAX_ROOMS 3
#define MAX_PLAYERS_PER_ROOM 3

typedef struct Room
{
    int id;
    int player_count;
    char status;
    User players[MAX_PLAYERS_PER_ROOM];
} Room;

// Các hàm quản lý phòng
void initRooms(Room rooms[]);
void checkRoomPlayers(Room *room);
int joinRoom(Room rooms[], User *user, int *assigned_room, Database *db);
void leaveRoom(Room rooms[], User *user, Database *db);

#endif // ROOM_H
