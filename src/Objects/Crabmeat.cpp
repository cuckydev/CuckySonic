#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

static const uint8_t animationStand[] =			{0x0F,0x00,0xFF};
static const uint8_t animationStandSlope[] =	{0x0F,0x02,0xFF};
static const uint8_t animationStandSlopeRev[] =	{0x0F,0x22,0xFF};
static const uint8_t animationWalk[] =			{0x0F,0x01,0x21,0x00,0xFF};
static const uint8_t animationWalkSlope[] =		{0x0F,0x21,0x03,0x02,0xFF};
static const uint8_t animationWalkSlopeRev[] =	{0x0F,0x01,0x23,0x22,0xFF};
static const uint8_t animationFiring[] =		{0x0F,0x04,0xFF};
static const uint8_t animationProjectile[] =	{0x01,0x05,0x06,0xFF};

static const uint8_t *animationList[] = {
	animationStand,
	animationStandSlope,
	animationStandSlopeRev,
	animationWalk,
	animationWalkSlope,
	animationWalkSlopeRev,
	animationFiring,
	animationProjectile,
};

//Crabmeat projectiles
void ObjCrabmeatProjectile(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Crabmeat.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			
			object->collisionType = COLLISIONTYPE_HURT;
			object->touchWidth = 8;
			object->touchHeight = 8;
			object->hurtType.reflect = true;
			
			object->widthPixels = 8;
			object->yVel = -0x400;
			object->anim = 7;
		}
	//Fallthrough
		case 1:
		{
			//Move and fall, animate and draw
			object->Animate(animationList);
			object->MoveAndFall();
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			
			//Delete if fell off stage
			if (object->y.pos >= gLevel->bottomBoundaryTarget)
				object->deleteFlag = true;
			break;
		}
	}
}

//Crabmeat object
int ObjCrabmeat_SetAni(OBJECT *object)
{
	if (object->angle < 0x80)
	{
		if (object->angle >= 0x06)
			return (object->status.xFlip ? 1 : 2);
	}
	else
	{
		if (object->angle <= 0xFA)
			return (object->status.xFlip ? 2 : 1);
	}
	
	return 0;
}

void ObjCrabmeat(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//U8
		SCRATCHU8_MODE =	0, //bit 0 = collision type (flips every frame), bit 1 = fire or walk
		SCRATCHU8_MAX =		1,
		//S16
		SCRATCHS16_FIRE_TIME =	0,
		SCRATCHS16_MAX =		1,
	};
	
	object->ScratchAllocU8(SCRATCHU8_MAX);
	object->ScratchAllocS16(SCRATCHS16_MAX);
	
	switch (object->routine)
	{
		case 0:
		{
			//Set collision hitbox
			object->xRadius = 8;
			object->yRadius = 16;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Crabmeat.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			
			object->collisionType = COLLISIONTYPE_ENEMY;
			object->touchWidth = 16;
			object->touchHeight = 16;
			
			object->widthPixels = 21;
			object->heightPixels = 32;
			object->routine++;
		}
	//Fallthrough
		case 1: //Waiting to spawn
		{
			//Fall down to the ground
			object->MoveAndFall();
			
			//Check for the floor
			uint8_t nextAngle;
			int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos, &nextAngle);
			if (distance >= 0)
				break;
			
			//Initialize state
			object->y.pos += distance;
			object->angle = nextAngle;
			object->yVel = 0;
			object->routine++;
			break;
		}
		case 2: //Moving forwards
		{
			switch (object->routineSecondary)
			{
				case 0:
				{
					//Wait to fire projectiles
					if (--object->scratchS16[SCRATCHS16_FIRE_TIME] < 0)
					{
						//Decide if we should fire or just turn around
						if (object->renderFlags.isOnscreen == false || ((object->scratchU8[SCRATCHU8_MODE] ^= 0x02) & 0x02))
						{
							//Turn around
							object->routineSecondary = 1;
							object->scratchS16[SCRATCHS16_FIRE_TIME] = 127;
							object->xVel = 0x80;
							object->anim = ObjCrabmeat_SetAni(object) + 3;
							if (object->status.xFlip ^= 1)
								object->xVel = -object->xVel;
						}
						else
						{
							//Fire projectiles
							object->scratchS16[SCRATCHS16_FIRE_TIME] = 59;
							object->anim = 6;
							
							OBJECT *projLeft = new OBJECT(&ObjCrabmeatProjectile);
							projLeft->x.pos = object->x.pos - 16;
							projLeft->y.pos = object->y.pos;
							projLeft->xVel = -0x100;
							gLevel->objectList.link_back(projLeft);
							
							OBJECT *projRight = new OBJECT(&ObjCrabmeatProjectile);
							projRight->x.pos = object->x.pos + 16;
							projRight->y.pos = object->y.pos;
							projRight->xVel = 0x100;
							gLevel->objectList.link_back(projRight);
						}
					}
					break;
				}
				case 1:
				{
					//Walk about
					if (--object->scratchS16[SCRATCHS16_FIRE_TIME] >= 0)
					{
						//Move and check for the floor
						object->Move();
						
						if (((object->scratchU8[SCRATCHU8_MODE] ^= 0x01) & 0x01))
						{
							//Check for floor to the sides of us, stop and fire if there is none
							int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos + (object->status.xFlip ? -16 : 16), object->y.pos, nullptr);
							if (distance >= -8 && distance < 12)
								break;
						}
						else
						{
							//Collide with any floors below us
							int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos, &object->angle);
							object->y.pos += distance;
							object->anim = ObjCrabmeat_SetAni(object) + 3;
							break;
						}
					}
					
					//Set to fire
					object->routineSecondary = 0;
					object->scratchS16[SCRATCHS16_FIRE_TIME] = 59;
					object->xVel = 0;
					object->anim = ObjCrabmeat_SetAni(object);
					break;
				}
			}
			
			//Animate and draw
			object->Animate_S1(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
