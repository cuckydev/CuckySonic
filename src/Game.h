#pragma once
#include "Level.h"

enum GAMEMODE
{
	GAMEMODE_SPLASH,		//"Sega" screen, but can be used for general splash
	GAMEMODE_TITLE,			//Title screen
	GAMEMODE_DEMO,			//Identical to game, but uses demo inputs
	GAMEMODE_GAME,			//Gameplay
	GAMEMODE_SPECIALSTAGE,	//Special stage
	GAMEMODE_CONTINUE,		//Continue screen
	GAMEMODE_ENDING,		//Ending sequence
	GAMEMODE_CREDITS,		//Credits sequence
};

extern GAMEMODE gGameMode;
extern LEVEL *gLevel;

bool EnterGameLoop();
