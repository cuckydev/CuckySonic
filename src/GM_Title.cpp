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
#include "Background.h"

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
const RECT titleEmblem = {0, 89, 256, 144};
const RECT titleBanner = {257, 106, 224, 74};
const int titleBannerJoin = 70;
const int titleBannerClipY = 10;

//Selection cursor
const RECT titleSelectionCursor[4] = {
	{257, 89, 8, 8},
	{266, 89, 8, 8},
	{275, 89, 8, 8},
	{266, 89, 8, 8},
};

//Sonic
const RECT titleSonicBody[4] = {
	{0,   0, 80, 72},
	{81,  0, 80, 88},
	{162, 0, 80, 80},
	{243, 0, 72, 80},
};

const struct
{
	RECT framerect;
	POINT jointPos;
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
		RECT thisCharRect = {((*current - 0x20) % 0x20) * 8, 234 + ((*current - 0x20) / 0x20) * 8, 8, 8};
		gSoftwareBuffer->DrawTexture(tex, tex->loadedPalette, &thisCharRect, TITLELAYER_MENU, dx, y, false, false);
		dx += 8;
	}
}

//Background function
static const uint8_t scrollRipple[64] =
{
	1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0,
	2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
	1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0,
	2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
};

void TitleBackground(BACKGROUND *background, bool doScroll, int cameraX, int cameraY)
{
	(void)doScroll;
	(void)cameraY;
	
	//Handle palette cycle
	static int paletteTimer = 0;
	
	if (++paletteTimer >= 8)
	{
		paletteTimer = 0;
		
		COLOUR c9 = background->texture->loadedPalette->colour[0x9];
		COLOUR cA = background->texture->loadedPalette->colour[0xA];
		COLOUR cB = background->texture->loadedPalette->colour[0xB];
		COLOUR cC = background->texture->loadedPalette->colour[0xC];
		background->texture->loadedPalette->colour[0x9] = COLOUR(cC);
		background->texture->loadedPalette->colour[0xA] = COLOUR(c9);
		background->texture->loadedPalette->colour[0xB] = COLOUR(cA);
		background->texture->loadedPalette->colour[0xC] = COLOUR(cB);
	}
	//Get our scroll values
	int scrollBG1 = cameraX / 24;
	int scrollBG2 = cameraX / 32;
	int scrollBG3 = cameraX / 2;
	
	//Draw clouds
	static unsigned int cloudScroll = 0;
	(cloudScroll += 0x6) %= (background->texture->width * 0x10);
	
	RECT clouds = {0,  0, background->texture->width,  32};
	background->DrawStrip(&clouds, TITLELAYER_BACKGROUND,   0, -(scrollBG1 + cloudScroll / 0x10), -(scrollBG1 + cloudScroll / 0x10));
	
	//Draw sky and mountains
	RECT mountains = {0,  32, background->texture->width,  128};
	background->DrawStrip(&mountains, TITLELAYER_BACKGROUND,  32, -scrollBG2, -scrollBG2);
	
	//Draw ocean
	static unsigned int rippleFrame = 0, rippleTimer = 4;
	if (++rippleTimer >= 8)
	{
		rippleTimer = 0;
		rippleFrame++;
	}
	
	RECT strip = {0, 160, background->texture->width, 1};
	for (int i = 160; i < background->texture->height; i++)
	{
		int x = scrollBG2 + (scrollBG3 - scrollBG2) * (i - 160) / (background->texture->height - 160);
		x += scrollRipple[(i + rippleFrame) % 64] * (i - 160) / ((background->texture->height - 160) / 2);
		background->DrawStrip(&strip, TITLELAYER_BACKGROUND, strip.y++, -x, -x);
	}
	
	//Clear screen with sky behind background
	RECT backQuad = {0, 0, gRenderSpec.width, gRenderSpec.height};
	gSoftwareBuffer->DrawQuad(TITLELAYER_BACKGROUND, &backQuad, &background->texture->loadedPalette->colour[0]);
}

//Gamemode code
bool GM_Title(bool *bError)
{
	//Load our title sheet and background
	TEXTURE titleTexture("data/Title.bmp");
	if (titleTexture.fail != nullptr)
		return (*bError = !Error(titleTexture.fail));
	
	BACKGROUND background("data/TitleBackground.bmp", &TitleBackground);
	if (background.fail != nullptr)
		return (*bError = !Error(background.fail));
	
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
	
	int backgroundScroll = 0, backgroundScrollSpeed = 0;
	
	//Sonic's animation and position
	int sonicTime = 54;
	
	int sonicX = (gRenderSpec.width / 2) * 0x100;
	int sonicY = (bannerY + 16) * 0x100;
	
	int sonicXsp = -0x400;
	int sonicYsp = -0x400;
	
	int sonicFrame = 0;
	int sonicHandFrame = 0;
	int sonicAnimTimer = 0;
	
	//Selection state
	bool selected = false;
	
	//Make our palette black for fade-in
	FillPaletteBlack(titleTexture.loadedPalette);
	FillPaletteBlack(background.texture->loadedPalette);
	
	//Our loop
	bool bExit = false;
	
	while (!(bExit || *bError))
	{
		//Handle events
		bExit = HandleEvents();
		
		//Fade in/out
		bool breakThisState = false;
		
		if (!selected)
		{
			//Fade asset sheet and background palette in
			PaletteFadeInFromBlack(titleTexture.loadedPalette);
			PaletteFadeInFromBlack(background.texture->loadedPalette);
		}
		else
		{
			//Fade asset sheet and background palette out
			bool res1 = PaletteFadeOutToBlack(titleTexture.loadedPalette);
			bool res2 = PaletteFadeOutToBlack(background.texture->loadedPalette);
			breakThisState = res1 && res2;
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
		
		//Render background
		background.Draw(true, backgroundScroll, 0);
		
		//Render title screen banner and emblem
		gSoftwareBuffer->DrawTexture(&titleTexture, titleTexture.loadedPalette, &titleEmblem, TITLELAYER_EMBLEM, emblemX, emblemY + titleYShift / 0x100, false, false);
		gSoftwareBuffer->DrawTexture(&titleTexture, titleTexture.loadedPalette, &titleBanner, TITLELAYER_BANNER, bannerX, bannerY + titleYShift / 0x100, false, false);
		
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
			RECT bodyRect = titleSonicBody[sonicFrame];
			
			const int midX = sonicX / 0x100;
			const int topY = sonicY / 0x100;
			const int bottomY = (sonicY / 0x100) + bodyRect.h;
			const int clipY = bannerY + titleBannerClipY;
			
			if (topY < clipY)
			{
				if (bottomY > clipY)
					bodyRect.h -= (bottomY - clipY);
				gSoftwareBuffer->DrawTexture(&titleTexture, titleTexture.loadedPalette, &bodyRect, TITLELAYER_SONIC, midX - 40, topY + titleYShift / 0x100, false, false);
			}
			
			//If animation is complete
			if (sonicFrame >= 3)
			{
				//Draw Sonic's hand
				int frame = sonicHandAnim[sonicHandFrame];
				gSoftwareBuffer->DrawTexture(&titleTexture, titleTexture.loadedPalette, &titleSonicHand[frame].framerect, TITLELAYER_SONIC_HAND, midX + 20 - titleSonicHand[frame].jointPos.x, topY + 72 - titleSonicHand[frame].jointPos.y + titleYShift / 0x100, false, false);
				
				//Update frame
				if (sonicHandFrame + 1 < 14)
				{
					sonicHandFrame++;
					backgroundScrollSpeed++;
				}
				
				//Scroll background
				(backgroundScroll += backgroundScrollSpeed) %= background.texture->width * 96;
			}
		}
		
		//Handle selection and menus
		if (gController[0].press.a || gController[0].press.b || gController[0].press.c || gController[0].press.start)
			selected = true;
		
		//Render our software buffer to the screen
		if ((*bError = gSoftwareBuffer->RenderToScreen(nullptr)) == true)
			break;
		
		if (breakThisState)
			break;
		
		//Increment frame
		frame++;
	}
	
	//Continue to game
	gGameLoadLevel = 0;
	gGameLoadCharacter = 0;
	gGameMode = GAMEMODE_GAME;
	return bExit;
}
