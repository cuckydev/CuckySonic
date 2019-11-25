#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Objects.h"
#include "../MathUtil.h"

//Buzz Bomber animations
static const uint8_t animationFly1[] =		{0x01,0x00,0x01,0xFF};
static const uint8_t animationFly2[] =		{0x01,0x02,0x03,0xFF};
static const uint8_t animationFiring[] =	{0x01,0x04,0x05,0xFF};

static const uint8_t *animationList[] = {
	animationFly1,
	animationFly2,
	animationFiring,
};

//Missile animations
static const uint8_t animationCharge[] =	{0x07,0x00,0x01,0xFC};
static const uint8_t animationFire[] =		{0x01,0x02,0x03,0xFF};

static const uint8_t *missileAnimationList[] = {
	animationCharge,
	animationFire,
};

//Buzz Bomber Missile
void ObjBuzzBomberMissile(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//S16
		SCRATCHS16_FIRE_DELAY =	0,
		SCRATCHS16_MAX =		1,
	};
	
	object->ScratchAllocS16(SCRATCHS16_MAX);
	
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/Missile.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			
			object->widthPixels = 8;
			object->heightPixels = 32;
			
			object->scratchS16[SCRATCHS16_FIRE_DELAY] = 14;
		}
	//Fallthrough
		case 1: //Charging fire
		{
			//Check if the buzz bomber has been destroyed
			if (object->parentObject == nullptr || object->parentObject->deleteFlag == true || object->parentObject->function == &ObjExplosion)
			{
				object->deleteFlag = true;
				break;
			}
			
			//Wait until fire delay is over
			if (--object->scratchS16[SCRATCHS16_FIRE_DELAY] >= 0)
				break;
			
			//Draw and animate
			object->Animate(missileAnimationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 2: //Missile hitbox initialization
		{
			//Set our collision and animation
			object->collisionType = COLLISIONTYPE_HURT;
			object->touchWidth = 6;
			object->touchHeight = 6;
			object->hurtType.reflect = true;
			object->anim = 1;
			object->routine++;
		}
	//Fallthrough
		case 3: //Missile
		{
			//Move, draw, and animate
			if (object->collisionType == COLLISIONTYPE_HURT)
				object->Move();
			else
				object->MoveAndFall();
			object->Animate(missileAnimationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			
			//Delete if below stage
			if (object->y.pos >= gLevel->bottomBoundaryTarget)
				object->deleteFlag = true;
			break;
		}
	}
}

//Buzz Bomber object
void ObjBuzzBomber(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//U8
		SCRATCHU8_STATUS =	0,
		SCRATCHU8_MAX =		1,
		//S16
		SCRATCHS16_TIME_DELAY =	0,
		SCRATCHS16_MAX =		1,
	};
	
	object->ScratchAllocU8(SCRATCHU8_MAX);
	object->ScratchAllocS16(SCRATCHS16_MAX);
	
	#define STATUS_FIRED		0x01
	#define STATUS_NEARPLAYER	0x02
	
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/BuzzBomber.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			
			object->collisionType = COLLISIONTYPE_ENEMY;
			object->touchWidth = 24;
			object->touchHeight = 12;
			
			object->widthPixels = 24;
			object->heightPixels = 32;
		}
	//Fallthrough
		case 1:
		{
			switch (object->routineSecondary)
			{
				case 0:
				{
					if (--object->scratchS16[SCRATCHS16_TIME_DELAY] < 0)
					{
						if (!(object->scratchU8[SCRATCHU8_STATUS] & STATUS_NEARPLAYER))
						{
							//If not near a player, fly for a bit longer
							object->routineSecondary = 1;
							object->scratchS16[SCRATCHS16_TIME_DELAY] = 127;
							object->anim = 1;
							if (object->status.xFlip)
								object->xVel = 0x400;
							else
								object->xVel = -0x400;
						}
						else
						{
							//If near a player, fire in our facing direction
							OBJECT *projectile = new OBJECT(&ObjBuzzBomberMissile);
							projectile->xVel = 0x200;
							projectile->yVel = 0x200;
							
							int16_t xOff = 24;
							if (!object->status.xFlip)
							{
								//Invert if facing left
								xOff = -xOff;
								projectile->xVel = -projectile->xVel;
							}
							
							projectile->x.pos = object->x.pos + xOff;
							projectile->y.pos = object->y.pos + 28;
							projectile->status = object->status;
							projectile->parentObject = object;
							gLevel->objectList.link_back(projectile);
							
							//Update our state
							object->scratchU8[SCRATCHU8_STATUS] = STATUS_FIRED;
							object->scratchS16[SCRATCHS16_TIME_DELAY] = 59;
							object->anim = 2;
						}
					}
					break;
				}
				case 1:
				{
					//Move for the given amount of time and turn around, or prematurely stop if near a player and fire a projectile
					if (--object->scratchS16[SCRATCHS16_TIME_DELAY] >= 0)
					{
						//Move and check if we're near a player
						object->Move();
						
						if (!object->scratchU8[SCRATCHU8_STATUS])
						{
							//Check all players and get the nearest absolute x-difference
							int16_t nearestX = 0x7FFF;
							for (size_t i = 0; i < gLevel->playerList.size(); i++)
							{
								int16_t xDiff = mabs(gLevel->playerList[i]->x.pos - object->x.pos);
								if (xDiff < nearestX)
									nearestX = xDiff;
							}
							
							//Set that we're near a player if we're within 96 pixels of one
							if (nearestX < 96 && object->renderFlags.isOnscreen)
							{
								object->scratchU8[SCRATCHU8_STATUS] = STATUS_NEARPLAYER;
								object->scratchS16[SCRATCHS16_TIME_DELAY] = 29;
							}
							else
							{
								//We're not near Sonic, keep moving
								break;
							}
						}
						else
						{
							//Status is non-zero, keep moving
							break;
						}
					}
					else
					{
						//Change direction
						object->scratchU8[SCRATCHU8_STATUS] = 0;
						object->status.xFlip = !object->status.xFlip;
						object->scratchS16[SCRATCHS16_TIME_DELAY] = 59;
					}
					
					//Set to fire or turn around state
					object->routineSecondary = 0;
					object->xVel = 0;
					object->anim = 0;
				}
			}
			
			//Animate and draw
			object->Animate(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
