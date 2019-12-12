#include "SDL_events.h"

bool Backend_HandleEvents()
{
	bool exit = false;
	bool focusYield = false;
	
	while (SDL_PollEvent(nullptr) || (focusYield && (!exit)))
	{
		//Wait for next event (instantaneous if focus gained, just polling then stopping when done)
		SDL_Event event;
		SDL_WaitEvent(&event);
		
		switch (event.type)
		{
			case SDL_QUIT:	//Window/game is closed
				exit = true;
				break;
			
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_FOCUS_GAINED: //Window focused
						//Unyield game
						focusYield = false;
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST: //Window unfocused
						//Yield game until refocused
						focusYield = true;
						break;
					default:
						break;
				}
				break;
		}
	}
	
	return exit;
}
