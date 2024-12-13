#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main()
{
    // Khởi tạo SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Tạo cửa sổ
    SDL_Window *window = SDL_CreateWindow("Waiting for players...",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Tạo renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Khởi tạo SDL_ttf
    if (TTF_Init() == -1)
    {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Tải font (thay thế bằng đường dẫn font ttf của bạn)
    TTF_Font *font = TTF_OpenFont("Assets/Fonts/OpenSans-Regular.ttf", 48);
    if (!font)
    {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Text thông báo
    SDL_Color textColor = {0, 0, 0, 255};
    char message[50] = "Please wait for other players...";
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, message, textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {SCREEN_WIDTH / 2 - textSurface->w / 2, SCREEN_HEIGHT / 2 - textSurface->h / 2, textSurface->w, textSurface->h};

    SDL_FreeSurface(textSurface);

    // Biến để tạo hiệu ứng thay đổi dấu chấm
    int dotCount = 0;
    int dotMax = 3;
    Uint32 lastTime = SDL_GetTicks();

    // Vòng lặp chính
    int quit = 0;
    SDL_Event e;
    while (!quit)
    {
        // Xử lý sự kiện
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = 1;
            }
        }

        // Cập nhật hiệu ứng dấu chấm
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime > 500)
        { // Cập nhật mỗi nửa giây
            lastTime = currentTime;
            dotCount = (dotCount + 1) % (dotMax + 1); // Đổi số lượng dấu chấm từ 0 đến 3
            snprintf(message, sizeof(message), "Please wait for other players");
            for (int i = 0; i < dotCount; ++i)
            {
                strncat(message, ".", sizeof(message) - strlen(message) - 1); // Thêm dấu chấm vào cuối
            }

            // Tạo lại text surface và texture
            SDL_Surface *newTextSurface = TTF_RenderText_Solid(font, message, textColor);
            SDL_DestroyTexture(textTexture); // Giải phóng texture cũ
            textTexture = SDL_CreateTextureFromSurface(renderer, newTextSurface);
            textRect.w = newTextSurface->w;
            textRect.h = newTextSurface->h;
            SDL_FreeSurface(newTextSurface);
        }

        // Vẽ background và text
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

        // Cập nhật màn hình
        SDL_RenderPresent(renderer);

        // Dừng vòng lặp một chút để CPU không quá tải
        SDL_Delay(16); // Thời gian trễ để giúp frame rate ổn định
    }

    // Dọn dẹp
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
