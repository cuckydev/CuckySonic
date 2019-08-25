#include <math.h>

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

//Title constants
enum TITLE_LAYERS
{
	TITLELAYER_BACKGROUND,
	TITLELAYER_EMBLEM,
	TITLELAYER_SONIC,
	TITLELAYER_SONIC_HAND,
	TITLELAYER_BANNER,
	TITLELAYER_MENU,
};

//Emblem and banner
const SDL_Rect titleEmblem = {0, 89, 256, 144};
const SDL_Rect titleBanner = {257, 106, 224, 74};
const int titleBannerJoin = 70;
const int titleBannerClipY = 10;

//Selection cursor
const SDL_Rect titleSelectionCursor[4] = {
	{257, 89, 8, 8},
	{266, 89, 8, 8},
	{275, 89, 8, 8},
	{266, 89, 8, 8},
};

//Sonic
const SDL_Rect titleSonicBody[4] = {
	{0,   0, 80, 72},
	{81,  0, 80, 88},
	{162, 0, 80, 80},
	{243, 0, 72, 80},
};

const struct
{
	SDL_Rect framerect;
	SDL_Point jointPos;
} titleSonicHand[3] = {
	{{316, 0, 40, 40}, {36, 36}},
	{{357, 0, 32, 48}, {18, 40}},
	{{390, 0, 40, 48}, {16, 41}},
};

const int sonicHandAnim[14] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1};

//Text render function
void DrawText(TEXTURE *tex, const char *text, int x, int y)
{
	const char *current;
	int dx = x;
	
	for (current = text; *current != 0; current++)
	{
		SDL_Rect thisCharRect = {((*current - 0x20) % 0x20) * 8, 234 + ((*current - 0x20) / 0x20) * 8, 8, 8};
		tex->Draw(TITLELAYER_MENU, tex->loadedPalette, &thisCharRect, dx, y, false, false);
		dx += 8;
	}
}

//Gamemode code
bool GM_Title(bool *noError)
{
	//Load our title sheet
	TEXTURE *titleTexture = new TEXTURE(NULL, "data/Title.bmp");
	if (titleTexture->fail != NULL)
		return (*noError = false);
	
	//Emblem and banner positions
	const int emblemX = (SCREEN_WIDTH - titleEmblem.w) / 2;
	const int emblemY = (SCREEN_HEIGHT - titleEmblem.h) / 2;
	const int bannerX = (SCREEN_WIDTH - titleBanner.w) / 2;
	const int bannerY = emblemY + titleBannerJoin;
	
	//Title state
	int titleYShift = 0;
	int frame = 0;
	
	//Sonic's animation and position
	int sonicTime = 54;
	
	int sonicX = ((SCREEN_WIDTH / 2) - 24) * 0x100;
	int sonicY = (bannerY + 16) * 0x100;
	
	int sonicXsp = -0x200;
	int sonicYsp = -0x4E0;
	
	int sonicFrame = 0;
	int sonicHandFrame = 0;
	int sonicAnimTimer = 0;
	
	//Background
	TEXTURE *backgroundTexture = new TEXTURE(NULL, "data/TitleBackground.bmp");
	if (backgroundTexture->fail != NULL)
		return (*noError = false);
	
	int backgroundX = 0;
	
	//Selection state
	bool selected = false;
	
	//Make our palette black for fade-in
	FillPaletteBlack(titleTexture->loadedPalette);
	FillPaletteBlack(backgroundTexture->loadedPalette);
	
	//Our loop
	bool noExit = true;
	
	while (noExit && *noError)
	{
		//Handle events
		if ((noExit = HandleEvents()) == false)
			break;
		
		//Fade in/out
		bool breakThisState = false;
		
		if (!selected)
		{
			PaletteFadeInFromBlack(titleTexture->loadedPalette);
			PaletteFadeInFromBlack(backgroundTexture->loadedPalette);
		}
		else
		{
			bool res1 = PaletteFadeOutToBlack(titleTexture->loadedPalette);
			bool res2 = PaletteFadeOutToBlack(backgroundTexture->loadedPalette);
			breakThisState = res1 && res2;
		}
		
		//Draw background
		uint16_t backgroundScroll[256];
		
		//Get our background's scroll
		uint16_t *arrValue = backgroundScroll;
		
		int scrollBG1 = backgroundX / 16;
		int scrollBG2 = backgroundX / 24;
		
		//Clouds
		for (int i = 0; i < 32; i++)
			*arrValue++ = scrollBG2;
		
		//Sky
		for (int i = 0; i < 64; i++)
			*arrValue++ = 0;
		
		//Mountains
		for (int i = 0; i < 64; i++)
			*arrValue++ = scrollBG1;
		
		//Water
		int32_t delta = (((backgroundX - scrollBG1) * 0x100) / 96) * 0x100;
		uint32_t accumulate = scrollBG1 * 0x10000;

		for (int i = 0; i < 96; i++)
		{
			*arrValue++ = accumulate / 0x10000;
			accumulate += delta;
		}
		
		//Draw each scanline
		for (int i = 0; i < backgroundTexture->height; i++)
		{
			for (int x = -(backgroundScroll[i] % backgroundTexture->width); x < SCREEN_WIDTH; x += backgroundTexture->width)
			{
				SDL_Rect backSrc = {0, i, backgroundTexture->width, 1};
				backgroundTexture->Draw(TITLELAYER_BACKGROUND, backgroundTexture->loadedPalette, &backSrc, x, i, false, false);
			}
		}
		
		//Render title screen banner and emblem
		titleTexture->Draw(TITLELAYER_EMBLEM, titleTexture->loadedPalette, &titleEmblem, emblemX, emblemY + titleYShift, false, false);
		titleTexture->Draw(TITLELAYER_BANNER, titleTexture->loadedPalette, &titleBanner, bannerX, bannerY + titleYShift, false, false);
		
		if (sonicTime-- <= 0)
		{
			//Clear timer (so there's no underflow)
			sonicTime = 0;
			
			//Move Sonic
			if ((sonicX += sonicXsp) > (SCREEN_WIDTH / 2) * 0x100)
				sonicX = (SCREEN_WIDTH / 2) * 0x100;
			else
				sonicXsp += 48;
				
			if ((sonicY += sonicYsp) < (bannerY - 70) * 0x100)
				sonicY = (bannerY - 70) * 0x100;
			else if ((sonicYsp += 0x24) > 0)
				sonicYsp = 0;
			
			//Animate Sonic
			if (sonicY < (bannerY - 32) * 0x100 && ++sonicAnimTimer >= 5)
			{
				//Reset timer and advance frame
				sonicAnimTimer = 0;
				if (sonicFrame < 3)
					sonicFrame++;
			}
			
			//Render Sonic
			SDL_Rect bodyRect = titleSonicBody[sonicFrame];
			
			const int midX = sonicX / 0x100;
			const int topY = sonicY / 0x100;
			const int bottomY = (sonicY / 0x100) + bodyRect.h;
			const int clipY = bannerY + titleBannerClipY;
			
			if (topY < clipY)
			{
				if (bottomY > clipY)
					bodyRect.h -= (bottomY - clipY);
				titleTexture->Draw(TITLELAYER_SONIC, titleTexture->loadedPalette, &bodyRect, midX - 40, topY + titleYShift, false, false);
			}
			
			//If animation is complete
			if (sonicFrame >= 3)
			{
				//Draw Sonic's hand
				int frame = sonicHandAnim[sonicHandFrame];
				titleTexture->Draw(TITLELAYER_SONIC_HAND, titleTexture->loadedPalette, &titleSonicHand[frame].framerect, midX + 20 - titleSonicHand[frame].jointPos.x, topY + 72 - titleSonicHand[frame].jointPos.y + titleYShift, false, false);
				
				//Update frame
				if (sonicHandFrame + 1 < 14)
					sonicHandFrame++;
				
				//Scroll background
				backgroundX += 8;
			}
		}
		
		//Handle selection and menus
		if (gController[0].press.a || gController[0].press.b || gController[0].press.c || gController[0].press.start)
			selected = true;
		
		//Render our software buffer to the screen
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&backgroundTexture->loadedPalette->colour[0])))
			return false;
		
		if (breakThisState)
			break;
		
		//Increment frame
		frame++;
	}
	
	//Unload our textures
	delete backgroundTexture;
	delete titleTexture;
	
	//Continue to game
	gGameLoadLevel = 0;
	gGameLoadCharacter = 0;
	gGameMode = GAMEMODE_GAME;
	return noExit;
}
