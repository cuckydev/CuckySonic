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

#define SPLASH_TIME 120

bool GM_Splash(bool *noError)
{
	//Load our splash image
	TEXTURE *splashTexture = new TEXTURE(NULL, "data/Splash.bmp");
	if (splashTexture->fail != NULL)
		return (*noError = !Error(splashTexture->fail));
	
	//Make our palette white for fade-in
	FillPaletteWhite(splashTexture->loadedPalette);
	
	//Our loop
	bool noExit = true;
	
	int frame = 0;
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Fade in/out
		if (frame < SPLASH_TIME - FADE_TIME)
		{
			//Fade in
			PaletteFadeInFromWhite(splashTexture->loadedPalette);
			
			//Skip screen if ABC or start is pressed
			if (gController[0].press.start || gController[0].press.a || gController[0].press.b || gController[0].press.c)
				frame = SPLASH_TIME - FADE_TIME; //Skip splash screen if ABC is pressed
		}
		else if (PaletteFadeOutToBlack(splashTexture->loadedPalette))
		{
			//Stop the jingle when transitioning to title
			StopSound(SOUNDID_SPLASHJINGLE);
		}
		
		//Play "Cucky" jingle once faded in
		if (frame == FADE_TIME)
			PlaySound(SOUNDID_SPLASHJINGLE);
		
		//Render our splash texture
		SDL_Rect src = {0, 0, splashTexture->width, splashTexture->height};
		gSoftwareBuffer->DrawTexture(splashTexture, splashTexture->loadedPalette, &src, 0, (gRenderSpec.width - splashTexture->width) / 2, (gRenderSpec.height - splashTexture->height) / 2, false, false);
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&splashTexture->loadedPalette->colour[0])))
			break;
		
		//Increment frame counter (and end once we reach splash time)
		if (frame++ >= SPLASH_TIME)
			break;
	}
	
	//Unload our splash screen texture
	delete splashTexture;
	
	//Go to title
	gGameMode = GAMEMODE_TITLE;
	return noExit;
}
