#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Input.h"
#include "Render.h"
#include "Fade.h"
#include "Level.h"

LEVEL *gLevel;

int gGameLoadLevel;
int gGameLoadCharacter;

const char *p0[] = {"data/Sonic/Sonic"};
const char *p1[] = {"data/Sonic/Sonic", "data/Tails/Tails"};
const char *p2[] = {"data/Knuckles/Knuckles"};

struct
{
	int players;
	const char **playerPaths;
} characters[] = {
	{1, p0}, //0 - Sonic Only
	{2, p1}, //1 - Sonic and Tails
	{1, p2}, //2 - Knuckles
};

bool GM_Game(bool *noError)
{
	//Load level with characters given
	gLevel = new LEVEL(gGameLoadLevel, characters[gGameLoadCharacter].players, characters[gGameLoadCharacter].playerPaths);
	if (gLevel->fail != NULL)
		return (*noError = false);
	
	//Initialize level fade
	ClearControllerInput();
	gLevel->Update();
	gLevel->SetFade(true, false);
	
	//Our loop
	bool noExit = true;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Update level
		bool breakThisState = false;
		
		if (!gLevel->fading)
		{
			gLevel->Update();
		}
		else
		{
			if (gLevel->isFadingIn)
			{
				gLevel->fading = !gLevel->UpdateFade();
			}
			else
			{
				//Fade out and enter next game state
				if (gLevel->UpdateFade())
				{
					gGameMode = gLevel->specialFade ? GAMEMODE_SPECIALSTAGE : (gGameMode == GAMEMODE_DEMO ? GAMEMODE_SPLASH : GAMEMODE_GAME);
					breakThisState = true;
				}
			}
		}
		
		gLevel->Draw();
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&gLevel->backgroundTexture->loadedPalette->colour[0])))
			return false;
		
		//Go to next state if set to break this state
		if (breakThisState)
			break;
	}
	
	//Unload level
	delete gLevel;
	
	//Exit
	return noExit;
}
