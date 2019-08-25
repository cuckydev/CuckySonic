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
	enum SCRATCH
	{
		SCRATCH_TIME = 0,			//1 byte
		SCRATCH_SMOKE_DELAY = 1,	//1 byte
	};
	
	switch (object->routine)
	{
		case 0:
		{
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
			object->mappings = new MAPPINGS("data/Object/Motobug.map");
			
			//Initialize other properties
			object->routine++;
			object->renderFlags.alignPlane = true;
			object->widthPixels = 0x18;
			object->priority = 4;
			
			//Collision
			object->xRadius = 8;
			object->yRadius = 14;
			object->collisionType = 0x0C;
		}
		//Fallthrough
		case 1: //Waiting to spawn
		{
			//Detect the floor (if not inside the ground, fall)
			object->MoveAndFall();
			
			int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos, NULL);
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
				case 0: //@move (nice wrong name)
				{
					if (--object->scratchS8[SCRATCH_TIME] < 0)
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
				case 1: //@findfloor (nice half wrong name)
				{
					//Move and check if we're going over an edge
					object->Move();
					
					int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos, NULL);
					if (distance < -8 || distance >= 12)
					{
						//Set state and wait
						object->routineSecondary = 0;
						object->scratchS8[SCRATCH_TIME] = 59;
						object->xVel = 0;
						object->anim = 0;
						break;
					}
					
					//Move across ground
					object->y.pos += distance;
					
					//Create smoke
					break;
				}
			}
			
			object->Animate(animationList);
			break;
		}
	}
}
