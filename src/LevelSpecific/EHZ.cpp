#include "../Level.h"
#include "../Game.h"

void EHZ_PaletteCycle(LEVEL *lvl)
{
	//Palette cycle 0 (waterfall and background)
	if (++lvl->palCycle[0].timer >= 8)
	{
		//Cycle colours and reset timer
		lvl->palCycle[0].timer = 0;
		
		//Cycle the tile texture (using the background palette, because backgroundTexture won't change until after)
		lvl->tileTexture->loadedPalette->colour[0x1F] = lvl->backgroundTexture->loadedPalette->colour[0x1E];
		lvl->tileTexture->loadedPalette->colour[0x1E] = lvl->backgroundTexture->loadedPalette->colour[0x14];
		lvl->tileTexture->loadedPalette->colour[0x14] = lvl->backgroundTexture->loadedPalette->colour[0x13];
		lvl->tileTexture->loadedPalette->colour[0x13] = lvl->backgroundTexture->loadedPalette->colour[0x1F];
		
		//Copy the background's palette from the tile texture
		lvl->backgroundTexture->loadedPalette->colour[0x13] = lvl->tileTexture->loadedPalette->colour[0x13];
		lvl->backgroundTexture->loadedPalette->colour[0x14] = lvl->tileTexture->loadedPalette->colour[0x14];
		lvl->backgroundTexture->loadedPalette->colour[0x1E] = lvl->tileTexture->loadedPalette->colour[0x1E];
		lvl->backgroundTexture->loadedPalette->colour[0x1F] = lvl->tileTexture->loadedPalette->colour[0x1F];
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

void EHZ_BackgroundScroll(bool updateScroll, uint16_t *array, int16_t *cameraX, int16_t *cameraY)
{
	uint16_t *arrValue = array;
	int line = 0;
	
	//Bit of sky at the top
	for (int i = 0; i < 22; i++)
	{
		*arrValue++ = 0;
		line++;
	}
	
	//Clouds + small island
	for (int i = 0; i < 58; i++)
	{
		*arrValue++ = *cameraX / 0x40;
		line++;
	}
	
	//Water at the horizon (change ripple every 8 frames)
	static int horWaterTimer = 4;
	static uint16_t horWaterRipple = 0;
	
	if (updateScroll)
	{
		if ((horWaterTimer++ & 0x7) == 0)
			--horWaterRipple;
	}
	
	for (int i = 0; i < 21; i++)
	{
		*arrValue++ = (*cameraX / 0x40) + ehzScrollRipple[(horWaterRipple & 0x1F) + i];
		line++;
	}
	
	//Water
	for (int i = 0; i < 11; i++)
	{
		*arrValue++ = 0;
		line++;
	}
	
	//Upper mountains
	for (int i = 0; i < 16; i++)
	{
		*arrValue++ = *cameraX / 0x10;
		line++;
	}
	
	//Lower mountains
	for (int i = 0; i < 16; i++)
	{
		*arrValue++ = (*cameraX * 0x18) / 0x100;
		line++;
	}
	
	//Field
	uint32_t delta = (((((*cameraX) / 2) - ((*cameraX) / 8)) * 0x100) / 0x30) * 0x100;
	uint32_t accumulate = ((*cameraX) / 8) * 0x10000;
	
	for (int i = line; i < gLevel->backgroundTexture->height;)
	{
		int mult = (i >= 196) ? 3 : (i >= 160 ? 2 : 1);
		for (int v = 0; v < mult; v++)
			*arrValue++ = accumulate / 0x10000;
		accumulate += delta * mult;
		i += mult;
	}
	
	//Clear camera offset
	*cameraX = 0;
	*cameraY = 0;
}
