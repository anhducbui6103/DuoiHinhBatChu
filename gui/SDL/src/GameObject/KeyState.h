#pragma once
#include <SDL.h>
class KeyState
{
public:
	static void HandleKeyState()
	{
		keystate = SDL_GetKeyboardState(NULL);
	}
	static bool IsKeyPressed(SDL_Scancode scancode)
	{
		return keystate[scancode];
	}
private:
	static inline const Uint8* keystate = NULL;
};