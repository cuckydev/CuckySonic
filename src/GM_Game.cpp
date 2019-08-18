#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "Level.h"

LEVEL *gLevel;

bool GM_Game(bool *noError)
{
	PALCOLOUR blank;
	SetPaletteColour(&blank, 0xFF, 0x00, 0xFF);
	
	//Load Level (EHZ1)
	gLevel = new LEVEL(2);
	if (gLevel->fail != NULL)
		return (*noError = false);
	
	//Our loop
	bool noExit = true;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Update level
		gLevel->Update();
		gLevel->Draw();
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&blank)))
			return false;
	}
	
	//Unload level
	delete gLevel;
	
	//Exit
	return noExit;
}
