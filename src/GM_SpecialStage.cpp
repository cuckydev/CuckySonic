#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "Input.h"
#include "SpecialStage.h"

bool GM_SpecialStage(bool *bError)
{
	//Load the special stage
	SPECIALSTAGE stage("data/SpecialStage/Stage/1");
	if (stage.fail != nullptr)
		return (*bError = true);
	
	//Our loop
	bool bExit = false;
	
	while (!(bExit || *bError))
	{
		//Handle events
		bExit = HandleEvents();
		
		//Update and draw stage
		stage.Update();
		stage.Draw();
		
		//Render our software buffer to the screen
		if ((*bError = gSoftwareBuffer->RenderToScreen(&stage.backgroundTexture->loadedPalette->colour[0])) == true)
			break;
		
		if (gController[0].press.a)
			break;
	}
	
	//Return to stage
	gGameMode = GAMEMODE_GAME;
	return bExit;
}
