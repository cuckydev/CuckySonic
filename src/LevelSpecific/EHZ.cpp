#include "../Level.h"
#include "../Game.h"

static const uint8_t ehzWaterfallCycle[4][4][3] = {
	{{0x66, 0x88, 0xAA}, {0x66, 0x88, 0xEE}, {0x88, 0xAA, 0xEE}, {0xAA, 0xCC, 0xEE}},
	{{0xAA, 0xCC, 0xEE}, {0x66, 0x88, 0xAA}, {0x66, 0x88, 0xEE}, {0x88, 0xAA, 0xEE}},
	{{0x88, 0xAA, 0xEE}, {0xAA, 0xCC, 0xEE}, {0x66, 0x88, 0xAA}, {0x66, 0x88, 0xEE}},
	{{0x66, 0x88, 0xEE}, {0x88, 0xAA, 0xEE}, {0xAA, 0xCC, 0xEE}, {0x66, 0x88, 0xAA}},
};

void EHZ_PaletteCycle(LEVEL *lvl)
{
	//Palette cycle 0 (waterfall)
	if (++lvl->palCycle[0].timer >= 8)
	{
		//Set colours
		SetPaletteColour(&lvl->tileTexture->loadedPalette->colour[0x13], ehzWaterfallCycle[lvl->palCycle[0].cycle][0][0], ehzWaterfallCycle[lvl->palCycle[0].cycle][0][1], ehzWaterfallCycle[lvl->palCycle[0].cycle][0][2]);
		SetPaletteColour(&lvl->tileTexture->loadedPalette->colour[0x14], ehzWaterfallCycle[lvl->palCycle[0].cycle][1][0], ehzWaterfallCycle[lvl->palCycle[0].cycle][1][1], ehzWaterfallCycle[lvl->palCycle[0].cycle][1][2]);
		SetPaletteColour(&lvl->tileTexture->loadedPalette->colour[0x1E], ehzWaterfallCycle[lvl->palCycle[0].cycle][2][0], ehzWaterfallCycle[lvl->palCycle[0].cycle][2][1], ehzWaterfallCycle[lvl->palCycle[0].cycle][2][2]);
		SetPaletteColour(&lvl->tileTexture->loadedPalette->colour[0x1F], ehzWaterfallCycle[lvl->palCycle[0].cycle][3][0], ehzWaterfallCycle[lvl->palCycle[0].cycle][3][1], ehzWaterfallCycle[lvl->palCycle[0].cycle][3][2]);
		
		//Increment cycle
		lvl->palCycle[0].timer = 0;
		if (++lvl->palCycle[0].cycle >= 4)
			lvl->palCycle[0].cycle = 0;
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
