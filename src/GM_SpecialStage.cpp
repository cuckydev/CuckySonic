#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "MathUtil.h"
#include "Input.h"
#include "Audio.h"

#define SPLASH_TIME 200
#define SPLASH_WARP 80

bool GM_SpecialStage(bool *noError)
{
	//Our loop
	bool noExit = true;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(NULL)))
			break;
	}
	
	//Return to stage
	gGameMode = GAMEMODE_GAME;
	return noExit;
}
