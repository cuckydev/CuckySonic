#include <stdint.h>
#include "../Endianness.h"
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"
#include "../Objects.h"
#include "../Audio.h"
#include "../MathUtil.h"

//#define BOUNCINGRING_BLINK		//When set, the rings will blink shortly before despawning
//#define BOUNCINGRING_ONLY_FLOOR	//When set, like in the originals, bouncing rings will only check for floor collision

static const uint8_t animationCollect[] =	{0x05,0x04,0x05,0x06,0x07,0xFC};

static const uint8_t *animationList[] = {
	animationCollect,
};

void ObjBouncingRing_Spawner(OBJECT *object)
{
	//Cap our rings
	unsigned int *rings = &gRings;
	if (*rings >= 32)
		*rings = 32;
	
	//Spawn the given amount of rings
	union
	{
		#ifdef CPU_BIGENDIAN
			struct
			{
				uint8_t speed;
				uint8_t angle;
			} asInd;
		#else
			struct
			{
				uint8_t angle;
				uint8_t speed;
			} asInd;
		#endif
		
		int16_t angleSpeed = 0x288;
	};
	
	int16_t xVel = 0, yVel = 0;
	for (unsigned int i = 0; i < *rings; i++)
	{
		//Create the ring object
		OBJECT *ring = new OBJECT(&ObjBouncingRing);
		ring->parentPlayer = object->parentPlayer;
		ring->x.pos = object->x.pos;
		ring->y.pos = object->y.pos;
		gLevel->objectList.link_back(ring);
		
		//Get the ring's velocity
		if (angleSpeed >= 0)
		{
			//Get this velocity
			int16_t sin, cos;
			GetSine(asInd.angle, &sin, &cos);
			
			xVel = sin * (1 << asInd.speed);
			yVel = cos * (1 << asInd.speed);
			
			//Get the next angle and speed
			asInd.angle += 0x10;
			
			if (asInd.angle < 0x10)
			{
				angleSpeed -= 0x80;
				if (angleSpeed < 0)
					angleSpeed = 0x288;
			}
		}
		
		//Set the ring's velocity
		ring->xVel = xVel;
		ring->yVel = yVel;
		
		xVel = -xVel;
		angleSpeed = -angleSpeed;
	}
	
	//Clear our rings and delete us
	PlaySound(SOUNDID_RING_LOSS);
	*rings = 0;
	object->deleteFlag = true;
}

void ObjBouncingRing(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//U8
		SCRATCHU8_ANIM_COUNT = 0,
		SCRATCHU8_MAX = 1,
		//U16
		SCRATCHU16_ANIM_ACCUM = 0,
		SCRATCHU16_MAX = 1,
	};
	
	object->ScratchAllocU8(SCRATCHU8_MAX);
	object->ScratchAllocU16(SCRATCHU16_MAX);
	
	switch (object->routine)
	{
		case 0:
		{
			//Initialize collision
			object->xRadius = 8;
			object->yRadius = 8;
			
			//Initialize render properties and load graphics
			object->widthPixels = 8;
			object->heightPixels = 8;
			object->texture = gLevel->GetObjectTexture("data/Object/Ring.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Ring.map");
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			
			//Initialize collision with Sonic and other properties
			object->collisionType = COLLISIONTYPE_OTHER;
			object->touchWidth = 6;
			object->touchHeight = 6;
			
			object->scratchU8[SCRATCHU8_ANIM_COUNT] = 0xFF;
			object->scratchU16[SCRATCHU16_ANIM_ACCUM] = 0x00;
			
			object->routine++;
		}
	//Fallthrough
		case 1:
		{
			//Move and fall
			object->xPosLong += object->xVel * 0x100;
			if (object->parentPlayer != nullptr && object->parentPlayer->status.reverseGravity)
				object->yPosLong -= object->yVel * 0x100;
			else
				object->yPosLong += object->yVel * 0x100;
			object->yVel += 0x18;
			
			//Check for collision with the floor or ceiling
			int16_t checkVel = object->yVel;
			if (object->parentPlayer != nullptr && object->parentPlayer->status.reverseGravity)
				checkVel = -checkVel;
			
			if (checkVel >= 0)
			{
				int16_t distance = FindFloor(object->x.pos, object->y.pos + object->yRadius, COLLISIONLAYER_NORMAL_TOP, false, nullptr);
				
				//If touching the floor, bounce off
				if (distance < 0)
				{
					object->y.pos += distance;
					object->yVel = object->yVel * 3 / -4;
				}
			}
		#ifndef BOUNCINGRING_ONLY_FLOOR
			else
			{
				int16_t distance = FindFloor(object->x.pos, object->y.pos - object->yRadius, COLLISIONLAYER_NORMAL_LRB, true, nullptr);
				
				//If touching a ceiling, bounce off
				if (distance < 0)
				{
					object->y.pos -= distance;
					object->yVel = -object->yVel;
				}
			}
			
			//Check for collision with walls
			if (object->xVel > 0)
			{
				int16_t distance = FindWall(object->x.pos + object->xRadius, object->y.pos, COLLISIONLAYER_NORMAL_LRB, false, nullptr);
				
				//If touching a wall, bounce off
				if (distance < 0)
				{
					object->x.pos += distance;
					object->xVel = object->xVel / -2;
				}
			}
			else if (object->xVel < 0)
			{
				int16_t distance = FindWall(object->x.pos - object->xRadius, object->y.pos, COLLISIONLAYER_NORMAL_LRB, true, nullptr);
				
				//If touching a wall, bounce off
				if (distance < 0)
				{
					object->x.pos -= distance;
					object->xVel = object->xVel / -2;
				}
			}
		#endif
			
			//Animate
			if (object->scratchU8[SCRATCHU8_ANIM_COUNT] != 0)
			{
				object->scratchU16[SCRATCHU16_ANIM_ACCUM] += object->scratchU8[SCRATCHU8_ANIM_COUNT]--;
				object->mappingFrame = (object->scratchU16[SCRATCHU16_ANIM_ACCUM] >> 9) & 0x3;
			}
			
			//Check for deletion
			if (object->scratchU8[SCRATCHU8_ANIM_COUNT] == 0)
				object->deleteFlag = true;
			else
			
		#ifdef BOUNCINGRING_BLINK
			if (object->scratchU8[SCRATCHU8_ANIM_COUNT] > 60 || gLevel->frameCounter & (object->scratchU8[SCRATCHU8_ANIM_COUNT] > 30 ? 0x4 : 0x2))
		#endif
				object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 2: //Touched player, collect a ring
		{
			//Increment routine
			object->routine++;
			
			//Clear collision and change priority
			object->collisionType = COLLISIONTYPE_NULL;
			object->priority = 1;
			
			//Collect the ring
			AddToRings(1);
		}
	//Fallthrough
		case 3: //Sparkling
		{
			object->Animate(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 4: //Deleting after sparkle
		{
			object->deleteFlag = true;
			break;
		}
	}
}
