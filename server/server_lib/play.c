#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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
#include <ctype.h>
#include "play.h"

#define BUFFER_SIZE 1024

int initQuestions(Question questions[], Database *db)
{
    for (int i = 0; i < QUESTION_COUNT; i++)
    {
        memset(questions[i].id, 0, QUESTION_ID_SIZE);
        memset(questions[i].hint, 0, HINT_SIZE);
        memset(questions[i].answer, 0, ANSWER_SIZE);
    }

    // Lấy câu hỏi từ cơ sở dữ liệu
    MYSQL *conn = getDatabaseConnection(db);
    MYSQL_RES *result;
    MYSQL_ROW row;
    char query[256];

    // Get total number of questions first
    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM questions");
    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Database error: %s\n", mysql_error(conn));
        return -1;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
    int total_questions = atoi(row[0]);
    mysql_free_result(result);

    // Select random questions using ORDER BY RAND() LIMIT
    snprintf(query, sizeof(query),
             "SELECT id, hint FROM questions ORDER BY RAND() LIMIT %d", QUESTION_COUNT);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Database error: %s\n", mysql_error(conn));
        return -1;
    }

    result = mysql_store_result(conn);
    int i = 0;
    while ((row = mysql_fetch_row(result)) && i < QUESTION_COUNT)
    {
        strncpy(questions[i].id, row[0], sizeof(questions[i].id) - 1);
        questions[i].id[sizeof(questions[i].id) - 1] = '\0';
        strncpy(questions[i].hint, row[1], sizeof(questions[i].hint) - 1);
        questions[i].hint[sizeof(questions[i].hint) - 1] = '\0';
        i++;
    }

    mysql_free_result(result);

    // Get answers for each question
    for (int i = 0; i < QUESTION_COUNT; i++)
    {
        snprintf(query, sizeof(query),
                 "SELECT answer_text FROM answers WHERE question_id = '%s'",
                 questions[i].id);

        if (mysql_query(conn, query))
        {
            fprintf(stderr, "Database error: %s\n", mysql_error(conn));
            return -1;
        }

        result = mysql_store_result(conn);
        if ((row = mysql_fetch_row(result)))
        {
            strncpy(questions[i].answer, row[0], sizeof(questions[i].answer) - 1);
            questions[i].answer[sizeof(questions[i].answer) - 1] = '\0';
        }
        mysql_free_result(result);
    }
}

void startGame(Room *room)
{
    char gameBuffer[BUFFER_SIZE];

    // Gửi thông điệp START_GAME cho tất cả người chơi trong phòng
    gameBuffer[0] = GAME_START;
    for (int i = 0; i < room->player_count; i++)
    {
        send(room->players[i].socket_fd, gameBuffer, BUFFER_SIZE, 0);
    }
}

void createQuestionBuffer(Question *question, char *buffer)
{
    memset(buffer, 0, BUFFER_SIZE);
    buffer[0] = QUESTION;
    strcat(buffer, " // ");
    strcat(buffer, question->id);
    strcat(buffer, " // ");
    strcat(buffer, question->hint);
}

void sendQuestion(Room *room, char *buffer)
{
    for (int i = 0; i < room->player_count; i++)
    {
        send(room->players[i].socket_fd, buffer, BUFFER_SIZE, 0);
    }
    buffer[0] = ANSWER_WAITING;
    for (int i = 0; i < room->player_count; i++)
    {
        send(room->players[i].socket_fd, buffer, BUFFER_SIZE, 0);
    }
}

void setBellStatus(Room *room, int status, char *buffer)
{
    if (status == BELL_RING_AVAILABLE)
    {
        buffer[0] = BELL_RING_AVAILABLE;
    }
    else if (status == BELL_RING_UNAVAILABLE)
    {
        buffer[0] = BELL_RING_UNAVAILABLE;
    }

    for (int i = 0; i < room->player_count; i++)
    {
        send(room->players[i].socket_fd, buffer, BUFFER_SIZE, 0);
    }
}

int checkAnswer(char *question_id, char *userAnswer, Database *db)
{
    MYSQL *conn = getDatabaseConnection(db);
    MYSQL_RES *result;
    MYSQL_ROW row;
    char query[256];
    char correctAnswer[ANSWER_SIZE];
    char lowerUserAnswer[ANSWER_SIZE];

    // Get the correct answer from database
    snprintf(query, sizeof(query),
             "SELECT answer_text FROM answers WHERE question_id = '%s'",
             question_id);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "Database error: %s\n", mysql_error(conn));
        return -1;
    }

    result = mysql_store_result(conn);
    if ((row = mysql_fetch_row(result)))
    {
        strncpy(correctAnswer, row[0], sizeof(correctAnswer) - 1);
        correctAnswer[sizeof(correctAnswer) - 1] = '\0';
    }
    else
    {
        mysql_free_result(result);
        return -1; // Answer not found
    }
    mysql_free_result(result);

    // Convert user answer to lowercase
    int i;
    for (i = 0; userAnswer[i]; i++)
    {
        lowerUserAnswer[i] = tolower(userAnswer[i]);
    }
    lowerUserAnswer[i] = '\0';

    // Compare answers
    return (strcmp(correctAnswer, lowerUserAnswer) == 0) ? 1 : 0;
}