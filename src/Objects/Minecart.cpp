#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../MathUtil.h"
#include "../Log.h"

void ObjMinecart(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//U32
		SCRATCHU32_ROLL =	0,
		SCRATCHU32_MAX =	1,
	};
	
	object->ScratchAllocU32(SCRATCHU32_ROLL);
	
	switch (object->routine)
	{
		case 0:
		{
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Minecart.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Minecart.map");
			
			//Initialize other properties
			object->routine++;
			object->renderFlags.alignPlane = true;
			object->widthPixels = 20;
			object->heightPixels = 16;
			object->priority = 1;
			
			object->xRadius = 18;
		}
		//Fallthrough
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
			break;
		}
		case 2:
		{
			int16_t lastX = object->x.pos;
			
			if (!object->routineSecondary) //On-ground
			{
				//Move
				int16_t sin, cos;
				GetSine(object->angle, &sin, &cos);
				object->xVel = (object->inertia * cos) / 0x100;
				object->yVel = (object->inertia * sin) / 0x100;
				
				object->Move();
				
				//Check for collision with the floor
				uint8_t lastAngle = object->angle;
				int16_t floorDistance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos + 8, &object->angle);
				
				if (object->angle & 0x1 || floorDistance >= 14 || abs((int8_t)(object->angle - lastAngle)) >= 0x20)
				{
					//Become airborne
					object->routineSecondary = 1;
				}
				else
				{
					//Move along floor
					object->y.pos += floorDistance;
					
					//Slope gravity
					sin = (sin * 0x40) / 0x100;
					
					//Apply our slope gravity
					if (object->inertia >= 0)
					{
						if (sin < 0)
							sin /= 4;
						object->inertia += sin;
					}
					else
					{
						if (sin >= 0)
							sin /= 4;
						object->inertia += sin;
					}
					
					//Set our mapping frame
					object->mappingFrame = (object->scratchU32[SCRATCHU32_ROLL] / 0xA00) % 2;
					if (abs(object->xVel) > 0x200 && gLevel->frameCounter & 0x1)
						object->mappingFrame += 2;
				}
			}
			else //Mid-air
			{
				//Move and fall
				object->MoveAndFall();
				
				//Check for collision with the floor
				if (object->yVel >= 0)
				{
					int16_t floorDistance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos + 8, &object->angle);
					if (floorDistance < 0)
					{
						//Become grounded
						object->routineSecondary = 0;
						object->y.pos += floorDistance;
						object->inertia = object->xVel;
					}
				}
				
				//Set our mapping frame
				object->mappingFrame = (object->scratchU32[SCRATCHU32_ROLL] / 0xA00) % 2;
			}
			
			//Check for collision with walls
			if (!(object->angle && !object->routineSecondary) || (object->routineSecondary && (abs(object->xVel) > abs(object->yVel) || object->yVel >= 0)))
			{
				if (object->xVel > 0)
				{
					int16_t distance = FindWall(object->x.pos + object->xRadius, object->y.pos - 10, COLLISIONLAYER_NORMAL_LRB, false, nullptr);
					
					//If touching a wall, bounce off and enter broken state
					if (distance < 0)
					{
						object->x.pos += distance;
						if (object->xVel >= 0x1B0 && object->routineSecondary)
						{
							object->yVel = -0x100;
							object->routine++;
						}
						
						object->xVel = -0x100;
						object->inertia = -0x100;
					}
				}
				else if (object->xVel < 0)
				{
					int16_t distance = FindWall(object->x.pos - object->xRadius, object->y.pos - 10, COLLISIONLAYER_NORMAL_LRB, true, nullptr);
					
					//If touching a wall, bounce off and enter broken state
					if (distance < 0)
					{
						object->x.pos -= distance;
						if (object->xVel <= -0x1B0 && object->routineSecondary)
						{
							object->yVel = -0x100;
							object->routine++;
						}
						
						object->xVel = 0x100;
						object->inertia = 0x100;
					}
				}
			}
			
			//Act as a solid and be pushed
			OBJECT_SOLIDTOUCH solid = object->SolidObject(object->xRadius + 8, 16, 6, lastX);
			
			for (size_t v = 0; v < gLevel->playerList.size(); v++)
			{
				PLAYER *player = &gLevel->playerList[v];
				
				//Push velocity stuff
				if (solid.side[v] && player->spindashing == false)
				{
					if (player->x.pos < object->x.pos && player->controlHeld.right)
					{
						//Push against the minecart
						if (player->status.inAir == false && player->status.inBall == false)
						{
							//Accelerate minecart
							if (object->inertia < player->top)
								object->inertia += player->acceleration / 2;
							
							player->status.pushing = true;
							player->status.xFlip = false;
							player->renderFlags.xFlip = false;
						}
						
						//Maintain minecart's velocity
						player->xVel = object->xVel + player->acceleration;
						player->inertia = object->inertia + player->acceleration;
						
						//Move with minecart if pushing into it
						if (object->xVel > 0)
							player->xPosLong += (player->xVel + 0x100) * 0x100;
						else
							player->x.pos++;
					}
					else if (player->controlHeld.left)
					{
						//Push against the minecart
						if (player->status.inAir == false && player->status.inBall == false)
						{
							//Accelerate minecart
							if (object->inertia > -player->top)
								object->inertia -= player->acceleration / 2;
							
							player->status.pushing = true;
							player->status.xFlip = true;
							player->renderFlags.xFlip = true;
						}
						
						//Maintain minecart's velocity
						player->xVel = object->xVel - player->acceleration;
						player->inertia = object->inertia - player->acceleration;
						
						//Move with minecart if pushing into it
						if (object->xVel < 0)
							player->xPosLong += (player->xVel - 0x100) * 0x100;
						else
							player->x.pos--;
					}
					
					//If player is touching a wall we're moving towards, bounce the minecart away so we don't push them in
					if (object->xVel > 0)
					{
						int16_t distance = FindWall(player->x.pos + player->xRadius, player->y.pos, COLLISIONLAYER_NORMAL_LRB, false, nullptr);
						if (distance < 0)
						{
							player->x.pos += distance;
							object->xVel = -0x100;
							object->inertia = -0x100;
						}
					}
					else if (object->xVel < 0)
					{
						int16_t distance = FindWall(player->x.pos - player->xRadius, player->y.pos, COLLISIONLAYER_NORMAL_LRB, true, nullptr);
						if (distance < 0)
						{
							player->x.pos -= distance;
							object->xVel = 0x100;
							object->inertia = 0x100;
						}
					}
				}
				
				//Friction when standing on minecart
				if (object->playerContact[v].standing)
					player->inertia = player->inertia * 8 / 9;
			}
			
			//Draw to screen and handle rolling
			object->scratchU32[SCRATCHU32_ROLL] += object->xVel;
			if (object->xVel > 0)
				object->renderFlags.xFlip = false;
			else if (object->xVel < 0)
				object->renderFlags.xFlip = true;
			
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 3: //Breaking
		{
			//Initialize broken state
			object->routine++;
			object->routineSecondary = 60;
			object->mappingFrame = 4;
			
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				PLAYER *player = &gLevel->playerList[i];
				
				if (object->playerContact[i].standing)
				{
					object->ExitPlatform(player, i);
					player->routine = PLAYERROUTINE_HURT;
					player->anim = PLAYERANIMATION_SLIDE;
					player->xVel = object->xVel * 2;
					player->yVel = object->yVel * 2;
				}
			}
		}
	//Fallthrough
		case 4: //Broken
		{
			//Draw to screen, flickering for 1 second
			object->MoveAndFall();
			
			if ((int8_t)(--object->routineSecondary) < 0)
				object->deleteFlag = true;
			else if (object->routineSecondary & 0x1)
				object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
		}
	}
	
	//If below the stage, delete us
	if (object->y.pos >= gLevel->bottomBoundaryTarget + 48)
		object->deleteFlag = true;
}
