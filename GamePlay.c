#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#undef main

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define QUESTION_WIDTH 500
#define QUESTION_HEIGHT 350
#define INPUT_HEIGHT 40
#define MAX_HINT_LENGTH 256

typedef struct Player
{
    char name[50];
    int score;
} Player;

typedef struct Question
{
    char imagePath[256];
    char answer[256];
    int hint;
} Question;

void renderText(SDL_Renderer *renderer, TTF_Font *font, const char *text, int x, int y, SDL_Color color)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderInputBoxes(SDL_Renderer *renderer, TTF_Font *font, const char *displayedAnswer, int x, int y, int numBoxes)
{
    int boxSize = 70; // Kích thước mỗi ô vuông
    int spacing = 10; // Khoảng cách giữa các ô
    SDL_Color borderColor = {0, 0, 0, 255};
    SDL_Color textColor = {0, 0, 0, 255};

    for (int i = 0; i < numBoxes; i++)
    {
        SDL_Rect box = {x + i * (boxSize + spacing), y, boxSize, boxSize};
        SDL_SetRenderDrawColor(renderer, borderColor.r, borderColor.g, borderColor.b, borderColor.a);
        SDL_RenderDrawRect(renderer, &box);

        if (displayedAnswer[i] != '_')
        {
            char letter[2] = {displayedAnswer[i], '\0'};
            renderText(renderer, font, letter, box.x + box.w / 4, box.y + box.h / 10, textColor);
        }
    }
}


void renderInput(SDL_Renderer *renderer, TTF_Font *font, const char *displayText, int x, int y)
{
    renderText(renderer, font, displayText, x + 10, y + (INPUT_HEIGHT / 4), (SDL_Color){0, 0, 0, 255});
}

void renderBorder(SDL_Renderer *renderer, int x, int y, int width, int height, SDL_Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect borderRect = {x, y, width, height};
    SDL_RenderDrawRect(renderer, &borderRect);
}

void renderImage(SDL_Renderer *renderer, const char *imagePath, int x, int y, int width, int height)
{
    SDL_Surface *imageSurface = IMG_Load(imagePath);
    if (!imageSurface)
    {
        printf("Failed to load image: %s\n", IMG_GetError());
        return;
    }

    SDL_Texture *imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    SDL_FreeSurface(imageSurface);

    SDL_Rect destRect = {x, y, width, height};
    SDL_RenderCopy(renderer, imageTexture, NULL, &destRect);
    SDL_DestroyTexture(imageTexture);
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0 || TTF_Init() != 0 || IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
    {
        printf("Failed to initialize SDL, TTF, or IMG: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Catch Phrase", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font *font;

    Player players[3] = {{"Player1", 0}, {"Player2", 0}, {"Player3", 0}};
    Question questions[] = {
        {"Assets/Images/question3.jpg", "GIAYBAC", 7},
        {"Assets/Images/question1.jpg", "BAOHAM", 6},
        {"Assets/Images/question2.jpg", "COLOA", 5},
        {"Assets/Images/question4.jpg", "BACHTHU", 7},
        {"Assets/Images/question5.jpg", "BIOI", 4},
        {"Assets/Images/question6.jpg", "GAUNGUA", 7},
        {"Assets/Images/question7.jpg", "BAIBAC", 6},
    };
    int totalQuestions = sizeof(questions) / sizeof(questions[0]);
    int currentQuestionIndex = 0;

    char displayedAnswer[256];
    memset(displayedAnswer, '_', questions[currentQuestionIndex].hint);
    displayedAnswer[questions[currentQuestionIndex].hint] = '\0';

    // Vị trí và kích thước nút chuông
    SDL_Rect bellRect = {SCREEN_WIDTH -150, QUESTION_HEIGHT + 50, 100, 100}; // Vùng chứa hình chuông

    int bellPressed = 0; // Trạng thái nhấn chuông (1 = đã nhấn, 0 = chưa nhấn)

    SDL_Rect inputBoxes[MAX_HINT_LENGTH]; // Mảng các ô vuông
    int currentBox = 0;                   // Ô hiện tại để nhập
    int numBoxes = questions[currentQuestionIndex].hint;

    int inputIndex = 0;
    int quit = 0;
    SDL_Event event;

    while (!quit)
    {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Render question image
        renderImage(renderer, questions[currentQuestionIndex].imagePath, 30, 30, QUESTION_WIDTH, QUESTION_HEIGHT);

        // Render score board
        renderBorder(renderer, SCREEN_WIDTH - 230, 30, 190, 220, (SDL_Color){0, 0, 0, 255});

        // Render scores
        font = TTF_OpenFont("Assets/Fonts/OpenSans-Regular.ttf", 30);
        int scoreX = SCREEN_WIDTH - 200;
        int scoreY = 40;
        renderText(renderer, font, "Scores:", scoreX, scoreY, (SDL_Color){0, 0, 0, 255});
        scoreY += 50;
        for (int i = 0; i < 3; i++)
        {
            char scoreText[100];
            snprintf(scoreText, sizeof(scoreText), "%s: %d", players[i].name, players[i].score);
            renderText(renderer, font, scoreText, scoreX, scoreY, (SDL_Color){0, 0, 0, 255});
            scoreY += 50;
        }

        // Render input box
        font = TTF_OpenFont("Assets/Fonts/Roboto-Regular.ttf", 60);
        renderInputBoxes(renderer, font, displayedAnswer, 10, QUESTION_HEIGHT + 70, numBoxes);

        // Render bell image
        if (bellPressed == 0)
            renderImage(renderer, "Assets/Images/yellow_bell.png", bellRect.x, bellRect.y, bellRect.w, bellRect.h);
        else
            renderImage(renderer, "Assets/Images/black_bell.png", bellRect.x, bellRect.y, bellRect.w, bellRect.h);

        SDL_RenderPresent(renderer);

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                // Kiểm tra nếu người chơi nhấn vào chuông
                if (mouseX >= bellRect.x && mouseX <= bellRect.x + bellRect.w &&
                    mouseY >= bellRect.y && mouseY <= bellRect.y + bellRect.h && bellPressed == 0) {
                    bellPressed = 1;
                    printf("Bell pressed! Player can answer now.\n");
                }
            }
            if (event.type == SDL_TEXTINPUT)
            {
                char inputChar = event.text.text[0];
                if (isalpha(inputChar) && currentBox < numBoxes)
                {
                    displayedAnswer[currentBox] = toupper(inputChar);
                    currentBox = (currentBox + 1) % numBoxes; // Chuyển sang ô tiếp theo
                }
            }
            if (event.type == SDL_KEYDOWN)
            {
                if (event.key.keysym.sym == SDLK_BACKSPACE && currentBox > 0)
                {
                    currentBox--;
                    displayedAnswer[currentBox] = '_';
                }
                else if (event.key.keysym.sym == SDLK_RETURN && bellPressed == 1)
                {
                    if (currentBox == numBoxes)
                    {
                        if (strcmp(displayedAnswer, questions[currentQuestionIndex].answer) == 0)
                        {
                            printf("Correct answer!\n");
                            players[0].score += 10;
                        }
                        else
                        {
                            printf("Wrong answer!\n");
                        }
                        // Move to next question
                        currentQuestionIndex++;
                        if (currentQuestionIndex >= totalQuestions)
                        {
                            printf("All questions answered!\n");
                            quit = 1;
                        }
                        else
                        {
                            memset(displayedAnswer, '_', questions[currentQuestionIndex].hint);
                            displayedAnswer[questions[currentQuestionIndex].hint] = '\0';
                            currentBox = 0;
                            numBoxes = questions[currentQuestionIndex].hint;
                        }
                    }
                }
                else if (event.key.keysym.sym == SDLK_LEFT && currentBox > 0)
                {
                    currentBox--;
                }
                else if (event.key.keysym.sym == SDLK_RIGHT && currentBox < numBoxes - 1)
                {
                    currentBox++;
                }
            }
        }

        SDL_Delay(16);
    }

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    return 0;
}
