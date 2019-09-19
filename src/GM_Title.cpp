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
#include "BackgroundScroll.h"

//Title constants
enum TITLE_LAYERS
{
	TITLELAYER_MENU,
	TITLELAYER_BANNER,
	TITLELAYER_SONIC_HAND,
	TITLELAYER_SONIC,
	TITLELAYER_EMBLEM,
	TITLELAYER_BACKGROUND,
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

const int sonicHandAnim[14] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1};

//Text render function
void DrawText(TEXTURE *tex, const char *text, int x, int y)
{
	int dx = x;
	for (const char *current = text; *current != 0; current++)
	{
		SDL_Rect thisCharRect = {((*current - 0x20) % 0x20) * 8, 234 + ((*current - 0x20) / 0x20) * 8, 8, 8};
		gSoftwareBuffer->DrawTexture(tex, tex->loadedPalette, &thisCharRect, TITLELAYER_MENU, dx, y, false, false);
		dx += 8;
	}
}

//Gamemode code
bool GM_Title(bool *noError)
{
	//Load our title sheet
	TEXTURE *titleTexture = new TEXTURE(NULL, "data/Title.bmp");
	if (titleTexture->fail != NULL)
		return (*noError = !Error(titleTexture->fail));
	
	//Emblem and banner positions
	const int emblemX = (gRenderSpec.width - titleEmblem.w) / 2;
	const int emblemY = (gRenderSpec.height - titleEmblem.h) / 2;
	
	const int bannerX = (gRenderSpec.width - titleBanner.w) / 2;
	const int bannerY = emblemY + titleBannerJoin;
	
	//Title state
	int titleYShift = gRenderSpec.height * 0x100;
	int titleYSpeed = -0x107E;
	int titleYGoal = 0;
	int frame = 0;
	
	//Sonic's animation and position
	int sonicTime = 54;
	
	int sonicX = (gRenderSpec.width / 2) * 0x100;
	int sonicY = (bannerY + 16) * 0x100;
	
	int sonicXsp = -0x400;
	int sonicYsp = -0x400;
	
	int sonicFrame = 0;
	int sonicHandFrame = 0;
	int sonicAnimTimer = 0;
	
	//Background
	TEXTURE *backgroundTexture = new TEXTURE(NULL, "data/TitleBackground.bmp");
	if (backgroundTexture->fail != NULL)
		return (*noError = !Error(backgroundTexture->fail));
	
	BACKGROUNDSCROLL *backgroundScroll = new BACKGROUNDSCROLL("data/Title.bsc", backgroundTexture);
	if (backgroundScroll->fail != NULL)
		return (*noError = !Error(backgroundScroll->fail));
	
	int backgroundX = 0;
	
	//Selection state
	bool selected = false;
	
	//Make our palette black for fade-in
	FillPaletteBlack(titleTexture->loadedPalette);
	FillPaletteBlack(backgroundTexture->loadedPalette);
	
	//Handle music
	const MUSICSPEC titleSpec = {"Title", 0, 1.0f};
	const MUSICSPEC menuSpec = {"Menu", 0, 1.0f};
	PlayMusic(titleSpec);
	
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
			//Fade asset sheet palette in
			PaletteFadeInFromBlack(titleTexture->loadedPalette);
			PaletteFadeInFromBlack(backgroundTexture->loadedPalette);
		}
		else
		{
			//Fade asset sheet palette out
			bool res1 = PaletteFadeOutToBlack(titleTexture->loadedPalette);
			bool res2 = PaletteFadeOutToBlack(backgroundTexture->loadedPalette);
			breakThisState = res1 && res2;
			
			//Fade music out
			SetMusicVolume(max(GetMusicVolume() - (1.0f / 24.0f), 0.0f));
		}
		
		//Draw background
		backgroundScroll->GetScroll(backgroundX, 0, NULL, NULL);
		
		//Draw each line
		SDL_Rect backSrc = {0, 0, backgroundTexture->width, 1};
		for (int i = 0; i < min(gRenderSpec.height, backgroundTexture->height); i++)
		{
			for (int x = -(backgroundScroll->scrollArray[i] % backgroundTexture->width); x < gRenderSpec.width; x += backgroundTexture->width)
				gSoftwareBuffer->DrawTexture(backgroundTexture, backgroundTexture->loadedPalette, &backSrc, LEVEL_RENDERLAYER_BACKGROUND, x, i, false, false);
			backSrc.y++;
		}
		
		//Move title screen at beginning
		if (titleYShift >= titleYGoal && titleYSpeed >= 0)
		{
			titleYSpeed /= -2;
			titleYShift = titleYGoal;
		}
		else
		{
			titleYSpeed += 0x80;
			titleYShift += titleYSpeed;
		}
		
		//Render title screen banner and emblem
		gSoftwareBuffer->DrawTexture(titleTexture, titleTexture->loadedPalette, &titleEmblem, TITLELAYER_EMBLEM, emblemX, emblemY + titleYShift / 0x100, false, false);
		gSoftwareBuffer->DrawTexture(titleTexture, titleTexture->loadedPalette, &titleBanner, TITLELAYER_BANNER, bannerX, bannerY + titleYShift / 0x100, false, false);
		
		if (sonicTime-- <= 0)
		{
			//Clear timer (so there's no underflow)
			sonicTime = 0;
			
			//Move Sonic
			if ((sonicX += sonicXsp) > (gRenderSpec.width / 2) * 0x100)
				sonicX = (gRenderSpec.width / 2) * 0x100;
			else
				sonicXsp += 54;
				
			if ((sonicY += sonicYsp) < (bannerY - 70) * 0x100)
				sonicY = (bannerY - 70) * 0x100;
			else if ((sonicYsp += 24) > 0)
				sonicYsp = 0;
			
			//Animate Sonic
			if (sonicY < (bannerY - 40) * 0x100 && ++sonicAnimTimer >= 5)
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
				gSoftwareBuffer->DrawTexture(titleTexture, titleTexture->loadedPalette, &bodyRect, TITLELAYER_SONIC, midX - 40, topY + titleYShift / 0x100, false, false);
			}
			
			//If animation is complete
			if (sonicFrame >= 3)
			{
				//Draw Sonic's hand
				int frame = sonicHandAnim[sonicHandFrame];
				gSoftwareBuffer->DrawTexture(titleTexture, titleTexture->loadedPalette, &titleSonicHand[frame].framerect, TITLELAYER_SONIC_HAND, midX + 20 - titleSonicHand[frame].jointPos.x, topY + 72 - titleSonicHand[frame].jointPos.y + titleYShift / 0x100, false, false);
				
				//Update frame
				if (sonicHandFrame + 1 < 14)
					sonicHandFrame++;
				
				//Scroll background
				backgroundX += sonicHandFrame / 2 + 1;
			}
		}
		
		//Handle selection and menus
		if (gController[0].press.a || gController[0].press.b || gController[0].press.c || gController[0].press.start)
			selected = true;
		
		//Render our software buffer to the screen
		if (!(*noError = gSoftwareBuffer->RenderToScreen(&backgroundTexture->loadedPalette->colour[0])))
			break;
		
		if (breakThisState)
			break;
		
		//Increment frame
		frame++;
		
		//Switch to menu music after a few seconds
		if (selected == false && frame == (17 * 60))
			PlayMusic(menuSpec);
	}
	
	//Unload our textures
	delete backgroundScroll;
	delete backgroundTexture;
	delete titleTexture;
	
	//Continue to game
	gGameLoadLevel = 0;
	gGameLoadCharacter = 0;
	gGameMode = GAMEMODE_GAME;
	return noExit;
}
