#include "../Game.h"
#include "../Objects.h"
#include "../MathUtil.h"

//Buzz Bomber animations
static const uint8_t animationBlank[] =		{0x0F,0x0A,ANICOMMAND_RESTART};
static const uint8_t animationDrop[] =		{0x13,0x00,0x01,0x03,0x04,0x05,ANICOMMAND_GO_BACK_FRAMES,0x01};
static const uint8_t animationFly_Pal0[] =	{0x02,0x06,0x07,ANICOMMAND_RESTART};
static const uint8_t animationFly_Pal1[] =	{0x02,0x08,0x09,ANICOMMAND_RESTART};
static const uint8_t animationFires[] =		{0x13,0x00,0x01,0x01,0x02,0x01,0x01,0x00,ANICOMMAND_ADVANCE_ROUTINE};

static const uint8_t *animationList[] = {
	animationBlank,
	animationDrop,
	animationFly_Pal0,
	animationFly_Pal1,
	animationFires,
};

//Missile animations
static const uint8_t animationFire[] =		{0x01,0x02,0x03,ANICOMMAND_RESTART};
static const uint8_t *missileAnimationList[] = {
	animationFire,
};

//Newtron Missile
void ObjNewtronMissile(OBJECT *object)
{
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
			
			//Set our collision and animation
			object->collisionType = COLLISIONTYPE_HURT;
			object->touchWidth = 6;
			object->touchHeight = 6;
			object->hurtType.reflect = true;
			
			//Draw and animate
			object->Animate_S1(missileAnimationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 1:
		{
			//Delete if off-screen
			if (!object->renderFlags.isOnscreen)
			{
				object->deleteFlag = true;
				return;
			}
			
			//Move, draw, and animate
			if (object->collisionType == COLLISIONTYPE_HURT)
				object->Move();
			else
				object->MoveAndFall();
			object->Animate_S1(missileAnimationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
	}
}

//Newtron / Leon object
void ObjNewtron(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
			if (object->subtype == 0)
				object->mapping.mappings = gLevel->GetObjectMappings("data/Object/NewtronBlue.map");
			else
				object->mapping.mappings = gLevel->GetObjectMappings("data/Object/NewtronGreen.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->priority = 4;
			
			//Initialize visual size and collision size
			object->widthPixels = 20;
			object->heightPixels = 32;
			object->yRadius = 16;
		}
	//Fallthrough
		case 1:
		{
			//Check all players and get the nearest x-difference (non-absolute)
			int16_t nearestX = 0x7FFF;
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				int16_t xDiff = gLevel->playerList[i]->x.pos - object->x.pos;
				if (mabs(xDiff) < mabs(nearestX))
					nearestX = xDiff;
			}
			
			switch (object->routineSecondary)
			{
				case 0: //Waiting for player to come nearby
				{
					//Face towards nearest player
					if (nearestX < 0)
						object->status.xFlip = false;
					else
						object->status.xFlip = true;
					nearestX = mabs(nearestX);
					
					//If near a player, appear
					if (nearestX < 128)
					{
						//Enter next routine and use animation
						if (object->subtype == 0)
						{
							//Blue - drop to the ground and fly across
							object->anim = 1;
							object->routineSecondary = 1;
						}
						else
						{
							//Green - appear, then shoot a missile
							object->anim = 4;
							object->routineSecondary = 4;
						}
					}
					break;
				}
				case 1: //Blue - appearing
				{
					if (object->mappingFrame < 4)
					{
						//Waiting to finish animation, face towards nearest player
						if (nearestX < 0)
							object->status.xFlip = false;
						else
							object->status.xFlip = true;
					}
					else
					{
						//Start falling
						if (object->mappingFrame == 1)
						{
							//Set collision if on appeared frame
							object->collisionType = COLLISIONTYPE_ENEMY;
							object->touchWidth = 20;
							object->touchHeight = 16;
						}
						
						//Fall, and check if we're on a floor yet
						object->MoveAndFall();
						
						int16_t distance = object->CheckCollisionDown_1Point(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos + object->yRadius, nullptr);
						if (distance < 0)
						{
							//We've touched a floor, clip out and prepare for our next routine
							object->y.pos += distance;
							object->yVel = 0;
							
							//Set our routine and animation
							object->routineSecondary = 2;
							if (object->subtype == 0) //now just wait a minute
								object->anim = 2;
							else
								object->anim = 3;
							
							//Set collision and velocity
							object->collisionType = COLLISIONTYPE_ENEMY;
							object->touchWidth = 20;
							object->touchHeight = 8;
							
							if (object->status.xFlip)
								object->xVel = 0x200;
							else
								object->xVel = -0x200;
						}
					}
					break;
				}
				case 2: //Blue - flying across floor
				{
					//Move and move across floor
					object->Move();
					
					int16_t distance = object->CheckCollisionDown_1Point(COLLISIONLAYER_NORMAL_TOP, object->x.pos + (object->status.xFlip ? -16 : 16), object->y.pos + object->yRadius, nullptr);
					if (distance >= -8 && distance < 12)
						object->y.pos += distance; //Still near floor, match y position
					else
						object->routineSecondary = 3; //Lost contact with floor, fly without checking floor now
					break;
				}
				case 3: //Blue - just flying (lost contact with floor)
				{
					//Just move, nothing else, lame
					object->Move();
					break;
				}
				case 4: //Green - appear, then fire missile
				{
					//Set collision if on appeared frame
					if (object->mappingFrame == 1)
					{
						object->collisionType = COLLISIONTYPE_ENEMY;
						object->touchWidth = 20;
						object->touchHeight = 16;
					}
					
					//Fire missile if appeared and not fired before
					if (object->mappingFrame == 2 && object->status.objectSpecific == false)
					{
						//Set flag that we've already fired
						object->status.objectSpecific = true;
						
						//Fire a missile in our facing direction
						OBJECT *projectile = new OBJECT(&ObjNewtronMissile);
						projectile->xVel = 0x200;
						
						int16_t xOff = 20;
						if (!object->status.xFlip)
						{
							//Invert if facing left
							xOff = -xOff;
							projectile->xVel = -projectile->xVel;
						}
						
						projectile->x.pos = object->x.pos + xOff;
						projectile->y.pos = object->y.pos - 8;
						projectile->status = object->status;
						gLevel->objectList.link_back(projectile);
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
		case 2:
		{
			//Green - deleted after firing missile
			object->deleteFlag = true;
			gLevel->ReleaseObjectLoad(object);
			break;
		}
	}
}
