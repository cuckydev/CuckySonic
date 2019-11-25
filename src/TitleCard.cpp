#include "Level.h"
#include "Game.h"
#include "TitleCard.h"
#include "MathUtil.h"
#include "Log.h"
#include "Error.h"

//Timing constants
#define TT_SHOWEND	110 //When the title card leaves the screen
#define TT_UNLOCK	125 //When the stage unlocks
#define TT_END		240 //When the title card unloads

//Class
TITLECARD::TITLECARD(std::string levelName, std::string levelSubtitle) : name(levelName), subtitle(levelSubtitle)
{
	//Load title card sheet
	texture = gLevel->GetObjectTexture("data/TitleCard.bmp");
	
	//Load font texture and font mappings
	TEXTURE *fontTexture = gLevel->GetObjectTexture("data/GenericFont.bmp");
	nameFont = new BITMAPFONT(fontTexture, 0, 0, 16, 16, 0, 0, 0x20, 0x20);
	subtitleFont = new BITMAPFONT(fontTexture, 0, 83, 8, 11, 0, 0, 0x20, 0x20);
	
	//Get our focus position
	focusX = gLevel->playerList[0]->x.pos - gLevel->camera->xPos;
	focusY = gLevel->playerList[0]->y.pos - gLevel->camera->yPos;
	
	//Initialize lines
	line[LINE_CUCKYSONIC_LABEL] = {(-64) * 0x100, (8) * 0x100, 0x400, 0x0, -0x20, 0x0, 0x90, 0x7FFF, -0x8000, 0x7FFF};
	line[LINE_LEVEL_NAME] = {((int)levelName.length() * -16 + (gRenderSpec.width - 398) / 2) * 0x100, (128) * 0x100, 0x780 + ((int)levelName.length() * 0x10), 0x0, -0x22, 0x0, 0x180, 0x7FFF, -0x8000, 0x7FFF};
	line[LINE_LEVEL_SUBTITLE] = {(gRenderSpec.width) * 0x100, line[LINE_LEVEL_NAME].y + (24) * 0x100, -0x500, 0x0, 0x20, 0x0, -0x8000, -0x100, -0x8000, 0x7FFF};
}

TITLECARD::~TITLECARD()
{
	//Free font mappings
	delete nameFont;
	delete subtitleFont;
}

//Ribbon drawer
void TITLECARD::DrawRibbon(const RECT *rect, int x, int y, int width)
{
	//Draw left, middle, and right
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &rect[0], LEVEL_RENDERLAYER_TITLECARD, x, y, false, false); x += rect[0].w;
	for (int i = 0; i < width; i++)
		{ gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &rect[1], LEVEL_RENDERLAYER_TITLECARD, x, y, false, false); x += rect[1].w; }
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &rect[2], LEVEL_RENDERLAYER_TITLECARD, x, y, false, false);
}

//General titlecard function
void TITLECARD::UpdateAndDraw()
{
	//Don't update or draw if ended
	if (frame >= TT_END)
		return;
	
	//Update lines
	for (size_t i = 0; i < LINE_MAX; i++)
	{
		line[i].x += line[i].xsp; line[i].y += line[i].ysp;
		line[i].xsp += line[i].xAcc; line[i].ysp += line[i].yAcc;
		line[i].xsp = mmin(mmax(line[i].xsp, line[i].xMin), line[i].xMax); line[i].ysp = mmin(mmax(line[i].ysp, line[i].yMin), line[i].yMax);
	}
	
	//Draw CuckySonic label
	const RECT cuckyLabel = {0, 0, 96, 16};
	const RECT cuckyRibbon[3] = {
		{0, 34, 16, 24},
		{17, 34, 16, 24},
		{34, 34, 16, 24},
	};
	
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &cuckyLabel, LEVEL_RENDERLAYER_TITLECARD, line[LINE_CUCKYSONIC_LABEL].x / 0x100 + 16, line[LINE_CUCKYSONIC_LABEL].y / 0x100 + 4, false, false);
	DrawRibbon(cuckyRibbon, line[LINE_CUCKYSONIC_LABEL].x / 0x100, line[LINE_CUCKYSONIC_LABEL].y / 0x100, 96 / 16);
	
	//Draw level name
	const RECT stageDisplay[] = {
		{0, 60, 192, 128}, //ZONEID_GHZ
		{0, 0, 0, 0}, //ZONEID_EHZ
	};
	
	const RECT nameRibbon[3] = {
		{0, 17, 16, 16},
		{17, 17, 16, 16},
		{34, 17, 16, 16},
	};
	
	nameFont->DrawString(name, LEVEL_RENDERLAYER_TITLECARD, line[LINE_LEVEL_NAME].x / 0x100 + 8, line[LINE_LEVEL_NAME].y / 0x100 - 8);
	DrawRibbon(nameRibbon, line[LINE_LEVEL_NAME].x / 0x100, line[LINE_LEVEL_NAME].y / 0x100, mmax((int)name.length() - 1, 192 / 16));
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &stageDisplay[gLevel->zone], LEVEL_RENDERLAYER_TITLECARD, line[LINE_LEVEL_NAME].x / 0x100, line[LINE_LEVEL_NAME].y / 0x100 - 128 + 8, false, false);
	
	//Draw level subtitle
	const RECT subtitleRibbon[3] = {
		{51, 51, 8, 8},
		{60, 51, 8, 8},
		{69, 51, 8, 8},
	};
	
	subtitleFont->DrawString(subtitle, LEVEL_RENDERLAYER_TITLECARD, line[LINE_LEVEL_SUBTITLE].x / 0x100 + 4, line[LINE_LEVEL_SUBTITLE].y / 0x100 - 5);
	DrawRibbon(subtitleRibbon, line[LINE_LEVEL_SUBTITLE].x / 0x100, line[LINE_LEVEL_SUBTITLE].y / 0x100, subtitle.length() - 1);
	
	//Speed lines up when ending
	if (frame == TT_SHOWEND)
	{
		for (size_t i = 0; i < LINE_MAX; i++)
			line[i].xAcc = line[i].xAcc * -7;
	}
	
	//Get our background position
	int backX = -focusX;
	int backY = -focusY;
	int openRadius = 0;
	
	if (frame < TT_SHOWEND)
	{
		//Scroll to target position
		backX += (TT_SHOWEND - frame);
		backY += (TT_SHOWEND - frame);
	}
	else
	{
		//Open up around target position
		openRadius += (frame - TT_SHOWEND);
	}
	
	//Draw background
	const RECT backRc[] = {
		{51, 17, 16, 16}, //Body
		{68, 17, 16, 16}, //TL
		{85, 17, 16, 16}, //TR
		{68, 34, 16, 16}, //BL
		{85, 34, 16, 16}, //BR
		{0, 0, 0, 0}, //Open
	};
	
	for (int x = -(backX % 16u); x < gRenderSpec.width; x += 16)
	{
		for (int y = -(backY % 16u); y < gRenderSpec.height; y += 16)
		{
			//Get how to draw the hole per tile
			const RECT *rc = &backRc[0];
			
			if (openRadius)
			{
				//Get our tile position (offset from focusX and focusY)
				int tx = (x - focusX) / 16;
				int ty = (y - focusY) / 16;
				if (tx >= 0)
					tx++;
				if (ty >= 0)
					ty++;
				
				//Open around focusX and focusY
				int rad = (openRadius - mabs(ty));
				if (mabs(tx) > rad)
					rc = &backRc[0];
				else if (mabs(tx) >= rad)
					rc = &backRc[1 + (((ty >= 0) << 1) | (tx >= 0))];
				else
					rc = &backRc[5];
			}
			
			//Draw tile
			gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, rc, LEVEL_RENDERLAYER_TITLECARD, x, y, false, false);
		}
	}

	//Increment frame and check for unlock
	if (++frame >= TT_UNLOCK)
		activeLock = false;
	return;
}
