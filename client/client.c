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
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

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

typedef struct
{
    SDL_Rect rect;
    char text[256];
    bool isSelected;
    bool isPassword;
} TextInput;

typedef struct
{
    SDL_Rect rect;
    char *text;
    bool isHovered;
} Button;

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;
    SDL_Texture *questionImage;
    TextInput answerInput;
    Button bellButton;
    Button answerButton;
    char hint[HINT_SIZE];
    char currentScore[20];
    char playerList[3][256]; // For 3 players' info
} GameUI;

typedef struct
{
    SDL_Rect rect;
    char username[256];
    int score;
} PlayerScore;

SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *path);
void renderTextInput(SDL_Renderer *renderer, TTF_Font *font, TextInput *input, bool isEnabled);
void renderButton(SDL_Renderer *renderer, TTF_Font *font, Button *button, bool isEnabled);
bool isMouseOverRect(SDL_Rect *rect, int mouseX, int mouseY);

void authenticateFunc();
void handleSigint(int sig);
void getQuestionFromServer();
void showDialog(const char *message);
void initGameUI(GameUI *ui);
void renderGameUI(GameUI *ui);
void handleGameUI();
void renderFinalScreen(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, const char *buffer);

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
                buffer[0] = JOIN_ROOM;
                send(sockfd, buffer, BUFFER_SIZE, 0);
            }
            else if (buffer[0] == LOGIN_FAILURE)
            {
                printf("Login failed\n");
                showDialog("Login failed! Please try again.");
            }
            else if (buffer[0] == SIGNUP_SUCCESS)
            {
                printf("Signup success\n");
                showDialog("Signup success!\n");
            }
            else if (buffer[0] == SIGNUP_FAILURE)
            {
                printf("Signup failed\n");
                showDialog("Signup failed! Please try again.");
            }
            else if (buffer[0] == JOIN_ROOM_SUCCESS)
            {
                printf("Join success. Waiting for other players...\n");
                showDialog("Join success! Waiting for other players...");
            }
            else if (buffer[0] == JOIN_ROOM_FAILURE)
            {
                printf("Join failed\n");
                showDialog("Join failed\n");
            }
            else if (buffer[0] == GAME_START)
            {
                printf("Game START !!! \n");

                // Initialize SDL components
                if (SDL_Init(SDL_INIT_VIDEO) < 0 ||
                    TTF_Init() < 0 ||
                    !(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG))
                {
                    printf("SDL initialization failed\n");
                    printf("SDL Error: %s\n", SDL_GetError());
                    printf("TTF Error: %s\n", TTF_GetError());
                    printf("IMG Error: %s\n", IMG_GetError());
                    return -1;
                }

                handleGameUI();

                // Cleanup SDL
                TTF_Quit();
                IMG_Quit();
                SDL_Quit();
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
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font = NULL;
    SDL_Texture *bgTexture = NULL;

    // Initialize SDL components
    if (SDL_Init(SDL_INIT_VIDEO) < 0 ||
        TTF_Init() < 0 ||
        !(IMG_Init(IMG_INIT_JPG) & IMG_INIT_JPG))
    {
        printf("SDL initialization failed\n");
        return;
    }

    // Create window and renderer
    window = SDL_CreateWindow("Login/Signup",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              960, 540,
                              SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    font = TTF_OpenFont("./font.ttf", 24);
    bgTexture = loadTexture(renderer, "bg.jpg");

    // Create input fields
    TextInput usernameInput = {
        .rect = {480 - 100, 270 - 60, 200, 40},
        .text = "",
        .isSelected = false,
        .isPassword = false};

    TextInput passwordInput = {
        .rect = {480 - 100, 270, 200, 40},
        .text = "",
        .isSelected = false,
        .isPassword = true};

    // Create buttons
    Button loginButton = {
        .rect = {480 - 110, 270 + 60, 100, 40},
        .text = "Login",
        .isHovered = false};

    Button signupButton = {
        .rect = {480 + 10, 270 + 60, 100, 40},
        .text = "Signup",
        .isHovered = false};

    SDL_StartTextInput();
    bool quit = false;
    SDL_Event e;

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                usernameInput.isSelected = isMouseOverRect(&usernameInput.rect, mouseX, mouseY);
                passwordInput.isSelected = isMouseOverRect(&passwordInput.rect, mouseX, mouseY);

                if (isMouseOverRect(&loginButton.rect, mouseX, mouseY))
                {
                    opt_choice = 1;
                    strcpy(username, usernameInput.text);
                    strcpy(password, passwordInput.text);
                    quit = true;
                }
                else if (isMouseOverRect(&signupButton.rect, mouseX, mouseY))
                {
                    opt_choice = 2;
                    strcpy(username, usernameInput.text);
                    strcpy(password, passwordInput.text);
                    quit = true;
                }
            }
            else if (e.type == SDL_TEXTINPUT)
            {
                if (usernameInput.isSelected && strlen(usernameInput.text) < 255)
                {
                    strcat(usernameInput.text, e.text.text);
                }
                else if (passwordInput.isSelected && strlen(passwordInput.text) < 255)
                {
                    strcat(passwordInput.text, e.text.text);
                }
            }
            else if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_BACKSPACE)
                {
                    if (usernameInput.isSelected && strlen(usernameInput.text) > 0)
                    {
                        usernameInput.text[strlen(usernameInput.text) - 1] = '\0';
                    }
                    else if (passwordInput.isSelected && strlen(passwordInput.text) > 0)
                    {
                        passwordInput.text[strlen(passwordInput.text) - 1] = '\0';
                    }
                }
            }
        }

        // Render
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
        renderTextInput(renderer, font, &usernameInput, true);
        renderTextInput(renderer, font, &passwordInput, true);
        renderButton(renderer, font, &loginButton, true);
        renderButton(renderer, font, &signupButton, true);
        SDL_RenderPresent(renderer);
    }

    // Prepare the buffer to send
    if (opt_choice == 1)
    {
        buffer[0] = LOGIN;
    }
    else if (opt_choice == 2)
    {
        buffer[0] = SIGNUP;
    }
    strcat(buffer, " // ");
    strcat(buffer, username);
    strcat(buffer, " // ");
    strcat(buffer, password);

    // Cleanup
    SDL_StopTextInput();
    SDL_DestroyTexture(bgTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
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

SDL_Texture *loadTexture(SDL_Renderer *renderer, const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if (!surface)
    {
        printf("Failed to load image: %s\n", IMG_GetError());
        return NULL;
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture)
    {
        printf("Failed to create texture: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
    return texture;
}

void renderTextInput(SDL_Renderer *renderer, TTF_Font *font, TextInput *input, bool isEnabled)
{
    // Draw the input box with different colors based on enabled state
    if (isEnabled)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // White for enabled
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Gray for disabled
    }
    SDL_RenderFillRect(renderer, &input->rect);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &input->rect);

    if (input->isSelected && isEnabled)
    {
        SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
        SDL_RenderFillRect(renderer, &input->rect);
    }

    // Render the text
    if (strlen(input->text) > 0)
    {
        SDL_Color textColor = isEnabled ? (SDL_Color){0, 0, 0, 255} : (SDL_Color){128, 128, 128, 255};
        const char *displayText = input->isPassword ? "********" : input->text;
        SDL_Surface *surface = TTF_RenderText_Solid(font, displayText, textColor);
        if (surface)
        {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture)
            {
                SDL_Rect textRect = {
                    input->rect.x + 5,
                    input->rect.y + (input->rect.h - surface->h) / 2,
                    surface->w,
                    surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &textRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
}

void renderButton(SDL_Renderer *renderer, TTF_Font *font, Button *button, bool isEnabled)
{
    // Draw button background with different colors based on enabled state
    if (isEnabled)
    {
        if (button->isHovered)
        {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        }
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 128); // Grayed out when disabled
    }
    SDL_RenderFillRect(renderer, &button->rect);

    // Draw button border
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &button->rect);

    // Render button text
    SDL_Color textColor = isEnabled ? (SDL_Color){0, 0, 0, 255} : (SDL_Color){128, 128, 128, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, button->text, textColor);
    if (surface)
    {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture)
        {
            SDL_Rect textRect = {
                button->rect.x + (button->rect.w - surface->w) / 2,
                button->rect.y + (button->rect.h - surface->h) / 2,
                surface->w,
                surface->h};
            SDL_RenderCopy(renderer, texture, NULL, &textRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

bool isMouseOverRect(SDL_Rect *rect, int mouseX, int mouseY)
{
    return mouseX >= rect->x && mouseX <= rect->x + rect->w &&
           mouseY >= rect->y && mouseY <= rect->y + rect->h;
}

void showDialog(const char *message)
{
    SDL_Window *dialogWindow = NULL;
    SDL_Renderer *dialogRenderer = NULL;
    TTF_Font *font = NULL;
    bool quit = false;
    SDL_Event e;

    // Create dialog window
    dialogWindow = SDL_CreateWindow("Message",
                                    SDL_WINDOWPOS_CENTERED,
                                    SDL_WINDOWPOS_CENTERED,
                                    300, 150,
                                    SDL_WINDOW_SHOWN);
    dialogRenderer = SDL_CreateRenderer(dialogWindow, -1, SDL_RENDERER_ACCELERATED);
    font = TTF_OpenFont("./font.ttf", 16);

    // Create OK button
    Button okButton = {
        .rect = {100, 90, 100, 40},
        .text = "OK",
        .isHovered = false};

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (isMouseOverRect(&okButton.rect, mouseX, mouseY))
                {
                    quit = true;
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(dialogRenderer, 240, 240, 240, 255);
        SDL_RenderClear(dialogRenderer);

        // Render message
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface *surface = TTF_RenderText_Solid(font, message, textColor);
        if (surface)
        {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(dialogRenderer, surface);
            if (texture)
            {
                SDL_Rect textRect = {
                    150 - surface->w / 2, // Center horizontally
                    30,                   // Fixed vertical position
                    surface->w,
                    surface->h};
                SDL_RenderCopy(dialogRenderer, texture, NULL, &textRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }

        // Render button
        renderButton(dialogRenderer, font, &okButton, true);

        SDL_RenderPresent(dialogRenderer);
    }

    // Cleanup
    TTF_CloseFont(font);
    SDL_DestroyRenderer(dialogRenderer);
    SDL_DestroyWindow(dialogWindow);
}

void initGameUI(GameUI *ui)
{
    // Create window and renderer
    ui->window = SDL_CreateWindow("Duoi Hinh Bat Chu",
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  1366, 768,
                                  SDL_WINDOW_SHOWN);
    ui->renderer = SDL_CreateRenderer(ui->window, -1, SDL_RENDERER_ACCELERATED);
    ui->font = TTF_OpenFont("./font.ttf", 24);

    // Initialize answer input
    ui->answerInput = (TextInput){
        .rect = {300, 650, 400, 40}, // Position for answer input
        .text = "",
        .isSelected = false,
        .isPassword = false};

    // Initialize bell button
    ui->bellButton = (Button){
        .rect = {900, 650, 100, 40}, // Position for bell button
        .text = "Bell",
        .isHovered = false};

    // Initialize answer button
    ui->answerButton = (Button){
        .rect = {780, 650, 100, 40}, // Position it to the left of the bell button
        .text = "Answer",
        .isHovered = false};

    // Initialize scores and player info
    strcpy(ui->playerList[0], "User 1     Score: 0");
    strcpy(ui->playerList[1], "User 2     Score: 0");
    strcpy(ui->playerList[2], "User 3     Score: 0");
}

void renderGameUI(GameUI *ui)
{
    SDL_SetRenderDrawColor(ui->renderer, 255, 255, 255, 255);
    SDL_RenderClear(ui->renderer);

    // Draw question area border
    SDL_Rect questionArea = {50, 50, 900, 500};
    SDL_SetRenderDrawColor(ui->renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(ui->renderer, &questionArea);

    // Draw question image or placeholder text
    if (!ui->questionImage)
    {
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface *surface = TTF_RenderText_Solid(ui->font, "Question Picture", textColor);
        if (surface)
        {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
            if (texture)
            {
                SDL_Rect textRect = {
                    questionArea.x + (questionArea.w - surface->w) / 2,
                    questionArea.y + (questionArea.h - surface->h) / 2,
                    surface->w,
                    surface->h};
                SDL_RenderCopy(ui->renderer, texture, NULL, &textRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    else
    {
        SDL_RenderCopy(ui->renderer, ui->questionImage, NULL, &questionArea);
    }

    // Render player list and scores
    for (int i = 0; i < 3; i++)
    {
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface *surface = TTF_RenderText_Solid(ui->font, ui->playerList[i], textColor);
        if (surface)
        {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
            if (texture)
            {
                SDL_Rect textRect = {
                    960, 50 + i * 40,
                    surface->w,
                    surface->h};
                SDL_RenderCopy(ui->renderer, texture, NULL, &textRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    // Render hint
    char hintText[256];
    snprintf(hintText, sizeof(hintText), "Hint: %s", ui->hint);
    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(ui->font, hintText, textColor);
    if (surface)
    {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(ui->renderer, surface);
        if (texture)
        {
            SDL_Rect textRect = {50, 650, surface->w, surface->h};
            SDL_RenderCopy(ui->renderer, texture, NULL, &textRect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }

    // Render answer input and buttons with bell_status
    renderTextInput(ui->renderer, ui->font, &ui->answerInput, bell_status == 1);
    renderButton(ui->renderer, ui->font, &ui->answerButton, bell_status == 1);
    renderButton(ui->renderer, ui->font, &ui->bellButton, bell_status == 1);

    SDL_RenderPresent(ui->renderer);
}

void handleGameUI()
{
    GameUI ui;
    initGameUI(&ui);
    bool quit = false;
    SDL_Event e;

    // Load initial question if available
    ui.questionImage = loadTexture(ui.renderer, "question_image.jpg");
    strcpy(ui.hint, hint);
    memset(ui.answerInput.text, 0, sizeof(ui.answerInput.text));

    while (!quit)
    {
        // Check for new messages from server
        fd_set readfds;
        struct timeval tv = {0, 100000}; // 100ms timeout
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        if (select(sockfd + 1, &readfds, NULL, NULL, &tv) > 0)
        {
            if (FD_ISSET(sockfd, &readfds))
            {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = recv(sockfd, buffer, BUFFER_SIZE, 0);
                if (valread > 0)
                {
                    if (buffer[0] == QUESTION)
                    {
                        getQuestionFromServer();
                        if (ui.questionImage)
                        {
                            SDL_DestroyTexture(ui.questionImage);
                        }
                        ui.questionImage = loadTexture(ui.renderer, "question_image.jpg");
                        strcpy(ui.hint, hint);
                        memset(ui.answerInput.text, 0, sizeof(ui.answerInput.text));
                    }
                    else if (buffer[0] == BELL_RING_AVAILABLE)
                    {
                        bell_status = 1;
                    }
                    else if (buffer[0] == BELL_RING_UNAVAILABLE)
                    {
                        bell_status = 0;
                    }
                    else if (buffer[0] == SCORE_UPDATE)
                    {
                        // Parse and update player scores
                        char *token = strtok(buffer + 1, " // "); // Skip message type
                        int player_idx = 0;

                        while (token != NULL && player_idx < 3)
                        {
                            char username[128]; // Smaller buffer for username
                            int score;

                            if (sscanf(token, "%127[^,],%d", username, &score) == 2)
                            {
                                // Debug print to check what we're receiving
                                printf("Updating score for %s: %d\n", username, score);

                                char temp[256];
                                snprintf(temp, sizeof(temp), "%-20s Score: %d", username, score);
                                strncpy(ui.playerList[player_idx], temp, sizeof(ui.playerList[player_idx]) - 1);
                                ui.playerList[player_idx][sizeof(ui.playerList[player_idx]) - 1] = '\0';
                            }

                            token = strtok(NULL, " // ");
                            player_idx++;
                        }
                    }
                    else if (buffer[0] == FINISH)
                    {
                        // Clean up game UI resources
                        if (ui.questionImage)
                        {
                            SDL_DestroyTexture(ui.questionImage);
                        }

                        // Show final score screen
                        renderFinalScreen(ui.window, ui.renderer, ui.font, buffer);
                        quit = true; // Exit the game loop after final screen
                    }
                }
            }
        }

        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (bell_status == 1)
                {
                    ui.answerInput.isSelected = isMouseOverRect(&ui.answerInput.rect, mouseX, mouseY);

                    if (isMouseOverRect(&ui.bellButton.rect, mouseX, mouseY))
                    {
                        buffer[0] = BELL_RING;
                        send(sockfd, buffer, BUFFER_SIZE, 0);
                    }
                    else if (isMouseOverRect(&ui.answerButton.rect, mouseX, mouseY))
                    {
                        // Send answer to server
                        memset(buffer, 0, BUFFER_SIZE); // Clear buffer first

                        // Format the message properly
                        char temp[BUFFER_SIZE];
                        snprintf(temp, sizeof(temp), "%c // %s", ANSWER, ui.answerInput.text);
                        strncpy(buffer, temp, BUFFER_SIZE - 1);
                        buffer[BUFFER_SIZE - 1] = '\0';

                        send(sockfd, buffer, BUFFER_SIZE, 0);

                        // Clear the answer input after sending
                        memset(ui.answerInput.text, 0, sizeof(ui.answerInput.text));
                    }
                }
            }
            else if (e.type == SDL_TEXTINPUT && ui.answerInput.isSelected && bell_status == 1)
            {
                if (strlen(ui.answerInput.text) < 255)
                {
                    strcat(ui.answerInput.text, e.text.text);
                }
            }
            else if (e.type == SDL_KEYDOWN && bell_status == 1)
            {
                if (e.key.keysym.sym == SDLK_BACKSPACE && ui.answerInput.isSelected)
                {
                    int len = strlen(ui.answerInput.text);
                    if (len > 0)
                    {
                        ui.answerInput.text[len - 1] = '\0';
                    }
                }
            }
        }

        renderGameUI(&ui);
    }

    // Cleanup
    if (ui.questionImage)
    {
        SDL_DestroyTexture(ui.questionImage);
    }
    TTF_CloseFont(ui.font);
    SDL_DestroyRenderer(ui.renderer);
    SDL_DestroyWindow(ui.window);
}

void renderFinalScreen(SDL_Window *window, SDL_Renderer *renderer, TTF_Font *font, const char *buffer)
{
    bool quit = false;
    SDL_Event e;

    // Create a local copy of the buffer that we can modify
    char localBuffer[BUFFER_SIZE];
    strncpy(localBuffer, buffer, BUFFER_SIZE - 1);
    localBuffer[BUFFER_SIZE - 1] = '\0';

    // Parse scores from buffer
    PlayerScore players[3] = {0};
    char *token = strtok(localBuffer + 1, " // "); // Skip message type
    int player_idx = 0;

    while (token != NULL && player_idx < 3)
    {
        sscanf(token, "%[^,],%d", players[player_idx].username, &players[player_idx].score);
        token = strtok(NULL, " // ");
        player_idx++;
    }

    // Create quit button
    Button quitButton = {
        .rect = {900, 500, 100, 40},
        .text = "Quit",
        .isHovered = false};

    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
            else if (e.type == SDL_MOUSEBUTTONDOWN)
            {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);

                if (isMouseOverRect(&quitButton.rect, mouseX, mouseY))
                {
                    quit = true;
                }
            }
        }

        // Clear screen with white background
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Render "Final Score" title
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface *surface = TTF_RenderText_Solid(font, "Final Score", textColor);
        if (surface)
        {
            SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture)
            {
                SDL_Rect titleRect = {
                    (1024 - surface->w) / 2, // Center horizontally
                    50,                      // Top margin
                    surface->w,
                    surface->h};
                SDL_RenderCopy(renderer, texture, NULL, &titleRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }

        // Define dashboard layout
        const int startY = 200;
        const int spacing = 150;
        const int centerX = 1024 / 2;

        // Render each player's score in a dashboard layout
        for (int i = 0; i < 3; i++)
        {
            if (strlen(players[i].username) > 0)
            {
                // Create score box
                SDL_Rect scoreBox = {
                    centerX - 300 + (i * 300), // Spread horizontally from center
                    startY,                    // Fixed vertical position
                    200,                       // Box width
                    100                        // Box height
                };

                // Draw score box border
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &scoreBox);

                // Render username
                surface = TTF_RenderText_Solid(font, players[i].username, textColor);
                if (surface)
                {
                    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (texture)
                    {
                        SDL_Rect userRect = {
                            scoreBox.x + (scoreBox.w - surface->w) / 2, // Center in box
                            scoreBox.y + 20,
                            surface->w,
                            surface->h};
                        SDL_RenderCopy(renderer, texture, NULL, &userRect);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surface);
                }

                // Render score
                char scoreText[32];
                snprintf(scoreText, sizeof(scoreText), "Score: %d", players[i].score);
                surface = TTF_RenderText_Solid(font, scoreText, textColor);
                if (surface)
                {
                    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (texture)
                    {
                        SDL_Rect scoreRect = {
                            scoreBox.x + (scoreBox.w - surface->w) / 2, // Center in box
                            scoreBox.y + 60,
                            surface->w,
                            surface->h};
                        SDL_RenderCopy(renderer, texture, NULL, &scoreRect);
                        SDL_DestroyTexture(texture);
                    }
                    SDL_FreeSurface(surface);
                }
            }
        }

        // Render quit button
        renderButton(renderer, font, &quitButton, true);

        SDL_RenderPresent(renderer);
    }
}