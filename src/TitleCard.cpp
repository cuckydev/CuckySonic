#include "Level.h"
#include "Game.h"
#include "TitleCard.h"
#include "MathUtil.h"
#include "Log.h"
#include "Error.h"

PALCOLOUR titleCardBackground;

//Class
TITLECARD::TITLECARD(const char *levelName, const char *levelSubtitle)
{
	memset(this, 0, sizeof(TITLECARD));
	
	//Load textures
	texture = gLevel->GetObjectTexture("data/TitleCard.bmp");
	if (texture == nullptr || texture->fail)
	{
		Error(fail = texture->fail);
		return;
	}
	
	fontTexture = gLevel->GetObjectTexture("data/GenericFont.bmp");
	if (fontTexture == nullptr || fontTexture->fail)
	{
		Error(fail = fontTexture->fail);
		return;
	}
	
	//Set zone and act
	name = levelName;
	subtitle = levelSubtitle;
	
	//Get position to draw at
	drawY = (gLevel->playerList->y.pos + (gLevel->playerList->yRadius - gLevel->playerList->defaultYRadius)) - gLevel->camera->y;
	
	if (drawY < gRenderSpec.height / 2)
		drawY += 48;
	else
		drawY -= 48;
	
	SetPaletteColour(&titleCardBackground, 0, 0, 0);
}

TITLECARD::~TITLECARD()
{
	//`texture` and `fontTexture` are freed in gLevel destructor
	return;
}

//Main update function
void TITLECARD::DrawText(const char *text, int x, int y)
{
	const char *current;
	int dx = x;
	
	for (current = text; *current != 0; current++)
	{
		SDL_Rect thisCharRect = {((*current - 0x20) % 0x20) * 16, ((*current - 0x20) / 0x20) * 16, 16, 16};
		gSoftwareBuffer->DrawTexture(fontTexture, fontTexture->loadedPalette, &thisCharRect, LEVEL_RENDERLAYER_TITLECARD, dx, y, false, false);
		dx += 16;
	}
}

#define END_TIME 160
#define TRANSITION_SPEED 16
#define TRANSITION_TIME (gRenderSpec.width / TRANSITION_SPEED)

void TITLECARD::UpdateAndDraw()
{
	const int subT = END_TIME / 2 - 20;
	const int finT = END_TIME / 2 + 20;
	
	//Quit if finished
	if (frame >= END_TIME)
		return;
	if (frame == finT)
		gLevel->inTitleCard = false;
	
	//Get the position of the level's name strip
	int namePosition = 0;
	
	if (frame < TRANSITION_TIME)
		namePosition = (TRANSITION_TIME - frame) * -TRANSITION_SPEED;
	if (frame >= END_TIME - TRANSITION_TIME)
		namePosition = (frame - END_TIME + TRANSITION_TIME) * TRANSITION_SPEED;
	
	//Get the position of the level's subtitle strip
	int subtitlePosition = 0;
	
	if (frame < TRANSITION_TIME)
		subtitlePosition = (TRANSITION_TIME * TRANSITION_SPEED) - (frame * TRANSITION_SPEED);
	if (frame >= END_TIME - TRANSITION_TIME)
		subtitlePosition = (frame - END_TIME + TRANSITION_TIME) * -TRANSITION_SPEED;
	
	//Draw level name
	DrawText(name, namePosition + 32, drawY - 12);
	
	//Draw level subtitle
	DrawText(subtitle, subtitlePosition + (gRenderSpec.width / 2 + 32), drawY + 12);
	
	//Draw the "CUCKYSONIC" label
	SDL_Rect labelSrc = {0, 0, 64, 8};
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &labelSrc, LEVEL_RENDERLAYER_TITLECARD, namePosition * -2, gRenderSpec.height - 8, false, false);
	
	//Draw the strip behind the name text
	for (int x = 0; x < gRenderSpec.width; x += 16)
	{
		SDL_Rect stripSrc = {0, 8, 16, 16};
		gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &stripSrc, LEVEL_RENDERLAYER_TITLECARD, x + namePosition, drawY - 12, false, false);
	}
	
	//Draw background (zoom out to show the player, then zoom completely out about a second later)
	int gap = 0;
	if (frame >= finT)
		gap = 32 + (frame - finT) * TRANSITION_SPEED;
	else if (frame >= subT)
		gap = min((frame - subT) * TRANSITION_SPEED / 2, 32);
	
	int16_t xg = gLevel->playerList->x.pos - gLevel->camera->x;
	int16_t yg = (gLevel->playerList->y.pos + (gLevel->playerList->yRadius - gLevel->playerList->defaultYRadius)) - gLevel->camera->y;
	
	SDL_Rect left = {0, 0, xg - gap, gRenderSpec.height};
	SDL_Rect top = {0, 0, gRenderSpec.width, yg - gap};
	SDL_Rect right = {xg + gap, 0, gRenderSpec.width - xg - gap, gRenderSpec.height};
	SDL_Rect bottom = {0, yg + gap, gRenderSpec.width, gRenderSpec.height - yg - gap};
	
	gSoftwareBuffer->DrawQuad(LEVEL_RENDERLAYER_TITLECARD, &left, &titleCardBackground);
	gSoftwareBuffer->DrawQuad(LEVEL_RENDERLAYER_TITLECARD, &right, &titleCardBackground);
	gSoftwareBuffer->DrawQuad(LEVEL_RENDERLAYER_TITLECARD, &top, &titleCardBackground);
	gSoftwareBuffer->DrawQuad(LEVEL_RENDERLAYER_TITLECARD, &bottom, &titleCardBackground);
	
	//Draw corners
	SDL_Rect cornerSrc = {16, 8, 32, 32};
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &cornerSrc, LEVEL_RENDERLAYER_TITLECARD, xg - gap, yg - gap, false, false);
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &cornerSrc, LEVEL_RENDERLAYER_TITLECARD, xg + gap - 32, yg - gap, true, false);
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &cornerSrc, LEVEL_RENDERLAYER_TITLECARD, xg - gap, yg + gap - 32, false, true);
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &cornerSrc, LEVEL_RENDERLAYER_TITLECARD, xg + gap - 32, yg + gap - 32, true, true);
	
	//Increment frame
	frame++;
}
