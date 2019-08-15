#include "Object.h"
#include "Game.h"
#include "Log.h"
#include "Error.h"

OBJECT::OBJECT(OBJECT **linkedList, void (*objectFunction)(OBJECT *object))
{
	memset(this, 0, sizeof(OBJECT));
	
	//Set type and subtype
	function = objectFunction;
	
	//Attach to linked list (if applicable)
	if (linkedList != NULL)
	{
		next = *linkedList;
		list = linkedList;
		*linkedList = this;
	}
}

OBJECT::~OBJECT()
{
	//Unload texture and mappings
	if (texture)
		delete texture;
	if (mappings)
		delete mappings;
	
	//Detach from linked list
	if (list != NULL)
	{
		for (OBJECT **object = list; *object != NULL; object = &(*object)->next)
		{
			if (*object == this)
			{
				*object = next;
				break;
			}
		}
	}
}

void OBJECT::Update()
{
	//Run our object code
	if (function != NULL)
		function(this);
}

void OBJECT::Draw()
{
	if (doRender)
	{
		//Don't draw if we don't have textures or mappings
		if (texture == NULL || mappings == NULL)
			return;
		
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
