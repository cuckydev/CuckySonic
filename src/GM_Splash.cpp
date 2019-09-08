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
	
	//Make our palette white for fade-in
	FillPaletteWhite(splashTexture->loadedPalette);
	
	//Our loop
	bool noExit = true;
	
	bool fadedIn = false;
	int frame = 0;
	int debugTextX = 0;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Fade in/out
		if (frame < SPLASH_TIME - FADE_TIME)
		{
			//Fade in
			if (!fadedIn)
			{
				//Fade in from white (initial fade-in)
				if (PaletteFadeInFromWhite(splashTexture->loadedPalette))
					fadedIn = true;
			}
			else
			{
				//Fade in from black (returning after)
				PaletteFadeInFromBlack(splashTexture->loadedPalette);
			}
			
			//Skip screen if ABC is pressed
			if ((gController[0].press.a || gController[0].press.b || gController[0].press.c) && frame > SPLASH_WARP && frame > FADE_TIME)
				frame = SPLASH_TIME - FADE_TIME; //Skip splash screen if ABC is pressed
		}
		else if (PaletteFadeOutToBlack(splashTexture->loadedPalette))
		{
			//Stop the jingle when transitioning to title
			StopSound(SOUNDID_SPLASHJINGLE);
		}
		
		//Debug cheat
		if (gController[0].held.a && gController[0].held.b && gController[0].held.c && gController[0].press.start && gDebugEnabled == false)
		{
			if (frame > SPLASH_WARP && fadedIn)
				frame = SPLASH_WARP; //Fade back in and restart timer
			PlaySound(SOUNDID_RING);
			gDebugEnabled = true;
		}
		else
		{
			//Play "Cucky" jingle if splash texture has finally finished the effect
			if (frame == SPLASH_WARP)
				PlaySound(SOUNDID_SPLASHJINGLE);
		}
		
		//Render our "debug enabled text"
		if (gDebugEnabled == true)
		{
			//Scroll in
			const int goalX = splashTexture->width;
			if ((debugTextX += 16) >= goalX)
				debugTextX = goalX;
			
			//Draw
			SDL_Rect src = {0, splashTexture->height - 16, splashTexture->width, 16};
			splashTexture->Draw(0, splashTexture->loadedPalette, &src, debugTextX - splashTexture->width, SCREEN_HEIGHT - 16, false, false);
		}
		
		//Render our splash texture
		SDL_Rect src = {0, 0, splashTexture->width, 1};
		
		for (int line = 0; line < (splashTexture->height - 16); line++)
		{
			//Get our draw offset
			int16_t sin;
			GetSine((line & 0xFE) + frame, &sin, NULL);
			
			int xOff = (sin * 2) * (frame < SPLASH_WARP ? (SPLASH_WARP - frame) : 0) / SPLASH_WARP;
			if (line & 0x1)
				xOff = -xOff;
			
			//Draw this line
			splashTexture->Draw(0, splashTexture->loadedPalette, &src, (SCREEN_WIDTH - splashTexture->width) / 2 + xOff, (SCREEN_HEIGHT - (splashTexture->height - 16)) / 2 + line, false, false);
			src.y++;
		}
		
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
