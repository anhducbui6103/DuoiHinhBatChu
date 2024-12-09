#include"Game.h"
#include <stdio.h>
int main(int argc, char* args[])
{
	//Start up SDL and create window
	Game::GetInstance()->Run();
	//Free resources and close SDL
	Game::GetInstance()->FreeInstance();

	return 0;
}