#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../MathUtil.h"
#include "../Log.h"

enum SCRATCH
{
	//U8
	SCRATCHU8_WEIGHT =	0,
	//S16
	SCRATCHS16_ORIG_X =	0,
	SCRATCHS16_ORIG_Y =	1,
	SCRATCHS16_Y =		2,
};

void ObjGHZPlatform_Move(OBJECT *object)
{
	uint8_t type = object->subtype & 0xF;
	
	switch (type)
	{
		case 0x0:
			break;
		case 0x5: //Left and right
		case 0x1:
		{
			//Get our direction
			uint8_t angle = object->angle;
			if (type == 0x5)
				angle = -angle + 0x40;
			else
				angle -= 0x40;
			
			object->x.pos = object->scratchS16[SCRATCHS16_ORIG_X] + angle;
			object->angle = (gLevel->oscillate[6][0] >> 8);
			break;
		}
		case 0xC: //Up and down
		case 0xB:
		case 0x6:
		case 0x2:
		{
			uint8_t angle;
			if (type == 0xC)
				angle = -(gLevel->oscillate[3][0] >> 8) + 0x30;
			else if (type == 0xB)
				angle = (gLevel->oscillate[3][0] >> 8) - 0x30;
			else if (type == 0x6)
				angle = -object->angle + 0x40;
			else
				angle = object->angle - 0x40;
			
			object->scratchS16[SCRATCHS16_Y] = object->scratchS16[SCRATCHS16_ORIG_Y] + angle;
			object->angle = (gLevel->oscillate[6][0] >> 8);
			break;
		}
	}
}

void ObjGHZPlatform_Nudge(OBJECT *object)
{
	//Get the force
	int16_t sin;
	GetSine(object->scratchU8[SCRATCHU8_WEIGHT], &sin, NULL);
	object->y.pos = object->scratchS16[SCRATCHS16_Y] + ((sin * 0x400) >> 16);
}

void ObjGHZPlatform(OBJECT *object)
{	
	switch (object->routine)
	{
		case 0:
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/GHZPlatform.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 32;
			object->priority = 4;
			
			//Set our origin position and other stuff
			object->scratchS16[SCRATCHS16_ORIG_X] = object->x.pos;
			object->scratchS16[SCRATCHS16_ORIG_Y] = object->y.pos;
			object->scratchS16[SCRATCHS16_Y] = object->y.pos;
			object->angle = 0x80;
			
			//Set our frame and size for the big platform
			if (object->subtype == 10)
			{
				object->mappingFrame = 1;
				object->widthPixels = 64;
			}
	//Fallthrough
		case 1:
		{
			//Is a player standing on us?
			bool touching = false;
			
			for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
			{
				if (player->status.shouldNotFall && player->interact == (void*)object)
					touching = true;
			}
			
			//Decrease / increase our weight
			if (touching)
			{
				if (object->scratchU8[SCRATCHU8_WEIGHT] != 0x40)
					object->scratchU8[SCRATCHU8_WEIGHT] += 4;
			}
			else
			{
				if (object->scratchU8[SCRATCHU8_WEIGHT])
					object->scratchU8[SCRATCHU8_WEIGHT] -= 4;
			}
			
			//Handle all other routines
			int16_t lastX = object->x.pos;
			ObjGHZPlatform_Move(object);
			ObjGHZPlatform_Nudge(object);
			object->PlatformObject(object->widthPixels, 9, lastX);
			
			//Draw
			object->Draw();
		}
	}
}
