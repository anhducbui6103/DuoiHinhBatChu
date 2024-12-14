#include <stdio.h>
#include <string.h>
#include "room.h"
#include "/opt/lampp/include/mysql.h"

// Hàm khởi tạo danh sách phòng
void initRooms(Room rooms[])
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        rooms[i].id = i + 1;       // Phòng ID bắt đầu từ 1
        rooms[i].player_count = 0; // Mặc định số người chơi là 0
        rooms[i].status = ROOM_WAITING;
        memset(rooms[i].players, 0, sizeof(rooms[i].players));
    }
}

void checkRoomPlayers(Room *room)
{
    if (room->player_count == MAX_PLAYERS_PER_ROOM) // Kiểm tra nếu phòng đủ 3 người chơi
    {
        room->status = ROOM_PLAYING; // Cập nhật trạng thái phòng
    }
}

// Hàm thêm người chơi vào phòng
int joinRoom(Room rooms[], User *user, int *assigned_room, Database *db)
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        if (rooms[i].status == ROOM_WAITING && rooms[i].player_count < MAX_PLAYERS_PER_ROOM)
        {
            rooms[i].players[rooms[i].player_count] = *user;
            rooms[i].player_count++;
            user->room_id = rooms[i].id;
            *assigned_room = rooms[i].id;
            // printf("room %d has %d users\n", rooms[i].id, rooms[i].player_count);

            // Ghi thông tin vào cơ sở dữ liệu
            MYSQL *conn = getDatabaseConnection(db);
            char query[256];
            snprintf(query, sizeof(query),
                     "INSERT INTO user_rooms (user_id, room_id, joined_at) VALUES (%d, %d, NOW())",
                     user->id, *assigned_room);

            if (mysql_query(conn, query))
            {
                fprintf(stderr, "Database error: %s\n", mysql_error(conn));
                return -1; // Lỗi ghi vào CSDL
            }

            // Kiểm tra trạng thái phòng
            checkRoomPlayers(&rooms[i]);

            return 0; // Thành công
        }
    }
    return -1; // Không còn phòng trống
}

// Hàm xử lý rời phòng
void leaveRoom(Room rooms[], User *user, Database *db)
{
    int room_id = user->room_id;
    int user_id = user->id;
    if (room_id < 1 || room_id > MAX_ROOMS)
    {
        // fprintf(stderr, "Invalid room ID\n");
        return;
    }

    Room *room = &rooms[room_id - 1];

    // Giảm số lượng người chơi trong phòng
    if (room->player_count > 0)
    {
        room->player_count--;
    }

    // Xóa người chơi khỏi danh sách players
    for (int i = 0; i < MAX_PLAYERS_PER_ROOM; i++)
    {
        if (room->players[i].id == user->id)
        {
            memset(&room->players[i], 0, sizeof(User));
            break;
        }
    }

    // printf("room %d has %d users\n", rooms[room_id - 1].id, rooms[room_id - 1].player_count);

    // Xóa thông tin người chơi khỏi cơ sở dữ liệu
    MYSQL *conn = getDatabaseConnection(db);
    char query[256];
    snprintf(query, sizeof(query),
             "DELETE FROM user_rooms WHERE user_id = %d AND room_id = %d",
             user_id, room_id);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Database error: %s\n", mysql_error(conn));
    }

    // Reset room_id trong user
    user->room_id = 0;
}
