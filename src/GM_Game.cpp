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
	//Load Level (EHZ1)
	gLevel = new LEVEL(2);
	if (gLevel->fail != NULL)
		return (*noError = false);
	
	//Initialize level fade
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
		if (!gLevel->fading)
			gLevel->Update();
		else
			gLevel->fading = !gLevel->UpdateFade();
		
		gLevel->Draw();
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&gLevel->backgroundTexture->loadedPalette->colour[0])))
			return false;
	}
	
	//Unload level
	delete gLevel;
	
	//Exit
	return noExit;
}
