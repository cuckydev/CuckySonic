#include "Game.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "MathUtil.h"
#include "Input.h"
#include "Audio.h"

#define SPLASH_TIME 100
#define TRANSITION_TIME 30

bool GM_Splash(bool *bError)
{
	//Load our textures
	TEXTURE splashTexture("data/Splash.bmp");
	if (splashTexture.fail != nullptr)
		return (*bError = Error(splashTexture.fail));
	
	FillPaletteWhite(splashTexture.loadedPalette);
	
	//Our loop
	bool bExit = false;
	
	bool didPlayJingle = false;
	unsigned int frame = 0, animFrame = 0;
	
	while (!(bExit || *bError))
	{
		//Handle events
		bExit = HandleEvents();
		
		//Handle fading
		if (gController[0].press.a || gController[0].press.b || gController[0].press.c || gController[0].press.start)
			frame = mmax(frame, SPLASH_TIME);
		
		bool bBreak = false;
		if (frame < SPLASH_TIME)
		{
			//Fade in and play splash jingle once done
			if (PaletteFadeInFromWhite(splashTexture.loadedPalette))
			{
				if (!didPlayJingle)
					PlaySound(SOUNDID_SPLASHJINGLE);
				didPlayJingle = true;
			}
		}
		else if (frame >= SPLASH_TIME + TRANSITION_TIME)
			bBreak = true;
		
		//Draw splash
		RECT strip = {0, 0, splashTexture.width, 1};
		
		for (int y = 0; y < gRenderSpec.height; y++)
		{
			//Get our distortion
			int xOff;
			int inY = y - (gRenderSpec.height - splashTexture.height) / 2;
			
			xOff = GetSin((y) + (animFrame * 2)) * 15 / 0x100;
			inY += GetCos((y) + (animFrame * 4)) * 4 / 0x100;
			
			if (frame > SPLASH_TIME)
			{
				const double div = 1.0 / (1.0 - ((double)(frame - SPLASH_TIME) / TRANSITION_TIME));
				inY = (int)((double)(inY - splashTexture.height / 2) / div) + (splashTexture.height / 2);
			}
			
			//Draw strip
			if (inY >= 0 && inY < splashTexture.height)
			{
				strip.y = inY;
				gSoftwareBuffer->DrawTexture(&splashTexture, splashTexture.loadedPalette, &strip, 0, (gRenderSpec.width - splashTexture.width) / 2 + xOff, y, false, false);
			}
		}
		
		//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
		if ((*bError = gSoftwareBuffer->RenderToScreen(&splashTexture.loadedPalette->colour[0])) == true)
			break;
		
		//Exit if faded out
		if (bBreak)
			break;
		
		//Increment frame counter
		if (!(gController[0].held.left && gController[0].held.right))
			frame++;
		animFrame++;
	}
	
	//Go to title
	gGameMode = GAMEMODE_TITLE;
	return bExit;
}
