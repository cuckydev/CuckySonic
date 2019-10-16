#include "../Level.h"
#include "../Game.h"

void GHZ_PaletteCycle()
{
	//Palette cycle 0 (waterfall and background)
	if (--gLevel->palCycle[0].timer < 0)
	{
		//Cycle colours and reset timer
		gLevel->palCycle[0].timer = 5;
		
		//Cycle the tile texture (using the background palette, because it won't change until after)
		gLevel->tileTexture->loadedPalette->colour[0x28] = gLevel->background->texture->loadedPalette->colour[0x2B];
		gLevel->tileTexture->loadedPalette->colour[0x29] = gLevel->background->texture->loadedPalette->colour[0x28];
		gLevel->tileTexture->loadedPalette->colour[0x2A] = gLevel->background->texture->loadedPalette->colour[0x29];
		gLevel->tileTexture->loadedPalette->colour[0x2B] = gLevel->background->texture->loadedPalette->colour[0x2A];
		
		//Copy the background's palette from the tile texture
		gLevel->background->texture->loadedPalette->colour[0x28] = gLevel->tileTexture->loadedPalette->colour[0x28];
		gLevel->background->texture->loadedPalette->colour[0x29] = gLevel->tileTexture->loadedPalette->colour[0x29];
		gLevel->background->texture->loadedPalette->colour[0x2A] = gLevel->tileTexture->loadedPalette->colour[0x2A];
		gLevel->background->texture->loadedPalette->colour[0x2B] = gLevel->tileTexture->loadedPalette->colour[0x2B];
	}
}

void GHZ_Background(BACKGROUND *background, bool doScroll, int cameraX, int cameraY)
{
	//Get our scroll values
	int16_t scrollBG1 = cameraX / 2;
	int16_t scrollBG2 = cameraX * 3 / 8;
	
	int16_t backY = -(cameraY / -0x20 + 0x20);
	
	//Scroll clouds
	static uint32_t cloudScroll[3] = {0, 0, 0};
	if (doScroll)
	{
		(cloudScroll[0] += 0x10) %= (background->texture->width * 0x10);
		(cloudScroll[1] += 0x0C) %= (background->texture->width * 0x10);
		(cloudScroll[2] += 0x08) %= (background->texture->width * 0x10);
	}
	
	//Draw clouds
	RECT cloud1 = {0,   0, background->texture->width,  32};
	RECT cloud2 = {0,  32, background->texture->width,  16};
	RECT cloud3 = {0,  48, background->texture->width,  16};
	background->DrawStrip(&cloud1, LEVEL_RENDERLAYER_BACKGROUND, backY +   0, -((cloudScroll[0] / 0x10) + scrollBG2), -((cloudScroll[0] / 0x10) + scrollBG2));
	background->DrawStrip(&cloud2, LEVEL_RENDERLAYER_BACKGROUND, backY +  32, -((cloudScroll[1] / 0x10) + scrollBG2), -((cloudScroll[1] / 0x10) + scrollBG2));
	background->DrawStrip(&cloud3, LEVEL_RENDERLAYER_BACKGROUND, backY +  48, -((cloudScroll[2] / 0x10) + scrollBG2), -((cloudScroll[2] / 0x10) + scrollBG2));
	
	//Draw mountains
	RECT upperMountain = {0,  64, background->texture->width,  48};
	RECT lowerMountain = {0, 112, background->texture->width,  40};
	background->DrawStrip(&upperMountain, LEVEL_RENDERLAYER_BACKGROUND, backY +  64, -scrollBG2, -scrollBG2);
	background->DrawStrip(&lowerMountain, LEVEL_RENDERLAYER_BACKGROUND, backY + 112, -scrollBG1, -scrollBG1);
	
	//Draw water
	RECT water = {0, 152, background->texture->width,  background->texture->height - 152};
	background->DrawStrip(&water, LEVEL_RENDERLAYER_BACKGROUND, backY +  152, -scrollBG1, -cameraX);
}
