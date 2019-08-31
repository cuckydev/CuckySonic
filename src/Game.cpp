#include "Game.h"
#include "Log.h"
#include "Error.h"
#include "GM.h"

#include "SDL_events.h"

//Debug bool
bool gDebugEnabled = false;

//Score, time, rings, and lives
unsigned int gScore;
unsigned int gTime;
unsigned int gRings;
unsigned int gLives;

//Generic game functions
void AddToScore(unsigned int score)
{
	//TODO: there's some extra life code that's supposed to be done here
	gScore += score;
}

void CollectRing()
{
	//Increment ring count
	if (gRings < 999)
		gRings++;
	PlaySound(SOUNDID_RING);
}

//Game loop
GAMEMODE gGameMode;

bool EnterGameLoop()
{
	//Initialize game memory
	gGameMode = GAMEMODE_SPLASH; //Start at splash screen
	
	//Initialize our score, time, rings and lives
	gScore = 0;
	gTime = 0;
	gRings = 0;
	gLives = 3;
	
	//Run game code
	bool noExit = true;
	bool noError = true;
	
	while (noExit)
	{
		switch (gGameMode)
		{
			case GAMEMODE_SPLASH:
				noExit = GM_Splash(&noError);
				break;
			case GAMEMODE_TITLE:
				noExit = GM_Title(&noError);
				break;
			case GAMEMODE_DEMO:
			case GAMEMODE_GAME:
				noExit = GM_Game(&noError);
				break;
			default:
				noExit = false;
				break;
		}
	}
	
	//End game loop
	return noError;
}
