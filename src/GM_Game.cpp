#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "Level.h"

bool GM_Game(bool *noError)
{
	//Load level (GHZ1 for testing)
	LEVEL *level = new LEVEL(0);
	if (level->fail != NULL)
		return (*noError = false);
	
	//Our loop
	bool noExit = true;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		level->Draw();
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(NULL)))
			return false;
	}
	
	//Unload level
	delete level;
	
	//Exit
	return noExit;
}
