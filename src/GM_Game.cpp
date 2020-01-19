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

static const char *sonicAndTails[] =	{"data/Sonic/Sonic", "data/Sonic/Sonic", nullptr};
static const char *sonicOnly[] =		{"data/Sonic/Sonic", nullptr};
static const char *tailsOnly[] =		{"data/Sonic/Sonic", nullptr};
static const char *knucklesOnly[] =		{"data/Sonic/Sonic", nullptr};

static const char **characterSetList[] = {
	sonicAndTails,
	sonicOnly,
	tailsOnly,
	knucklesOnly,
};

bool GM_Game(bool *bError)
{
	//Load level with characters given
	gLevel = new LEVEL(gGameLoadLevel, characterSetList[gGameLoadCharacter]);
	if (gLevel->fail != nullptr)
		return (*bError = true);
	
	//Fade level from black
	gLevel->SetFade(true, false);
	
	//Our loop
	bool bExit = false;
	
	while (!(bExit || *bError))
	{
		//Handle events
		bExit = HandleEvents();
		
		//Update level
		if ((*bError = gLevel->Update()) == true)
			break;
		
		//Handle level fading
		bool breakThisState = false;
		
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
		if ((*bError = gSoftwareBuffer->RenderToScreen(&gLevel->background->texture->loadedPalette->colour[0])) == true)
			break;
		
		//Go to next state if set to break this state
		if (breakThisState)
			break;
	}
	
	//Unload level and exit
	delete gLevel;
	return bExit;
}
