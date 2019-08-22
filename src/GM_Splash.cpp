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

bool GM_Splash(bool *noError)
{
	//Load our splash image
	TEXTURE *splashTexture = new TEXTURE(NULL, "data/Splash.bmp");
	if (splashTexture->fail != NULL)
		return (*noError = false);
	
	//Make our palette white
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
		if (frame < (FADE_TIME * 4) && (frame % 4) == 0)
		{
			if (PaletteFadeInFromWhite(splashTexture->loadedPalette))
				PlaySound(SOUNDID_SPLASHJINGLE);
		}
		else if (frame >= SPLASH_TIME - FADE_TIME)
		{
			PaletteFadeOutToBlack(splashTexture->loadedPalette);
		}
		else
		{
			if (gController[0].held.a || gController[0].held.b || gController[0].held.c)
			{
				frame = SPLASH_TIME - FADE_TIME; //Skip splash screen if ABC is pressed
				StopSound(SOUNDID_SPLASHJINGLE);
			}
		}
		
		if (gController[0].held.a && gController[0].held.b && gController[0].held.c && gController[0].press.start && gDebugEnabled == false)
		{
			PlaySound(SOUNDID_RING);
			gDebugEnabled = true;
		}
		
		//Render our splash texture
		SDL_Rect src = {0, 0, splashTexture->width, 1};
		
		for (int line = 0; line < splashTexture->height; line++)
		{
			//Get our draw offset
			int16_t sin;
			GetSine((line & 0xFE) + frame, &sin, NULL);
			
			int xOff = (sin * 2) * (frame < SPLASH_WARP ? (SPLASH_WARP - frame) : 0) / SPLASH_WARP;
			if (line & 0x1)
				xOff = -xOff;
			
			//Draw this line
			splashTexture->Draw(0, splashTexture->loadedPalette, &src, (SCREEN_WIDTH - splashTexture->width) / 2 + xOff, (SCREEN_HEIGHT - splashTexture->height) / 2 + line, false, false);
			src.y++;
		}
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&splashTexture->loadedPalette->colour[0])))
			return false;
		
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
