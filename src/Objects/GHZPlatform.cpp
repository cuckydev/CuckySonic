#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../MathUtil.h"
#include "../Log.h"

enum SCRATCH
{
	//U8
	SCRATCHU8_WEIGHT =		0,
	SCRATCHU8_SPAWN_TYPE =	1,
	//U16
	SCRATCHU16_FALL_TIME =	0,
	//S16
	SCRATCHS16_SPAWN_X =	0,
	SCRATCHS16_SPAWN_Y =	1,
	//S32
	SCRATCHS32_ORIG_X =		0,
	SCRATCHS32_ORIG_Y =		1,
	SCRATCHS32_Y =			2,
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
				for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
					if (player->status.shouldNotFall && player->interact == (void*)object)
						object->scratchU16[SCRATCHU16_FALL_TIME] = 30; //Wait for 0.5 seconds
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
				bool touching = false;
				
				int i = 0;
				for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
				{
					if (player->status.shouldNotFall && player->interact == (void*)object)
					{
						//Make player airborne
						player->status.inAir = true;
						player->status.shouldNotFall = false;
						object->playerContact[i].standing = false;
						player->yVel = object->yVel;
					}
					
					i++;
				}
				
				//No longer act as a solid platform
				object->routine = 2;
			}
			
			//Fall
			object->scratchS32[SCRATCHS32_Y] += object->yVel * 0x100;
			object->yVel += 0x38;
			
			//Delete if reached bottom boundary
			if ((object->scratchS32[SCRATCHS32_Y] >> 16) >= gLevel->bottomBoundaryTarget)
				object->routine = 3;
			break;
		}
	}
}

void ObjGHZPlatform_Nudge(OBJECT *object)
{
	//Get the force
	int16_t sin;
	GetSine(object->scratchU8[SCRATCHU8_WEIGHT], &sin, NULL);
	object->y.pos = (object->scratchS32[SCRATCHS32_Y] >> 16) + ((sin * 0x400) >> 16);
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
			object->scratchS32[SCRATCHS32_ORIG_X] = object->x.pos << 16;
			object->scratchS32[SCRATCHS32_ORIG_Y] = object->y.pos << 16;
			object->scratchS32[SCRATCHS32_Y] = object->y.pos << 16;
			object->angle = 0x80;
			
			//Reset other values
			object->scratchU8[SCRATCHU8_WEIGHT] = 0;
			object->scratchU16[SCRATCHU16_FALL_TIME] = 0;
			object->yVel = 0;
			
			//Remember our spawning position and subtype
			object->scratchS16[SCRATCHS16_SPAWN_X] = object->x.pos;
			object->scratchS16[SCRATCHS16_SPAWN_Y] = object->y.pos;
			object->scratchU8[SCRATCHU8_SPAWN_TYPE] = object->subtype;
			
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
				if (player->status.shouldNotFall && player->interact == (void*)object)
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
			ObjGHZPlatform_Nudge(object);
			object->PlatformObject(object->widthPixels, 9, lastX);
			
			//Draw
			object->Draw();
			break;
		}
		case 2:
		{
			//Handle routines and draw
			ObjGHZPlatform_Move(object);
			ObjGHZPlatform_Nudge(object);
			object->Draw();
			break;
		}
		case 3:
		{
			//Respawn when left screen horizontally (emulates how original game does stuff)
			int16_t xDiff = object->x.pos - gLevel->camera->x;
			if (xDiff <= -object->widthPixels || xDiff >= SCREEN_WIDTH + object->widthPixels)
			{
				object->xPosLong = object->scratchS16[SCRATCHS16_SPAWN_X] << 16;
				object->yPosLong = object->scratchS16[SCRATCHS16_SPAWN_Y] << 16;
				object->subtype = object->scratchU8[SCRATCHU8_SPAWN_TYPE];
				object->routine = 0;
			}
			break;
		}
	}
}
