#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include "../lib/protocol.h"
#include "./server_lib/account.h"
#include "./server_lib/database.h"

#define PORT 8080
#define LOCALHOST "127.0.0.1"
#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

Database db;

void handleSigint(int sig)
{
    printf("\nServer shutting down...\n");
    disconnectDatabase(&db); // Đóng kết nối đến database
    exit(0);                 // Thoát chương trình
}

int main()
{
    int server_fd, new_socket, max_sd, activity, i;
    struct sockaddr_in address;
    int opt = 1;
    fd_set readfds;
    char buffer[BUFFER_SIZE];
    User users[MAX_CLIENTS];

    // Đăng ký handler cho SIGINT
    signal(SIGINT, handleSigint);

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
    connectToDatabase(&db, LOCALHOST, "root", NULL, "duoihinhbatchu", 3306);

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
                buffer[0] = authenticateUser(&db, buffer);
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

                    // Close the socket and mark as 0 in list for reuse
                    close(sd);
                    users[i].socket_fd = 0;
                }
            }

            if (buffer[0] == LOGIN)
            {
                buffer[0] = authenticateUser(&db, buffer);
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
        }
    }

    disconnectDatabase(&db);
    return 0;
}