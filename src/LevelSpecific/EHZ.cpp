#include "../Level.h"
#include "../Game.h"

void EHZ_PaletteCycle()
{
	//Palette cycle 0 (waterfall and background)
	if (++gLevel->palCycle[0].timer >= 8)
	{
		//Cycle colours and reset timer
		gLevel->palCycle[0].timer = 0;
		
		//Cycle the tile texture (using the background palette, because it won't change until after)
		gLevel->tileTexture->loadedPalette->colour[0x1F] = gLevel->background->texture->loadedPalette->colour[0x1E];
		gLevel->tileTexture->loadedPalette->colour[0x1E] = gLevel->background->texture->loadedPalette->colour[0x14];
		gLevel->tileTexture->loadedPalette->colour[0x14] = gLevel->background->texture->loadedPalette->colour[0x13];
		gLevel->tileTexture->loadedPalette->colour[0x13] = gLevel->background->texture->loadedPalette->colour[0x1F];
		
		//Copy the background's palette from the tile texture
		gLevel->background->texture->loadedPalette->colour[0x13] = gLevel->tileTexture->loadedPalette->colour[0x13];
		gLevel->background->texture->loadedPalette->colour[0x14] = gLevel->tileTexture->loadedPalette->colour[0x14];
		gLevel->background->texture->loadedPalette->colour[0x1E] = gLevel->tileTexture->loadedPalette->colour[0x1E];
		gLevel->background->texture->loadedPalette->colour[0x1F] = gLevel->tileTexture->loadedPalette->colour[0x1F];
	}
}

static const uint8_t ehzScrollRipple[] =
{
	1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0,
	2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
	1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0,
	2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
	1, 2
};

void EHZ_Background(BACKGROUND *background, bool doScroll, int cameraX, int cameraY)
{
	(void)cameraY;
	
	//Get our scroll values
	int16_t scrollBG1 = cameraX / 0x40;
	int16_t scrollBG2 = (cameraX * 0x18) / 0x100;
	int16_t scrollBG3 = cameraX / 0x10;
	int16_t scrollBG4 = cameraX / 0x08;
	int16_t scrollBG5 = cameraX / 0x02;
	
	//Clouds, sky, and islands
	RECT sky = {0,  0, background->texture->width,  80};
	background->DrawStrip(&sky, LEVEL_RENDERLAYER_BACKGROUND, 0, -scrollBG1, -scrollBG1);
	
	//Rippling water at the horizon (change ripple every 8 frames)
	static int horWaterTimer = 4;
	static uint16_t horWaterRipple = 0;
	
	if (doScroll)
	{
		if ((horWaterTimer++ & 0x7) == 0)
			--horWaterRipple;
	}
	
	RECT waterRipple = {0,  80, background->texture->width,  1};
	for (int i = 0; i < 21; i++)
	{
		int x = -(scrollBG1 + ehzScrollRipple[(horWaterRipple & 0x1F) + i]);
		background->DrawStrip(&waterRipple, LEVEL_RENDERLAYER_BACKGROUND, waterRipple.y++, x, x);
	}
	
	//Water
	RECT water = {0, 101, background->texture->width,  11};
	background->DrawStrip(&water, LEVEL_RENDERLAYER_BACKGROUND, 101, 0, 0);
	
	//Draw mountains
	RECT upperMountain = {0, 112, background->texture->width,  16};
	RECT lowerMountain = {0, 128, background->texture->width,  16};
	background->DrawStrip(&upperMountain, LEVEL_RENDERLAYER_BACKGROUND, 112, -scrollBG3, -scrollBG3);
	background->DrawStrip(&lowerMountain, LEVEL_RENDERLAYER_BACKGROUND, 128, -scrollBG2, -scrollBG2);
	
	//Draw field
	uint32_t delta = (((scrollBG5 - scrollBG4) * 0x100) / 0x30) * 0x100;
	uint32_t accumulate = (cameraX / 8) * 0x10000;
	
	RECT strip = {0, 144, background->texture->width, 1};
	for (int i = 144; i < background->texture->height;)
	{
		int mult = (i >= 177) ? 3 : (i >= 159 ? 2 : 1);
		for (int v = 0; v < mult && i < background->texture->height; v++)
		{
			background->DrawStrip(&strip, LEVEL_RENDERLAYER_BACKGROUND, strip.y++, -accumulate / 0x10000, -accumulate / 0x10000);
			i++;
		}
		
		accumulate += delta * mult;
	}
}
