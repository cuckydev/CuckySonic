#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"

bool GM_Title(bool *noError)
{
	//Our loop
	bool noExit = true;
	
	//Continue to game
	gGameMode = GAMEMODE_GAME;
	return noExit;
}
