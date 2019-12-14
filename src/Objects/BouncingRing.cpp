#include <stdint.h>
#include "../Endianness.h"
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"
#include "../Objects.h"
#include "../Audio.h"
#include "../MathUtil.h"

//#define BOUNCINGRING_BLINK              //When set, the rings will blink shortly before despawning

//#define BOUNCINGRING_ONLY_FLOOR	        //When set, like in the originals, bouncing rings will only check for floor collision
//#define BOUNCINGRING_ORIGINAL_COLLISION //When set, the game will filter the collision so only a quarter of the rings will check for collision in a frame

static const uint8_t animationCollect[] =	{0x05,0x04,0x05,0x06,0x07,ANICOMMAND_ADVANCE_ROUTINE};

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
		#ifdef ENDIAN_BIG
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
			//Set our velocity
			xVel = GetSin(asInd.angle) * (1 << asInd.speed);
			yVel = GetCos(asInd.angle) * (1 << asInd.speed);
			
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
	//Define and allocate our scratch
	struct SCRATCH
	{
		uint8_t animCount = 255;
		uint16_t animAccum = 0;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
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
			object->texture = gLevel->GetObjectTexture("data/Object/Generic.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/Ring.map");
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			
			//Initialize collision with Sonic and other properties
			object->collisionType = COLLISIONTYPE_OTHER;
			object->touchWidth = 6;
			object->touchHeight = 6;
			
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
			
		#ifdef BOUNCINGRING_ORIGINAL_COLLISION
			if ((gLevel->frameCounter + gLevel->objectList.pos_of_val(object)) % 4 == 0)
		#endif
			{
				//Check for collision with the floor or ceiling
				int16_t checkVel = object->yVel;
				if (object->parentPlayer != nullptr && object->parentPlayer->status.reverseGravity)
					checkVel = -checkVel;
				
				if (checkVel >= 0)
				{
					int16_t distance = GetCollisionV(object->x.pos, object->y.pos + object->yRadius, COLLISIONLAYER_NORMAL_TOP, false, nullptr);
					
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
					int16_t distance = GetCollisionV(object->x.pos, object->y.pos - object->yRadius, COLLISIONLAYER_NORMAL_LRB, true, nullptr);
					
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
					int16_t distance = GetCollisionH(object->x.pos + object->xRadius, object->y.pos, COLLISIONLAYER_NORMAL_LRB, false, nullptr);
					
					//If touching a wall, bounce off
					if (distance < 0)
					{
						object->x.pos += distance;
						object->xVel = object->xVel / -2;
					}
				}
				else if (object->xVel < 0)
				{
					int16_t distance = GetCollisionH(object->x.pos - object->xRadius, object->y.pos, COLLISIONLAYER_NORMAL_LRB, true, nullptr);
					
					//If touching a wall, bounce off
					if (distance < 0)
					{
						object->x.pos -= distance;
						object->xVel = object->xVel / -2;
					}
				}
			#endif
			}
			
			//Animate
			if (scratch->animCount != 0)
			{
				scratch->animAccum += scratch->animCount--;
				object->mappingFrame = (scratch->animAccum >> 9) & 0x3;
			}
			
			//Check for deletion
			if (scratch->animCount == 0)
				object->deleteFlag = true;
			else
			
		#ifdef BOUNCINGRING_BLINK
			if (scratch->animCount > 60 || gLevel->frameCounter & (scratch->animCount > 30 ? 0x4 : 0x2))
		#endif
				object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
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
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 4: //Deleting after sparkle
		{
			object->deleteFlag = true;
			break;
		}
	}
}
