#include <string.h>

#include "SDL_rwops.h"
#include "Background.h"
#include "Game.h"
#include "Path.h"
#include "Error.h"

BACKGROUND::BACKGROUND(const char *name, BACKGROUNDFUNCTION backFunction)
{
	//Clear memory
	memset(this, 0, sizeof(BACKGROUND));
	
	//Load the given texture
	texture = new TEXTURE(nullptr, name);
	if (texture->fail)
	{
		fail = texture->fail;
		return;
	}
	
	//Set function to use
	function = backFunction;
}

BACKGROUND::~BACKGROUND()
{
	//Unload texture
	delete texture;
}

void BACKGROUND::DrawStrip(SDL_Rect *src, int layer, int16_t y, int16_t fromX, int16_t toX)
{
	if (toX == fromX)
	{
		//Just draw the strip in its entirety
		for (int x = -(-fromX % (unsigned)texture->width); x < gRenderSpec.width; x += texture->width)
			gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, src, layer, x, y, false, false);
	}
	else
	{
		//Draw a strip with shearing from fromX to toX
		SDL_Rect strip = {src->x, src->y, src->w, 1};
		for (int sy = 0; sy < src->h; sy++)
		{
			int16_t xp = fromX + ((toX - fromX) * sy / src->h);
			for (int x = -(-xp % (unsigned)texture->width); x < gRenderSpec.width; x += texture->width)
				gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &strip, layer, x, y + sy, false, false);
			strip.y++;
		}
	}
}

void BACKGROUND::Draw(bool doScroll, int16_t cameraX, int16_t cameraY)
{
	//Run our given function
	if (function != nullptr)
		function(this, doScroll, cameraX, cameraY);
}
