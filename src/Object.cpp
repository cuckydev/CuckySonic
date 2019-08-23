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
		list = linkedList;
		
		//If linked list is unset, set us as the first 
		if (*linkedList == NULL)
		{
			*linkedList = this;
			return;
		}
		
		//Attach us to the linked list
		for (OBJECT *object = *linkedList; 1; object = object->next)
		{
			if (object->next == NULL)
			{
				object->next = this;
				break;
			}
		}
	}
}

OBJECT::~OBJECT()
{
	//Unload texture and mappings
	if (texture && texture->list == NULL)
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
	
	//Destroy children
	for (OBJECT *object = children; object != NULL;)
	{
		OBJECT *next = object->next;
		delete object;
		object = next;
	}
}

//Shared animate function
void OBJECT::Animate(const uint8_t **animationList)
{
	//Restart animation if not the same as the previous
	if (anim != prevAnim)
	{
		prevAnim = anim;
		animFrame = 0;
		animFrameDuration = 0;
	}
	
	//Wait for next frame
	if (--animFrameDuration >= 0)
		return;
	
	//Get the animation to reference
	const uint8_t *animation = animationList[anim];
	
	//Set next frame duration
	animFrameDuration = animation[0];
	
	//Handle commands
	if (animation[1 + animFrame] >= 0x80)
	{
		switch (animation[1 + animFrame])
		{
			case 0xFF:	//Restart animation
				animFrame = 0;
				break;
			case 0xFE:	//Go back X amount of frames
				animFrame -= animation[2 + animFrame];
				break;
			case 0xFD:	//Switch to X animation
				anim = animation[2 + animFrame];
				return;
			case 0xFC:	//Advance routine
				routine++;
				animFrameDuration = 0;
				animFrame++; //???
				return;
			case 0xFB:	//Reset animation and clear secondary routine
				animFrame = 0;
				routineSecondary = 0;
				return;
			case 0xFA:	//Increment secondary routine
				routineSecondary++;
				return;
			case 0xF9:	//Not sure
				return;
		}
	}
	
	//Set our mapping frame and flip
	mappingFrame = animation[1 + animFrame] & 0x7F;
	renderFlags.xFlip = status.xFlip;
	renderFlags.yFlip = status.yFlip;
	animFrame++;
}

//Update and drawing objects
void OBJECT::Update()
{
	//Run our object code
	if (function != NULL)
		function(this);
	
	//Update children's code
	for (OBJECT *object = children; object != NULL; object = object->next)
		object->Update();
}

void OBJECT::Draw()
{
	if (doRender)
	{
		//Don't draw if we don't have textures or mappings
		if (texture != NULL && mappings != NULL)
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
			texture->Draw((highPriority ? LEVEL_RENDERLAYER_OBJECT_HIGH_0 : LEVEL_RENDERLAYER_OBJECT_0) + ((OBJECT_LAYERS - 1) - priority), texture->loadedPalette, mapRect, x.pos - origX - alignX, y.pos - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
		}
	}
	else
	{
		doRender = true;
	}
	
	//Draw children
	for (OBJECT *object = children; object != NULL; object = object->next)
		object->Draw();
}
