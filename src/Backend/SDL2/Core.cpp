#include "SDL.h"

bool Backend_InitCore()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS) < 0)
		return true;
	return false;
}

void Backend_QuitCore()
{
	//Quit SDL
	SDL_Quit();
}
