#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"

#include "SDL_mouse.h"

#define SPLASH_TIME 1500

bool GM_Splash(bool *noError)
{
	//Load our splash image
	TEXTURE *splashTexture = new TEXTURE("data/Splash.bmp");
	if (splashTexture->fail != NULL)
	{
		*noError = Error(splashTexture->fail);
		return false;
	}
	
	//Make our palette white
	FillPaletteWhite(splashTexture->loadedPalette);
	
	//TEST: FUNNY SONIC CURSOR!!!
	TEXTURE *sonicTexture = new TEXTURE("data/Sonic.bmp");
	
	//Our loop
	bool noExit = true;
	int frame = 0;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Fade in/out
		if (frame < FADE_TIME)
			PaletteFadeInFromWhite(splashTexture->loadedPalette);
		if (frame >= SPLASH_TIME - FADE_TIME)
			noExit = !PaletteFadeOutToBlack(splashTexture->loadedPalette);
		
		//Render our splash texture
		SDL_Rect src = {0, 0, splashTexture->width, splashTexture->height};
		splashTexture->Draw(splashTexture->loadedPalette, &src, (SCREEN_WIDTH - splashTexture->width) / 2, (SCREEN_HEIGHT - splashTexture->height) / 2);
		
		//Render sonic the hedgehog
		int x, y;
		SDL_GetMouseState(&x, &y);
		SDL_Rect sonSrc = {0, 0, 32, 40};
		sonicTexture->Draw(sonicTexture->loadedPalette, &sonSrc, x / 2 - 16, y / 2 - 20);
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&splashTexture->loadedPalette->colour[0])))
			return false;
		
		//Increment frame counter
		frame++;
	}
	
	//Unload our splash screen texture
	delete splashTexture;
	delete sonicTexture;
	
	//Go to title
	gGameMode = GAMEMODE_TITLE;
	return noExit;
}