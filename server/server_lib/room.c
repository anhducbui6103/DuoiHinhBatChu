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
    }
}

// Hàm thêm người chơi vào phòng
int joinRoom(Room rooms[], User *user, int *assigned_room, Database *db)
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        if (rooms[i].player_count < MAX_PLAYERS_PER_ROOM)
        {
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

    // Giảm số lượng người chơi trong phòng
    rooms[room_id - 1].player_count--;
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
}
