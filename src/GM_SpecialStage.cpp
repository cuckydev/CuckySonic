#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "Input.h"
#include "SpecialStage.h"

bool GM_SpecialStage(bool *noError)
{
	//Load the special stage
	SPECIALSTAGE *stage = new SPECIALSTAGE("data/SpecialStage/Stage/1");
	if (stage->fail != nullptr)
		return (*noError = false);
	
	//Our loop
	bool noExit = true;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Update and draw stage
		stage->Update();
		stage->Draw();
		
		//Render our software buffer to the screen
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&stage->backgroundTexture->loadedPalette->colour[0])))
			break;
		
		if (gController[0].press.a)
			break;
	}
	
	//Unload the stage
	delete stage;
	
	//Return to stage
	gGameMode = GAMEMODE_GAME;
	return noExit;
}
