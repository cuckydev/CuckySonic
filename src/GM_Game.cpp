#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "Player.h"

bool GM_Game(bool *noError)
{
	//Our loop
	bool noExit = true;
	
	//Background colour for testing
	PALCOLOUR backColour;
	SetPaletteColour(&backColour, 255, 0, 255);
	ModifyPaletteColour(&backColour, 0, 0, 0);
	
	PLAYER *player = new PLAYER("data/Sonic", NULL, 0);
	if (player->fail != NULL)
		return (*noError = false);
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Fade in background
		FadeInFromBlack(&backColour);
		
		//Sonic the hedgehog
		player->Update();
		player->Draw();
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&backColour)))
			return false;
	}
	
	delete player;
	
	//Return to title
	gGameMode = GAMEMODE_TITLE;
	return noExit;
}
