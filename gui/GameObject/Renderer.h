#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "GameManager/Singleton.h"
class Renderer :public CSingleton<Renderer>{
public:
	Renderer();
	~Renderer();
	bool Init();
	SDL_Window* GetWindow();
	void SetWindow(SDL_Window* mWindow);
	SDL_Renderer* GetRenderer();
	void SetRenderer(SDL_Renderer* mRenderer);
private:
	SDL_Window* gWindow;
	SDL_Renderer* gRenderer;
};