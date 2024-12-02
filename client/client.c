#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include "../lib/protocol.h"

#define PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 100
#define PASSWORD_SIZE 100

int opt_choice;
char buffer[BUFFER_SIZE];
char username[USERNAME_SIZE];
char password[PASSWORD_SIZE];

void authenticateFunc();

int main()
{
    int sockfd, max_sd, activity;
    struct sockaddr_in server_addr;
    fd_set readfds;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Clear the set and add descriptors
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);       // Monitor server socket
        FD_SET(STDIN_FILENO, &readfds); // Monitor input from user (stdin)

        max_sd = sockfd;

        // Use select() to monitor multiple file descriptors
        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR)
        {
            perror("select error");
            break;
        }

        // If something happens on the server socket, we receive the message
        if (FD_ISSET(sockfd, &readfds))
        {
            memset(buffer, 0, BUFFER_SIZE); // Clear buffer before receiving
            int valread = recv(sockfd, buffer, BUFFER_SIZE, 0);
            if (valread <= 0)
            {
                printf("Server disconnected.\n");
                break;
            }

            if (buffer[0] == AUTHENTICATE)
            {
                do
                {
                    authenticateFunc();
                    send(sockfd, buffer, BUFFER_SIZE, 0);
                } while (opt_choice < 1 || opt_choice > 2);
            }
            else if (buffer[0] == LOGIN_SUCCESS)
            {
                printf("Login success\n");
            }
            else if (buffer[0] == LOGIN_FAILURE)
            {
                printf("Login failed\n");
            }
            else if (buffer[0] == SIGNUP_SUCCESS)
            {
                printf("Signup success\n");
            }
            else if (buffer[0] == SIGNUP_FAILURE)
            {
                printf("Signup failed\n");
            }
        }

        // If input is available from the user
        // if (FD_ISSET(STDIN_FILENO, &readfds))
        // {
        //     memset(buffer, 0, BUFFER_SIZE); // Clear buffer before sending
        //     fgets(buffer, BUFFER_SIZE, stdin);
        //     send(sockfd, buffer, strlen(buffer), 0);
        // }
    }

    // Clean up
    close(sockfd);
    return 0;
}

void authenticateFunc()
{
    printf("1. Login\n2. Register\nYour choice:\n");
    scanf("%d", &opt_choice);
    getchar();
    if (opt_choice == 1)
    {
        buffer[0] = LOGIN;
    }
    else if (opt_choice == 2)
    {
        buffer[0] = SIGNUP;
    }
    if (opt_choice == 1 || opt_choice == 2)
    {
        strcat(buffer, " // ");
        printf("Username:\n");
        fgets(username, USERNAME_SIZE, stdin);
        username[strcspn(username, "\n")] = 0;
        strcat(buffer, username);
        strcat(buffer, " // ");
        printf("Password:\n");
        fgets(password, PASSWORD_SIZE, stdin);
        password[strcspn(password, "\n")] = 0;
        strcat(buffer, password);
    }
}