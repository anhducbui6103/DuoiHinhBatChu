#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "../lib/protocol.h"
#include "server_lib/account.h"
#include "../lib/database.h"
#include "server_lib/room.h"
#include "server_lib/play.h"

#define PORT 8080
#define LOCALHOST "127.0.0.1"
#define MAX_CLIENTS 9
#define BUFFER_SIZE 1024
#define CHAT_BUFFER_SIZE 256

Database db;
User users[MAX_CLIENTS];
Room rooms[MAX_ROOMS];
Question questions[QUESTION_COUNT];
char chat_buffer[CHAT_BUFFER_SIZE];
int current_question;
int assigned_room = -1;

void handleSigint(int sig)
{
    printf("\nServer shutting down...\n");

    // Make all current users leave all rooms
    for (int j = 0; j < MAX_CLIENTS; j++)
    {
        leaveRoom(rooms, &users[j], &db);
    }

    disconnectDatabase(&db); // Đóng kết nối đến database
    exit(0);                 // Thoát chương trình
}

// Signal handler to prevent zombie processes
void handleSigchld(int sig)
{
    (void)sig; // Ignore unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
}

int main()
{
    int server_fd, new_socket, max_sd, activity, i;
    struct sockaddr_in address;
    int opt = 1;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    pid_t pid;

    // Đăng ký handler cho SIGINT
    signal(SIGINT, handleSigint);

    // Đăng ký handler cho SIGCHLD
    signal(SIGCHLD, handleSigchld);

    // Initialize client sockets to 0
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        users[i].socket_fd = 0;
    }

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Define server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Open connect to database
    connectToDatabase(&db, LOCALHOST, DATABASE_USER, DATABASE_PASSWORD, DATABASE_NAME, 3306);

    // Initialize rooms
    initRooms(rooms);

    printf("Listening on port %d...\n", PORT);

    while (1)
    {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add server socket to set
        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        // Add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            // Socket descriptor
            int sd = users[i].socket_fd;

            // If valid socket descriptor then add to read list
            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }

            // Keep track of the maximum socket descriptor
            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        // Wait for activity on one of the sockets
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // Incoming connection
        if (FD_ISSET(server_fd, &readfds))
        {
            socklen_t addrlen = sizeof(address);
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0)
            {
                perror("accept");
                disconnectDatabase(&db);
                exit(EXIT_FAILURE);
            }

            printf("New connection, IP: %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Add new client socket to array
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                // If position is empty
                if (users[i].socket_fd == 0)
                {
                    users[i].socket_fd = new_socket;

                    // Receive user_info from client
                    buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline
                    strcpy(users[i].username, buffer);

                    break;
                }
            }

            // Send AUTHENTICATE
            buffer[0] = AUTHENTICATE;
            send(new_socket, buffer, BUFFER_SIZE, 0);
            memset(buffer, 0, BUFFER_SIZE);
            if (recv(new_socket, buffer, BUFFER_SIZE, 0) <= 0)
            {
                printf("Client disconnected!\n");
                break;
            };

            // Login request
            if (buffer[0] == LOGIN)
            {
                buffer[0] = authenticateUser(&db, buffer, &users[i]);
                if (buffer[0] == LOGIN_SUCCESS)
                {
                    send(new_socket, buffer, BUFFER_SIZE, 0);
                }
                else if (buffer[0] == LOGIN_FAILURE)
                {
                    send(new_socket, buffer, BUFFER_SIZE, 0);
                    buffer[0] = AUTHENTICATE;
                    send(new_socket, buffer, BUFFER_SIZE, 0);
                }
            }

            // Signup request
            else if (buffer[0] == SIGNUP)
            {
                buffer[0] = signUpUser(&db, buffer);
                send(new_socket, buffer, BUFFER_SIZE, 0);
                buffer[0] = AUTHENTICATE;
                send(new_socket, buffer, BUFFER_SIZE, 0);
            }
        }

        // Check for I/O operations on other sockets
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            int sd = users[i].socket_fd;

            if (FD_ISSET(sd, &readfds))
            {
                if (recv(sd, buffer, BUFFER_SIZE, 0) <= 0)
                {
                    // Client disconnected
                    socklen_t addrlen = sizeof(address);
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                    printf("Client disconnected, IP: %s, port: %d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    // Delete user_room information in database
                    leaveRoom(rooms, &users[i], &db);

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    freeUser(&users[i]);
                }
                else
                {
                    if (buffer[0] == LOGIN)
                    {
                        buffer[0] = authenticateUser(&db, buffer, &users[i]);
                        if (buffer[0] == LOGIN_SUCCESS)
                        {
                            send(new_socket, buffer, BUFFER_SIZE, 0);
                        }
                        else if (buffer[0] == LOGIN_FAILURE)
                        {
                            send(new_socket, buffer, BUFFER_SIZE, 0);
                            buffer[0] = AUTHENTICATE;
                            send(new_socket, buffer, BUFFER_SIZE, 0);
                        }
                    }

                    else if (buffer[0] == SIGNUP)
                    {
                        buffer[0] = signUpUser(&db, buffer);
                        send(new_socket, buffer, BUFFER_SIZE, 0);
                        buffer[0] = AUTHENTICATE;
                        send(new_socket, buffer, BUFFER_SIZE, 0);
                    }

                    else if (buffer[0] == JOIN_ROOM)
                    {
                        if (joinRoom(rooms, &users[i], &assigned_room, &db) == 0)
                        {
                            buffer[0] = JOIN_ROOM_SUCCESS;
                            send(users[i].socket_fd, buffer, strlen(buffer), 0);
                            printf("User %s assigned to room%d\n", users[i].username, assigned_room);

                            if (rooms[assigned_room - 1].status == ROOM_PLAYING)
                            {
                                initQuestions(questions, &db);
                                current_question = 0;
                                startGame(&rooms[assigned_room - 1]);
                                setBellStatus(&rooms[assigned_room - 1], BELL_RING_AVAILABLE, buffer);
                                createQuestionBuffer(&questions[current_question], buffer);
                                sendQuestion(&rooms[assigned_room - 1], buffer);
                                sendScoreUpdate(&rooms[assigned_room - 1], buffer);
                                current_question++;
                            }
                        }
                        else
                        {
                            buffer[0] = JOIN_ROOM_FAILURE;
                            send(users[i].socket_fd, buffer, strlen(buffer), 0);
                        }
                    }

                    else if (buffer[0] == OUT_ROOM)
                    {
                        leaveRoom(rooms, &users[i], &db);
                    }

                    else if (buffer[0] == BELL_RING)
                    {
                        printf("Bell ring from user %s\n", users[i].username);

                        // Find the room the user is in
                        Room *current_room = &rooms[users[i].room_id - 1];

                        // Send BELL_RING_UNAVAILABLE to other users in the room
                        buffer[0] = BELL_RING_UNAVAILABLE;
                        for (int j = 0; j < MAX_PLAYERS_PER_ROOM; j++)
                        {
                            if (current_room->players[j].socket_fd != 0 &&
                                current_room->players[j].socket_fd != users[i].socket_fd)
                            {
                                send(current_room->players[j].socket_fd, buffer, BUFFER_SIZE, 0);
                            }
                        }

                        // Send ANSWER_REQUEST to the user who rang the bell
                        buffer[0] = ANSWER_REQUEST;
                        send(users[i].socket_fd, buffer, BUFFER_SIZE, 0);
                    }
                    else if (buffer[0] == CHAT_MESSAGE)
                    {
                        // Lấy nội dung tin nhắn từ buffer
                        char *message = buffer + 1;

                        // Xác định phòng mà người dùng đang tham gia
                        Room *current_room = &rooms[users[i].room_id - 1];

                        // Chuẩn bị gói tin broadcast
                        memset(chat_buffer, 0, CHAT_BUFFER_SIZE);
                        chat_buffer[0] = CHAT_BROADCAST;
                        snprintf(chat_buffer + 1, CHAT_BUFFER_SIZE - 1, "%s: %s", users[i].username, message);

                        // Gửi tin nhắn đến tất cả các client trong phòng
                        for (int j = 0; j < current_room->player_count; j++)
                        {
                            if (current_room->players[j].socket_fd != 0)
                            {
                                send(current_room->players[j].socket_fd, chat_buffer, CHAT_BUFFER_SIZE, 0);
                            }
                        }
                    }

                    else if (buffer[0] == ANSWER)
                    {
                        // Find the room the user is in
                        Room *current_room = &rooms[users[i].room_id - 1];

                        // Get the answer from the buffer
                        char answer[256] = {0};        // Buffer for the answer
                        char *p = strchr(buffer, '/'); // Find first '/'
                        if (p != NULL)
                        {
                            p = strchr(p + 1, '/'); // Find second '/'
                            if (p != NULL)
                            {
                                // Skip the " // " separator
                                p += 2;
                                strncpy(answer, p, sizeof(answer) - 1);
                                printf("Answer from user %s: %s\n", users[i].username, answer);

                                // Check if the answer is correct
                                if (checkAnswer(questions[current_question - 1].id, answer, &db) == 1)
                                {
                                    buffer[0] = CORRECT;
                                    // Update score for both User and Room's player
                                    users[i].score += 10;
                                    // Update score in the room's player array
                                    for (int j = 0; j < current_room->player_count; j++)
                                    {
                                        if (strcmp(current_room->players[j].username, users[i].username) == 0)
                                        {
                                            current_room->players[j].score = users[i].score;
                                            break;
                                        }
                                    }
                                    send(users[i].socket_fd, buffer, BUFFER_SIZE, 0);

                                    // Send score update before sending next question
                                    sendScoreUpdate(current_room, buffer);

                                    if (current_question >= QUESTION_COUNT)
                                    {
                                        // Send FINISH message to all players in the room
                                        for (int j = 0; j < current_room->player_count; j++)
                                        {
                                            memset(buffer, 0, BUFFER_SIZE);
                                            buffer[0] = FINISH;
                                            strcat(buffer, " // ");
                                            char score_str[20];                                           // Buffer for score string
                                            snprintf(score_str, sizeof(score_str), "%d", users[j].score); // Convert int to string
                                            strcat(buffer, score_str);
                                            send(current_room->players[j].socket_fd, buffer, BUFFER_SIZE, 0);
                                        }
                                    }
                                    else
                                    {
                                        // Send the next question
                                        setBellStatus(current_room, BELL_RING_AVAILABLE, buffer);
                                        createQuestionBuffer(&questions[current_question], buffer);
                                        sendQuestion(current_room, buffer);
                                        current_question++;
                                    }
                                }
                                else
                                {
                                    buffer[0] = INCORRECT;
                                    send(users[i].socket_fd, buffer, BUFFER_SIZE, 0);
                                    // Set the bell status to available
                                    setBellStatus(current_room, BELL_RING_AVAILABLE, buffer);
                                    buffer[0] = ANSWER_WAITING;
                                    for (int i = 0; i < current_room->player_count; i++)
                                    {
                                        send(current_room->players[i].socket_fd, buffer, BUFFER_SIZE, 0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    disconnectDatabase(&db);
    close(new_socket);
    close(server_fd);
    return 0;
}