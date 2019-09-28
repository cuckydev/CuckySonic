#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

void ObjSpring(OBJECT *object)
{
	enum SCRATCH
	{
		//S16
		SCRATCHS16_FORCE = 0,
	};
	
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			//object->texture = gLevel->GetObjectTexture("data/Object/YellowSpring.bmp");
			//object->mappings = gLevel->GetObjectMappings("data/Object/SpringV.map");
			
			//Set render properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 16;
			object->priority = 4;
			
			//Subtype specific initialization
			switch ((object->subtype >> 4) & 0x7)
			{
				case 0: //Up
					break; //Just use the common initialization below
				case 1: //Horizontal
					object->routine = 2;
					object->anim = 2;
					object->mappingFrame = 0x03;
					object->widthPixels = 8;
					break;
				case 2: //Down
					object->routine = 3;
					object->mappingFrame = 0x06;
					object->status.yFlip = true;
					break;
				case 3: //Diagonally up
					object->routine = 4;
					object->anim = 4;
					object->mappingFrame = 0x07;
					break;
				case 4: //Diagonally down
					object->routine = 5;
					object->anim = 4;
					object->mappingFrame = 0x0A;
					object->status.yFlip = true;
					break;
			}
			
			//Get our spring force
			if (object->subtype & 0x2)
				object->scratchS16[SCRATCHS16_FORCE] = -0x0A00; //Yellow spring's force
			else
				object->scratchS16[SCRATCHS16_FORCE] = -0x1000; //Red spring's force
			
			//Check if we should draw as a red spring
			if (!(object->subtype & 0x2))
				;//object->texture = gLevel->GetObjectTexture("data/Object/RedSpring.bmp");
			break;
		}
		case 1: //Upwards
		case 3: //Downwards
		{
			//Act as solid
			OBJECT_SOLIDTOUCH touch = object->SolidObject(27, 8, object->x.pos);
			
			//Check if any players touched the top of us
			int i = 0;
			for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
			{
				if (object->routine == 1 ? object->playerContact[i++].standing : touch.bottom[i++])
				{
					//Play bouncing animation
					object->anim = 1;
					object->prevAnim = 0;
					
					//Launch player
					player->y.pos -= object->routine == 1 ? 8 : -8;
					player->yVel = object->scratchS16[SCRATCHS16_FORCE];
					player->status.inAir = true;
					player->status.shouldNotFall = false;
					if (object->routine == 1)
						player->anim = PLAYERANIMATION_SPRING;
					player->routine = PLAYERROUTINE_CONTROL; //WHY
					
					if (object->routine == 3) //Reverse if downwards spring
						player->yVel = -player->yVel;
					
					if (object->subtype & 0x80) //Unused flag?
						player->xVel = 0;
					
					if (object->subtype & 0x1) //Twirl flag
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & 0x2) ? 0 : 1; //Do 1 extra flip if a red spring
						player->flipSpeed = 4;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & 0xC) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & 0xC) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//PlaySound(SOUNDID_SPRING);
				}
			}
			
			//object->Draw();
			break;
		}
		case 2: //Horizontal
		{
			//Act as solid
			object->SolidObject(19, 15, object->x.pos);
			
			//Check if any players touched our sides
			int i = 0;
			for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
			{
				if (object->playerContact[i++].pushing)
				{
					//Reverse flip if to the left
					bool direction = object->status.xFlip;
					if ((object->x.pos - player->x.pos) >= 0)
						direction ^= 1;
					
					//Play bouncing animation
					object->anim = 3;
					object->prevAnim = 0;
					
					//Launch player
					player->xVel = object->scratchS16[SCRATCHS16_FORCE];
					player->x.pos += direction ? -8 : 8;
					player->status.xFlip = true;
					
					if (!(object->status.xFlip ^ direction)) //Reverse if flipped
					{
						player->status.xFlip = false;
						player->xVel = -player->xVel;
					}
					
					//Handle player launching on the ground (lock controls for a bit, play walk animation)
					player->moveLock = 15;
					player->inertia = player->xVel;
					if (!player->status.inBall)
						player->anim = PLAYERANIMATION_WALK;
					
					if (object->subtype & 0x80) //Unused flag?
						player->yVel = 0;
					
					if (object->subtype & 0x1) //Twirl flag
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & 0x2) ? 1 : 3; //Do 2 extra flips if a red spring
						player->flipSpeed = 8;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & 0xC) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & 0xC) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//PlaySound(SOUNDID_SPRING);
				}
			}
			
			//object->Draw();
			break;
		}
	}
}
