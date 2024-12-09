#include"Renderer.h"
#include "Define.h"
Renderer::Renderer()
{
	gWindow = NULL;
	gRenderer = NULL;
}
	
Renderer::~Renderer()
{
	//Destroy window	
	if (gRenderer != nullptr) {
		SDL_DestroyRenderer(gRenderer);
		gRenderer = nullptr;
	}

	// Destroy window if it's initialized
	if (gWindow != nullptr) {
		SDL_DestroyWindow(gWindow);
		gWindow = nullptr;
	}
	IMG_Quit();
	TTF_Quit();
}
bool Renderer::Init()
{
	//Initialization flag
	bool success = true;
	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}
	//Create window
	gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
	{
		printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create renderer for window ( tao doi tuong renderer de co the render on window)
		gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		if (gRenderer == NULL)
		{
			printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Initialize renderer color
			SDL_SetRenderDrawColor(Renderer::GetInstance()->GetRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);
		}
	}

	//	//Initialize PNG loading
	bool ret = true;
	int imgFlags = IMG_INIT_PNG; //we can use other type maybe jpg, bmp...
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		ret = false;
	}

	//Initialize SDL_ttf
	if (TTF_Init() == -1)
	{
		printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
		ret = false;
	}

	return (success && ret);
}
SDL_Window* Renderer::GetWindow()
{
	return gWindow;
}
SDL_Renderer* Renderer::GetRenderer()
{
	return gRenderer;
}
void Renderer::SetWindow(SDL_Window* mWindow)
{
	gWindow = mWindow;
}

void Renderer::SetRenderer(SDL_Renderer* mRenderer)
{
	gRenderer = mRenderer;
}
