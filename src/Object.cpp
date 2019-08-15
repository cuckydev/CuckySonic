#include "Object.h"
#include "Game.h"

OBJECT::OBJECT(int objectType, uint8_t objectSubtype)
{

}

OBJECT::~OBJECT()
{
	
}

void OBJECT::Update()
{
	
}

void OBJECT::Draw()
{
	if (doRender)
	{
		//Draw our sprite
		SDL_Rect *mapRect = &mappings->rect[mappingFrame];
		SDL_Point *mapOrig = &mappings->origin[mappingFrame];
		
		int origX = mapOrig->x;
		int origY = mapOrig->y;
		if (renderFlags.xFlip)
			origX = mapRect->w - origX;
		if (renderFlags.yFlip)
			origY = mapRect->h - origY;
		
		int alignX = renderFlags.alignPlane ? gLevel->camera->x : 0;
		int alignY = renderFlags.alignPlane ? gLevel->camera->y : 0;
		texture->Draw(texture->loadedPalette, mapRect, x.pos - origX - alignX, y.pos - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
	}
	else
	{
		doRender = true;
	}
}
