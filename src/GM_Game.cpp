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

int gGameLoadLevel = 0;
int gGameLoadCharacter = 0;

static const char *sonicOnly[] =		{"data/Sonic/Sonic", nullptr};
static const char *sonicAndTails[] =	{"data/Sonic/Sonic", "data/Tails/Tails", nullptr};
static const char *tailsOnly[] =		{"data/Knuckles/Knuckles", nullptr};
static const char *knucklesOnly[] =		{"data/Knuckles/Knuckles", nullptr};

static const char **characterSetList[] = {
	sonicOnly,
	sonicAndTails,
	tailsOnly,
	knucklesOnly,
};

bool GM_Game(bool *noError)
{
	//Load level with characters given
	gLevel = new LEVEL(gGameLoadLevel, characterSetList[gGameLoadCharacter]);
	if (gLevel->fail != nullptr)
		return (*noError = false);
	
	//Fade level from black
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
		
		if (!(*noError = gLevel->Update()))
			break;
		
		//Handle level fading
		if (gLevel->fading)
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
		
		//Draw level to the screen
		gLevel->Draw();
		
		//Render our software buffer to the screen
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&gLevel->background->texture->loadedPalette->colour[0])))
			break;
		
		//Go to next state if set to break this state
		if (breakThisState)
			break;
	}
	
	//Unload level and exit
	delete gLevel;
	return noExit;
}
