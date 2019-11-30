#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

static const uint8_t animationStand[] =	{0x0F,0x02,0xFF};
static const uint8_t animationWalk[] =	{0x07,0x00,0x01,0x00,0x02,0xFF};
static const uint8_t animationSmoke[] =	{0x01,0x03,0x06,0x04,0x06,0x04,0x06,0x04,0x06,0x05,0xFC};

static const uint8_t *animationList[] = {
	animationStand,
	animationWalk,
	animationSmoke,
};

void ObjMotobug(OBJECT *object)
{
	//Define and allocate our scratch
	struct SCRATCH
	{
		int8_t time = 0;
		int8_t smokeDelay = 0;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
	if (object->routine == 0)
	{
		//Load graphics
		object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
		object->mapping.mappings = gLevel->GetObjectMappings("data/Object/Motobug.map");
		
		//Initialize other properties
		object->routine++;
		object->renderFlags.alignPlane = true;
		object->widthPixels = 24;
		object->heightPixels = 32;
		object->priority = 4;
		
		if (object->anim == 0)
		{
			//If a motobug, setup our collision
			object->xRadius = 8;
			object->yRadius = 14;
			object->collisionType = COLLISIONTYPE_ENEMY;
			object->touchWidth = 20;
			object->touchHeight = 16;
		}
		else
			object->routine = 3; //Dust routine
	}
	
	switch (object->routine)
	{
		case 1: //Waiting to spawn
		{
			//Fall down to the ground
			object->MoveAndFall();
			
			//Check for the floor
			int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos, nullptr);
			if (distance >= 0)
				break;
			
			//Initialize state
			object->yVel = 0;
			object->routine++;
			object->y.pos += distance;
			object->status.xFlip ^= 1;
			break;
		}
		case 2: //Moving forwards
		{
			switch (object->routineSecondary)
			{
				case 0:
				{
					if (--scratch->time < 0)
					{
						//Set state and turn around
						object->routineSecondary = 1;
						object->status.xFlip ^= 1;
						object->anim = 1;
						
						if (object->status.xFlip)
							object->xVel = 0x100;
						else
							object->xVel = -0x100;
					}
					break;
				}
				case 1:
				{
					//Move and check if we're going over an edge
					object->Move();
					
					int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos, nullptr);
					if (distance < -8 || distance >= 12)
					{
						//Set state and wait
						object->routineSecondary = 0;
						scratch->time = 59;
						object->xVel = 0;
						object->anim = 0;
						break;
					}
					
					//Move across ground
					object->y.pos += distance;
					
					//Create smoke every 16 frames
					if (--scratch->smokeDelay < 0)
					{
						//Restart timer and create object
						scratch->smokeDelay = 15;
						
						OBJECT *newSmoke = new OBJECT(&ObjMotobug);
						newSmoke->x.pos = object->x.pos;
						newSmoke->y.pos = object->y.pos;
						newSmoke->status = object->status;
						newSmoke->anim = 2;
						gLevel->objectList.link_back(newSmoke);
					}
					break;
				}
			}
			
			//Animate and draw
			object->Animate_S1(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
		case 3: //Smoke
		{
			//Animate and draw
			object->Animate_S1(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 4: //Smoke deletion
		{
			//Delete us
			object->deleteFlag = true;
			break;
		}
		default:
			break;
	}
}
