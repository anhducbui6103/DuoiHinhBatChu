#ifndef PLAY_H
#define PLAY_H

#include "room.h"
#include "../../lib/protocol.h"

#define QUESTION_COUNT 3
#define QUESTION_ID_SIZE 10
#define HINT_SIZE 10
#define ANSWER_SIZE 1024

typedef struct Question
{
    char id[QUESTION_ID_SIZE];
    char hint[HINT_SIZE];
    char answer[ANSWER_SIZE];
} Question;

int initQuestions(Question questions[], Database *db);
void startGame(Room *room);
void createQuestionBuffer(Question *question, char *buffer);
void sendQuestion(Room *room, char *buffer);
void setBellStatus(Room *room, int status, char *buffer);
int checkAnswer(char *question_id, char *userAnswer, Database *db);
void sendScoreUpdate(Room *room, char *buffer);

#endif