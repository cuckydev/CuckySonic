#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../MathUtil.h"
#include "../Log.h"

enum SCRATCH
{
	//U8
	SCRATCHU8_WEIGHT =		0,
	SCRATCHU8_MAX =			1,
	//U16
	SCRATCHU16_FALL_TIME =	0,
	SCRATCHU16_MAX =		1,
	//S32
	SCRATCHS32_ORIG_X =		0,
	SCRATCHS32_ORIG_Y =		1,
	SCRATCHS32_Y =			2,
	SCRATCHS32_MAX =		3,
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
			int8_t angle = object->angle;
			if (type == 0x5)
				angle = -angle + 0x40;
			else
				angle -= 0x40;
			
			object->x.pos = (object->scratchS32[SCRATCHS32_ORIG_X] >> 16) + angle;
			object->angle = (gLevel->oscillate[6][0] >> 8);
			break;
		}
		case 0xC: //Up and down
		case 0xB:
		case 0x6:
		case 0x2:
		{
			int8_t angle;
			if (type == 0xC)
				angle = -(gLevel->oscillate[3][0] >> 8) + 0x30;
			else if (type == 0xB)
				angle = (gLevel->oscillate[3][0] >> 8) - 0x30;
			else if (type == 0x6)
				angle = -object->angle + 0x40;
			else
				angle = object->angle - 0x40;
			
			object->scratchS32[SCRATCHS32_Y] = (((object->scratchS32[SCRATCHS32_ORIG_Y] >> 16) + angle) << 16) | (object->scratchS32[SCRATCHS32_Y] & 0x0000FFFF);
			object->angle = (gLevel->oscillate[6][0] >> 8);
			break;
		}
		case 0x3: //Falling (stationary)
		{
			if (!object->scratchU16[SCRATCHU16_FALL_TIME])
			{
				//Wait for the player to stand on us
				for (size_t i = 0; i < gLevel->playerList.size(); i++)
				{
					PLAYER *player = gLevel->playerList[i];
					if (player->status.shouldNotFall && player->interact == (void*)object)
						object->scratchU16[SCRATCHU16_FALL_TIME] = 30; //Wait for 0.5 seconds
				}
			}
			else
			{
				//Wait for the timer to end
				if (--object->scratchU16[SCRATCHU16_FALL_TIME] == 0)
				{
					object->scratchU16[SCRATCHU16_FALL_TIME] = 32;
					object->subtype++;
				}
			}
			break;
		}
		case 0x4: //Falling
		{
			//Wait for our 30 second timer to run out
			if (object->scratchU16[SCRATCHU16_FALL_TIME] != 0 && --object->scratchU16[SCRATCHU16_FALL_TIME] == 0)
			{
				//Make players standing on us fall off
				for (size_t i = 0; i < gLevel->playerList.size(); i++)
				{
					//Get the player
					PLAYER *player = gLevel->playerList[i];
					
					if (player->status.shouldNotFall && player->interact == (void*)object)
					{
						//Make player airborne
						player->status.inAir = true;
						player->status.shouldNotFall = false;
						object->playerContact[i].standing = false;
						player->yVel = object->yVel;
					}
				}
				
				//No longer act as a solid platform
				object->routine = 2;
			}
			
			//Fall
			object->scratchS32[SCRATCHS32_Y] += object->yVel * 0x100;
			object->yVel += 0x38;
			
			//Delete if reached bottom boundary
			if ((object->scratchS32[SCRATCHS32_Y] >> 16) >= gLevel->bottomBoundaryTarget)
				object->deleteFlag = true;
			break;
		}
		case 0xA: //Big platform
		{
			//Move up and down
			object->scratchS32[SCRATCHS32_Y] = (((object->scratchS32[SCRATCHS32_ORIG_Y] >> 16) + (object->angle - 0x40) / 2) << 16) | (object->scratchS32[SCRATCHS32_Y] & 0x0000FFFF);
			object->angle = (gLevel->oscillate[6][0] >> 8);
			break;
		}
	}
	
	//Set our y position according to our position and the weight of a player above us
	object->y.pos = (object->scratchS32[SCRATCHS32_Y] >> 16) + ((GetSin(object->scratchU8[SCRATCHU8_WEIGHT]) * 0x400) >> 16);
}

void ObjGHZPlatform(OBJECT *object)
{
	//Allocate scratch memory
	object->ScratchAllocU8(SCRATCHU8_MAX);
	object->ScratchAllocU16(SCRATCHU16_MAX);
	object->ScratchAllocS32(SCRATCHS32_MAX);
	
	switch (object->routine)
	{
		case 0:
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZPlatform.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 32;
			object->heightPixels = 32;
			object->priority = 4;
			
			//Set our origin position and other stuff
			object->scratchS32[SCRATCHS32_ORIG_X] = object->x.pos << 16;
			object->scratchS32[SCRATCHS32_ORIG_Y] = object->y.pos << 16;
			object->scratchS32[SCRATCHS32_Y] = object->y.pos << 16;
			object->angle = 0x80;
			
			//Reset other values
			object->scratchU8[SCRATCHU8_WEIGHT] = 0;
			object->scratchU16[SCRATCHU16_FALL_TIME] = 0;
			object->yVel = 0;
			
			//Set our frame for the big platform
			if (object->subtype == 10)
				object->mappingFrame = 1;
	//Fallthrough
		case 1:
		{
			//Is a player standing on us?
			bool touching = false;
			for (int i = 0; i < OBJECT_PLAYER_REFERENCES; i++)
				if (object->playerContact[i].standing)
					touching = true;
			
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
			if (object->routine == 1)
				object->PlatformObject(object->widthPixels, 9, lastX, false, nullptr);
			
			//Draw
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->scratchS32[SCRATCHS32_ORIG_X] >> 16);
			break;
		}
		case 2:
		{
			//Handle routines and draw
			ObjGHZPlatform_Move(object);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->scratchS32[SCRATCHS32_ORIG_X] >> 16);
			break;
		}
	}
}
