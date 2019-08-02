#include "Game.h"
#include "Error.h"
#include "Render.h"

#include "SDL_events.h"

GAMEMODE gGameMode;

bool EnterGameLoop()
{
	//Initialize game memory
	gGameMode = GAMEMODE_SPLASH; //Start at splash screen
	
	PALETTE *newPalette = new PALETTE;
	SetPaletteColour(&newPalette->colour[0], 0xFF, 0xFF, 0xFF);
	SetPaletteColour(&newPalette->colour[1], 0xC0, 0xC0, 0xFF);
	SetPaletteColour(&newPalette->colour[2], 0xA0, 0xA0, 0xFF);
	SetPaletteColour(&newPalette->colour[3], 0x80, 0x80, 0xFF);
	SetPaletteColour(&newPalette->colour[4], 0x60, 0x60, 0xFF);
	SetPaletteColour(&newPalette->colour[5], 0x40, 0x40, 0xFF);
	SetPaletteColour(&newPalette->colour[6], 0x20, 0x20, 0xFF);
	SetPaletteColour(&newPalette->colour[7], 0x00, 0x00, 0xFF);
	
	TEXTURE *newTexture = new TEXTURE("SplashNew.bmp");
	
	while (1)
	{
		SDL_Event event;
		bool quit = false;
		
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					quit = true;
					break;
				default:
					break;
			}
		}
		
		if (quit)
			break;
		
		SDL_Rect src = {0, 0, 320, 224};
		newTexture->Draw(newPalette, &src, 0, 0);
		gSoftwareBuffer->RenderToScreen(&newPalette->colour[0]);
	}
	
	delete newPalette;
	delete newTexture;
	
	//Run game code
	bool noExit = true;
	bool noError = true;
	
	while (noExit)
	{
		switch (gGameMode)
		{
			case GAMEMODE_SPLASH:
				noExit = false;//GM_Splash(&noError);
				break;
			default:
				noExit = false;
				break;
		}
	}
	
	//End game loop
	return noError;
}
