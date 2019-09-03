#include "../Level.h"
#include "../Game.h"

void GHZ_PaletteCycle(LEVEL *lvl)
{
	(void)lvl;
}

const uint32_t ghzCloudScroll[] = {0x10000, 0xC000, 0x8000};

void GHZ_BackgroundScroll(bool updateScroll, uint16_t *array, int16_t *cameraX, int16_t *cameraY)
{
	uint16_t *arrValue = array;
	int line = 0;
	
	//Get our scroll values
	int16_t scrollBG2 = (*cameraX) / 2;
	int16_t scrollBG3 = (*cameraX) * 3 / 8;
	
	//Scroll clouds
	static uint16_t cloudScroll[3] = {0, 0, 0};
	
	if (updateScroll)
	{
		(cloudScroll[0] += 0x10) %= (gLevel->backgroundTexture->width * 0x10);
		(cloudScroll[1] += 0x0C) %= (gLevel->backgroundTexture->width * 0x10);
		(cloudScroll[2] += 0x08) %= (gLevel->backgroundTexture->width * 0x10);
	}
	
	//Cloud layers
	for (int i = 0; i < 0x20; i++)
	{
		*arrValue++ = (cloudScroll[0] / 0x10) + scrollBG3;
		line++;
	}
	
	for (int i = 0; i < 0x10; i++)
	{
		*arrValue++ = (cloudScroll[1] / 0x10) + scrollBG3;
		line++;
	}
	
	for (int i = 0; i < 0x10; i++)
	{
		*arrValue++ = (cloudScroll[2] / 0x10) + scrollBG3;
		line++;
	}
	
	//Upper hills
	for (int i = 0; i < 0x30; i++)
	{
		*arrValue++ = scrollBG3;
		line++;
	}
	
	//Lower hills
	for (int i = 0; i < 0x28; i++)
	{
		*arrValue++ = scrollBG2;
		line++;
	}
	
	//Water
	int32_t delta = ((((*cameraX) - scrollBG2) * 0x100) / 0x68) * 0x100;
	uint32_t accumulate = scrollBG2 << 16;
	
	for (int i = line; i < gLevel->backgroundTexture->height; i++)
	{
		*arrValue++ = accumulate >> 16;
		accumulate += delta;
		line++;
	}
	
	//Set camera offset
	*cameraX = 0;
	*cameraY = (*cameraY) / -0x20 + 0x20;
}
