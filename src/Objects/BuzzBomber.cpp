#include "../Game.h"
#include "../Objects.h"
#include "../MathUtil.h"

//Buzz Bomber animations
static const uint8_t animationFly1[] =		{0x01,0x00,0x01,ANICOMMAND_RESTART};
static const uint8_t animationFly2[] =		{0x01,0x02,0x03,ANICOMMAND_RESTART};
static const uint8_t animationFiring[] =	{0x01,0x04,0x05,ANICOMMAND_RESTART};

static const uint8_t *animationList[] = {
	animationFly1,
	animationFly2,
	animationFiring,
};

//Missile animations
static const uint8_t animationCharge[] =	{0x07,0x00,0x01,ANICOMMAND_ADVANCE_ROUTINE};
static const uint8_t animationFire[] =		{0x01,0x02,0x03,ANICOMMAND_RESTART};

static const uint8_t *missileAnimationList[] = {
	animationCharge,
	animationFire,
};

//Buzz Bomber Missile
void ObjBuzzBomberMissile(OBJECT *object)
{
	//Define and allocate our scratch
	struct SCRATCH
	{
		int16_t fireDelay = 14;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
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
			if (--scratch->fireDelay >= 0)
				break;
			
			//Draw and animate
			object->Animate_S1(missileAnimationList);
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
			object->Animate_S1(missileAnimationList);
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
	//Define and allocate our scratch
	struct SCRATCH
	{
		uint8_t state = 0;
		int16_t timeDelay = 0;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
	//Status bitmask
	#define STATE_FIRED		0x01
	#define STATE_NEARPLAYER	0x02
	
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
					if (--scratch->timeDelay < 0)
					{
						if (!(scratch->state & STATE_NEARPLAYER))
						{
							//If not near a player, fly for a bit longer
							object->routineSecondary = 1;
							scratch->timeDelay = 127;
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
							scratch->state = STATE_FIRED;
							scratch->timeDelay = 59;
							object->anim = 2;
						}
					}
					break;
				}
				case 1:
				{
					//Move for the given amount of time and turn around, or prematurely stop if near a player and fire a projectile
					if (--scratch->timeDelay >= 0)
					{
						//Move and check if we're near a player
						object->Move();
						
						if (!scratch->state)
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
								scratch->state = STATE_NEARPLAYER;
								scratch->timeDelay = 29;
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
						scratch->state = 0;
						object->status.xFlip = !object->status.xFlip;
						scratch->timeDelay = 59;
					}
					
					//Set to fire or turn around state
					object->routineSecondary = 0;
					object->xVel = 0;
					object->anim = 0;
				}
			}
			
			//Animate and draw
			object->Animate_S1(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
