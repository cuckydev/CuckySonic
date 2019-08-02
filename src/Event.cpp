#include "SDL_events.h"

bool HandleEvents()
{
	SDL_Event event;
	bool noExit = true;
	
	//TODO: add code to pause game when window is unfocused
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				noExit = false;
				break;
			default:
				break;
		}
	}
	
	return noExit;
}
