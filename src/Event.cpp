#include "SDL_events.h"
#include "Input.h"

bool HandleEvents()
{
	SDL_Event event;
	bool noExit = true;
	
	//TODO: add code to pause game when window is unfocused
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:	//Window/game is closed
				noExit = false;
				break;
				
			case SDL_KEYDOWN:	//Input events
			case SDL_KEYUP:
				HandleInputEvent(&event);
				break;
				
			default:	//Unhandled just to be handled by our operating system
				break;
		}
	}
	
	//Update our input (such as pressed keys, or analogue stick input)
	UpdateInput();
	
	return noExit;
}
