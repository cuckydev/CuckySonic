#include "SDL_events.h"
#include "Input.h"
#include "Audio.h"

bool HandleEvents()
{
	bool noExit = true;
	bool focusYield = false;
	
	int musicPoint = -1;
	
	while (SDL_PollEvent(NULL) || (focusYield && noExit))
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
						
						//Resume music
						if (musicPoint >= 0)
							ResumeMusic(musicPoint);
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST: //Window unfocused
						//Yield game until refocused
						focusYield = true;
						
						//Clear controller input and pause music
						ClearControllerInput();
						musicPoint = PauseMusic();
						break;
					default:
						break;
				}
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
