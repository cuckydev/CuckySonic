#include "Game.h"
#include "Error.h"
#include "Render.h"

#include "SDL_events.h"

GAMEMODE gGameMode;

bool EnterGameLoop()
{
	//Initialize game memory
	gGameMode = GAMEMODE_SPLASH; //Start at splash screen
	
	//Run game code
	bool noExit = true;
	bool noError = true;
	
	while (noExit)
	{
		switch (gGameMode)
		{
			case GAMEMODE_SPLASH:
				noExit = false;//GM_Splash(&noError);
				break;
			default:
				noExit = false;
				break;
		}
	}
	
	//End game loop
	return noError;
}
