#include <string.h>

#include "Hud.h"
#include "Level.h"
#include "Game.h"
#include "Error.h"

#define SONICCD_LONG_TIME	//In Sonic CD, the time includes milliseconds (divided by 10)

//Positioning constants
#define SCORE_Y	8
#define SCORE_LEFT	16
#define SCORE_RIGHT	120

#define TIME_Y	24
#define TIME_LEFT	16
#define TIME_RIGHT	120 //96 for short time, 120 for long time

#define RINGS_Y	40
#define RINGS_LEFT	16
#define RINGS_RIGHT	96

#define LIVES_Y	(gRenderSpec.height - 24)
#define LIVES_LEFT 16
#define LIVES_NUM_LEFT	44

//Constructor and deconstructor
HUD::HUD()
{
	//Load HUD texture
	texture = gLevel->GetObjectTexture("data/HUD.bmp");
	if (texture->fail != nullptr)
	{
		Error(fail = texture->fail);
		return;
	}
	
	//Load font
	TEXTURE *fontTexture = gLevel->GetObjectTexture("data/GenericFont.bmp");
	font = new BITMAPFONT(fontTexture, 0, 49, 8, 11, 0, 0, 0x20, 0x20);
}

HUD::~HUD()
{
	//Free our loaded font
	delete font;
}

//Drawing functions
void HUD::DrawLabel(int xPos, int yPos, int srcX, int srcY)
{
	RECT src = {srcX * 48, srcY * 16, 48, 16};
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &src, LEVEL_RENDERLAYER_HUD, xPos, yPos, false, false);
}

//Core draw function
#define PAD_NUMBER_STRING(padString, len) (std::string(len - padString.length(), '0').append(padString))

void HUD::Draw()
{
	//Blink the time and ring labels
	bool scoreAlt = false;
	bool timeAlt = false;
	bool ringAlt = false;
	
	if (gLevel->frameCounter & 0x8)
	{
		if (gTime >= (60 * 60 * 9))
			timeAlt = true;
		if (gRings == 0)
			ringAlt = true;
	}
	
	//Draw score, time, rings, and lives labels
	DrawLabel(SCORE_LEFT,	SCORE_Y,	0, scoreAlt ?	1 : 0);
	DrawLabel(TIME_LEFT,	TIME_Y,		1, timeAlt ?	1 : 0);
	DrawLabel(RINGS_LEFT,	RINGS_Y,	2, ringAlt ?	1 : 0);
	DrawLabel(LIVES_LEFT,	LIVES_Y,	3, 0);
	
	//Draw score value
	std::string score = std::to_string(gScore);
	font->DrawString(score, LEVEL_RENDERLAYER_HUD, SCORE_RIGHT - (8 * score.length()), SCORE_Y);
	
	//Draw time value
	std::string mins = std::to_string((gTime / 60) / 60);
	std::string secs = std::to_string((gTime / 60) % 60);
	
	#ifdef SONICCD_LONG_TIME
		std::string mils = std::to_string((gTime * 100 / 60) % 100);
		std::string time = mins + "'" + PAD_NUMBER_STRING(secs, 2) + "\"" + PAD_NUMBER_STRING(mils, 2); //M'ss"mm
	#else
		std::string time = mins + ":" + PAD_NUMBER_STRING(secs, 2); //M:ss
	#endif
	
	font->DrawString(time, LEVEL_RENDERLAYER_HUD, TIME_RIGHT - (8 * time.length()), TIME_Y);
	
	//Draw rings value
	std::string rings = std::to_string(gRings);
	font->DrawString(rings, LEVEL_RENDERLAYER_HUD, RINGS_RIGHT - (8 * rings.length()), RINGS_Y);
	
	//Draw lives value
	font->DrawString(std::to_string(gLives), LEVEL_RENDERLAYER_HUD, LIVES_NUM_LEFT, LIVES_Y + 2);
}
