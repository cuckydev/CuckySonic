#include "SDL_events.h"
#include "Input.h"
#include "Audio.h"
#include "Render.h"

bool HandleEvents()
{
	bool noExit = true;
	bool focusYield = false;
	
	while (SDL_PollEvent(nullptr) || (focusYield && noExit))
	{
		//Wait for next event (instantaneous if focus gained, just polling then stopping when done)
		SDL_Event event;
		SDL_WaitEvent(&event);
		
		switch (event.type)
		{
			case SDL_QUIT:	//Window/game is closed
				noExit = false;
				break;
			
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_FOCUS_GAINED: //Window focused
						//Unyield game
						focusYield = false;
						YieldAudio(false);
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST: //Window unfocused
						//Yield game until refocused
						focusYield = true;
						YieldAudio(true);
						
						//Clear controller input
						ClearControllerInput();
						break;
					default:
						break;
				}
				break;
		}
		
		//Check this event for input
		HandleInputEvent(&event);
	}
	
	//Update our input (such as pressed keys, or analogue stick input)
	UpdateInput();
	
	return noExit;
}
