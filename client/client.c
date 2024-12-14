#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include "../lib/protocol.h"
#include "../lib/database.h"

#define PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024
#define USERNAME_SIZE 100
#define PASSWORD_SIZE 100
#define HINT_SIZE 10
#define ANSWER_SIZE 1024
#define QUESTION_ID_SIZE 10
#define LOCALHOST "127.0.0.1"
#define SELECT_TIMEOUT_SEC 0
#define SELECT_TIMEOUT_USEC 100000 // 100ms

int sockfd;
int opt_choice;
char buffer[BUFFER_SIZE];
char username[USERNAME_SIZE];
char password[PASSWORD_SIZE];
char question_id[QUESTION_ID_SIZE];
char hint[HINT_SIZE];
char answer[ANSWER_SIZE];
int bell_status = 0;
Database db;

void authenticateFunc();
void handleSigint(int sig);
void getQuestionFromServer();

int main()
{
    int max_sd, activity;
    struct sockaddr_in server_addr;
    fd_set readfds;
    struct timeval tv;

    // Đăng ký handler cho SIGINT
    signal(SIGINT, handleSigint);

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

    // Open connect to database
    connectToDatabase(&db, LOCALHOST, DATABASE_USER, DATABASE_PASSWORD, DATABASE_NAME, 3306);

    while (1)
    {
        // Clear the set and add descriptors
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);       // Monitor server socket
        FD_SET(STDIN_FILENO, &readfds); // Monitor input from user (stdin)

        max_sd = sockfd;

        tv.tv_sec = SELECT_TIMEOUT_SEC;
        tv.tv_usec = SELECT_TIMEOUT_USEC;

        // Use select() to monitor multiple file descriptors
        activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);

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
                printf("Login success! Press any key to start game!\n");
                getchar();
                buffer[0] = JOIN_ROOM;
                send(sockfd, buffer, BUFFER_SIZE, 0);
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
            else if (buffer[0] == JOIN_ROOM_SUCCESS)
            {
                printf("Join success. Waiting for other players...\n");
            }
            else if (buffer[0] == JOIN_ROOM_FAILURE)
            {
                printf("Join failed\n");
            }
            else if (buffer[0] == GAME_START)
            {
                printf("Game START !!! \n");
            }
            else if (buffer[0] == QUESTION)
            {
                getQuestionFromServer();
                printf("Question ID: %s\n", question_id);
                printf("Hint: %s\n", hint);
            }
            else if (buffer[0] == BELL_RING_AVAILABLE)
            {
                bell_status = 1;
            }
            else if (buffer[0] == BELL_RING_UNAVAILABLE)
            {
                bell_status = 0;
            }
            else if (buffer[0] == ANSWER_WAITING)
            {
                printf("Answer waiting. Press any key to ring the bell\n");
                while (1)
                {
                    FD_ZERO(&readfds);
                    FD_SET(sockfd, &readfds);
                    FD_SET(STDIN_FILENO, &readfds);

                    tv.tv_sec = SELECT_TIMEOUT_SEC;
                    tv.tv_usec = SELECT_TIMEOUT_USEC;

                    activity = select(max_sd + 1, &readfds, NULL, NULL, &tv);

                    if (activity < 0 && errno != EINTR)
                    {
                        perror("select error");
                        break;
                    }

                    if (FD_ISSET(sockfd, &readfds))
                    {
                        memset(buffer, 0, BUFFER_SIZE);
                        int valread = recv(sockfd, buffer, BUFFER_SIZE, 0);
                        if (valread <= 0)
                        {
                            printf("Server disconnected.\n");
                            break;
                        }

                        if (buffer[0] == BELL_RING_AVAILABLE)
                        {
                            bell_status = 1;
                        }
                        else if (buffer[0] == BELL_RING_UNAVAILABLE)
                        {
                            bell_status = 0;
                            printf("A player has ring the bell\n");
                        }
                        else if (buffer[0] == QUESTION)
                        {
                            getQuestionFromServer();
                            printf("Question ID: %s\n", question_id);
                            printf("Hint: %s\n", hint);
                            printf("Answer waiting. Press any key to ring the bell\n");
                        }
                        else if (buffer[0] == FINISH)
                        {
                            break;
                        }
                    }

                    if (FD_ISSET(STDIN_FILENO, &readfds))
                    {
                        getchar(); // Consume the keypress
                        if (bell_status == 1)
                        {
                            buffer[0] = BELL_RING;
                            send(sockfd, buffer, BUFFER_SIZE, 0);
                        }
                        else
                        {
                            printf("Bell is not available\n");
                        }
                        break;
                    }
                }
            }
            else if (buffer[0] == ANSWER_REQUEST)
            {
                printf("Your answer: ");
                fgets(answer, ANSWER_SIZE, stdin);
                answer[strcspn(answer, "\n")] = 0;
                memset(buffer, 0, BUFFER_SIZE);
                buffer[0] = ANSWER;
                strcat(buffer, " // ");
                strcat(buffer, answer);
                send(sockfd, buffer, BUFFER_SIZE, 0);
            }
            else if (buffer[0] == CORRECT)
            {
                printf("Correct!\n");
            }
            else if (buffer[0] == INCORRECT)
            {
                printf("Incorrect!\n");
            }
            if (buffer[0] == FINISH)
            {
                char *score = strtok(buffer, " // ");
                score = strtok(NULL, " // ");
                printf("Game finished. Your score: %s\n", score);
                printf("Press any key to exit\n");
                getchar();
                break;
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
    disconnectDatabase(&db);
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

void handleSigint(int sig)
{
    printf("\nExiting...\n");
    disconnectDatabase(&db);
    exit(0);
}

void getQuestionFromServer()
{
    char *p;
    memset(question_id, 0, QUESTION_ID_SIZE);
    memset(hint, 0, HINT_SIZE);

    // Tách question_id và hint từ buffer
    p = strtok(buffer, " // ");
    for (int i = 0; i < 2; i++)
    {
        p = strtok(NULL, " // ");
        if (p != NULL)
        {
            if (i == 0)
            {
                strcpy(question_id, p);
            }
            if (i == 1)
            {
                strcpy(hint, p);
            }
        }
    }

    // Get question's image from database
    MYSQL *conn = getDatabaseConnection(&db);
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned long *lengths; // To store lengths of result fields
    char query[256];

    snprintf(query, sizeof(query), "SELECT image FROM questions WHERE id = '%s'", question_id);
    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Database error: %s\n", mysql_error(conn));
        return;
    }

    result = mysql_store_result(conn);
    if (result == NULL)
    {
        fprintf(stderr, "Failed to get result set\n");
        return;
    }

    // Get the row
    row = mysql_fetch_row(result);
    if (row == NULL)
    {
        fprintf(stderr, "No data found\n");
        mysql_free_result(result);
        return;
    }

    // Get the lengths of each field
    lengths = mysql_fetch_lengths(result);
    if (lengths == NULL)
    {
        fprintf(stderr, "Failed to get field lengths\n");
        mysql_free_result(result);
        return;
    }

    // Now row[0] contains the image data and lengths[0] contains its length
    unsigned long image_size = lengths[0];
    unsigned char *image_data = malloc(image_size);
    if (image_data == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for image\n");
        mysql_free_result(result);
        return;
    }

    // Copy the image data
    memcpy(image_data, row[0], image_size);

    // Now you can use image_data and image_size
    // For example, save to a file:
    FILE *fp = fopen("question_image.jpg", "wb");
    if (fp != NULL)
    {
        fwrite(image_data, 1, image_size, fp);
        fclose(fp);
    }

    // Clean up
    free(image_data);
    mysql_free_result(result);
}