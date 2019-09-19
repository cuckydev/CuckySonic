#include "Object.h"
#include "Objects.h"
#include "Game.h"
#include "Log.h"
#include "LevelCollision.h"
#include "Error.h"

OBJECT::OBJECT(OBJECT **linkedList, void (*objectFunction)(OBJECT *object))
{
	memset(this, 0, sizeof(OBJECT));
	
	//Set type and subtype
	function = objectFunction;
	
	//Attach to linked list (if applicable)
	if (linkedList != NULL)
	{
		list = linkedList;
		
		//If linked list is unset, set us as the first 
		if (*linkedList == NULL)
		{
			*linkedList = this;
			return;
		}
		
		//Attach us to the linked list
		for (OBJECT *object = *linkedList; 1; object = object->next)
		{
			if (object->next == NULL)
			{
				object->next = this;
				break;
			}
		}
	}
}

OBJECT::~OBJECT()
{
	//Detach from linked list
	if (list != NULL)
	{
		for (OBJECT **object = list; *object != NULL; object = &(*object)->next)
		{
			if (*object == this)
			{
				*object = next;
				break;
			}
		}
	}
	
	//Destroy children
	for (OBJECT *object = children; object != NULL;)
	{
		OBJECT *next = object->next;
		delete object;
		object = next;
	}
}

//Movement and gravity
void OBJECT::Move()
{
	xPosLong += xVel * 0x100;
	yPosLong += yVel * 0x100;
}

void OBJECT::MoveAndFall()
{
	xPosLong += xVel * 0x100;
	yPosLong += yVel * 0x100;
	yVel += 0x38;
}

//Hurt function
const uint16_t enemyPoints[] = {100, 200, 500, 1000};

bool OBJECT::Hurt(PLAYER *player)
{
	//Handle chain point bonus
	uint16_t lastCounter = player->chainPointCounter;
	player->chainPointCounter++;
	
	//Cap our actual bonus at 3
	if (lastCounter >= 3)
		lastCounter = 3;
	
	//move.w	d0,objoff_3E(a1)
	
	//Get our bonus points
	uint16_t bonus = enemyPoints[lastCounter];
	
	//If destroyed 16 enemies, give us 10000 points
	if (player->chainPointCounter >= 16)
	{
		bonus = 10000;
		//move.w	#$A,objoff_3E(a1)
	}
	
	//Give us the points bonus
	AddToScore(bonus);
	
	//Turn us into an explosion
	function = &ObjExplosion;
	routine = 0;
	
	//Adjust player's velocity
	if (player->yVel >= 0)
	{
		if (player->y.pos < y.pos)
			player->yVel = -player->yVel; //If above us, reverse player velocity
		else
			player->yVel -= 0x100; //If below us, slow down a bit
	}
	else
	{
		player->yVel += 0x100; //If moving upwards, slow down a bit
	}
	
	return true;
}

//Shared animate function
void OBJECT::Animate(const uint8_t **animationList)
{
	//Restart animation if not the same as the previous
	if (anim != prevAnim)
	{
		prevAnim = anim;
		animFrame = 0;
		animFrameDuration = 0;
	}
	
	//Wait for next frame
	if (--animFrameDuration >= 0)
		return;
	
	//Get the animation to reference
	const uint8_t *animation = animationList[anim];
	
	//Set next frame duration
	animFrameDuration = animation[0];
	
	//Handle commands
	if (animation[1 + animFrame] >= 0x80)
	{
		switch (animation[1 + animFrame])
		{
			case 0xFF:	//Restart animation
				animFrame = 0;
				break;
			case 0xFE:	//Go back X amount of frames
				animFrame -= animation[2 + animFrame];
				break;
			case 0xFD:	//Switch to X animation
				anim = animation[2 + animFrame];
				return;
			case 0xFC:	//Advance routine
				routine++;
				animFrameDuration = 0;
				animFrame++; //???
				return;
			case 0xFB:	//Reset animation and clear secondary routine
				animFrame = 0;
				routineSecondary = 0;
				return;
			case 0xFA:	//Increment secondary routine
				routineSecondary++;
				return;
			case 0xF9:	//Not sure
				return;
		}
	}
	
	//Set our mapping frame and flip
	mappingFrame = animation[1 + animFrame] & 0x7F;
	renderFlags.xFlip = status.xFlip;
	renderFlags.yFlip = status.yFlip;
	animFrame++;
}

//Collision functions
int16_t OBJECT::CheckFloorEdge(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle)
{
	int16_t distance = FindFloor(xPos, yPos + yRadius, layer, false, outAngle);
	if (outAngle != NULL)
		*outAngle = ((*outAngle) & 1) ? 0 : (*outAngle);
	return distance;
}

//Object collision functions
void OBJECT::PlatformObject(int16_t width, int16_t height, int16_t lastXPos)
{
	int i = 0;
	for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
	{
		//If the player is already standing on us
		if (playerContact[i].standing == true)
		{
			//Check if we're still on the platform
			int16_t xDiff = player->x.pos - lastXPos + width;
			
			if (!player->status.inAir && xDiff >= 0 && xDiff <= width * 2)
				player->MoveOnPlatform((void*)this, height, lastXPos);
			else
				ExitPlatform(player, i);
		}
		else
		{
			//Check if we're going to land on the platform
			LandOnPlatform(player, i, width, height, lastXPos);
		}
		
		//Check next player's contact
		i++;
	}
}

void OBJECT::LandOnPlatform(PLAYER *player, int i, int16_t width, int16_t height, int16_t lastXPos)
{
	//Check if we're in a state where we can enter onto the platform
	int16_t xDiff = (player->x.pos - lastXPos) + width;
	if (player->yVel < 0 || xDiff < 0 || xDiff > width * 2)
		return;
	
	if (player->status.reverseGravity)
	{
		//Land on platform
		int16_t playerBottom = (player->y.pos - player->yRadius) - 4;
		int16_t yThing = -((y.pos + height) - playerBottom);
		
		//If we're on top of the platform, and not in an intangible state
		if (yThing > 0 || yThing < -16 || player->objectControl.disableObjectInteract || player->routine == PLAYERROUTINE_DEATH)
			return;
		
		//Land on top of the platform
		player->y.pos += yThing + 4;
		player->AttachToObject((void*)this, (&playerContact[i].standing - (size_t)this));
	}
	else
	{
		//Land on platform
		int16_t playerBottom = (player->y.pos + player->yRadius) + 4;
		int16_t yThing = (y.pos - height) - playerBottom;
		
		//If we're on top of the platform, and not in an intangible state
		if (yThing > 0 || yThing < -16 || player->objectControl.disableObjectInteract || player->routine == PLAYERROUTINE_DEATH)
			return;
		
		//Land on top of the platform
		player->y.pos += yThing + 4;
		player->AttachToObject((void*)this, (&playerContact[i].standing - (size_t)this));
	}
}

void OBJECT::ExitPlatform(PLAYER *player, int i)
{
	player->status.shouldNotFall = false;
	playerContact[i].standing = false;
	player->status.inAir = true;
}

//Solid object and its variants
OBJECT_SOLIDTOUCH OBJECT::SolidObject(int16_t width, int16_t height, int16_t lastXPos)
{
	//Get cleared solid touch
	OBJECT_SOLIDTOUCH solidTouch;
	memset(&solidTouch, 0, sizeof(OBJECT_SOLIDTOUCH));
	
	//Check all players
	int i = 0;
	for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
	{
		//Check if we're still standing on the object
		if (playerContact[i].standing)
		{
			//Check if we're to exit the top of the object
			int16_t xDiff = (player->x.pos - lastXPos) + width;
			if (player->status.inAir || xDiff < 0 || xDiff >= width * 2)
			{
				//Exit top as platform, then check next player
				ExitPlatform(player, i++);
				continue;
			}
			
			//Move with the object
			player->MoveOnPlatform((void*)this, height, lastXPos);
			
			//Check next player
			i++;
			continue;
		}
		else
		{
			//Check for collisions, then check next player
			SolidObjectCont(&solidTouch, player, i++, width, height, lastXPos);
			continue;
		}
	}
	
	return solidTouch;
}

void OBJECT::SolidObjectCont(OBJECT_SOLIDTOUCH *solidTouch, PLAYER *player, int i, int16_t width, int16_t height, int16_t lastXPos)
{
	//Check if we're within range
	int16_t xDiff = (player->x.pos - x.pos) + width; //d0
	
	int16_t checkHeight; //d2
	int16_t yDiff; //d3
	
	if (player->status.reverseGravity)
	{
		checkHeight = player->yRadius + height;
		yDiff = (-(player->y.pos - y.pos) + 4) + checkHeight;
	}
	else
	{
		checkHeight = player->yRadius + height;
		yDiff = (player->y.pos - y.pos + 4) + checkHeight;
	}
	
	//Perform main collision checks if within range and not under the influence of another object
	if (!(player->objectControl.disableObjectInteract || (xDiff < 0 || xDiff > (width * 2)) || (yDiff < 0 || yDiff > (checkHeight * 2))))
	{
		//Check if we're intangible
		if (!(player->routine == PLAYERROUTINE_DEATH || player->debug))
		{
			//Get our clip differences
			int16_t xClip = xDiff;
			if (xDiff >= width)
			{
				xDiff -= width * 2;
				xClip = -xDiff;
			}
			
			int16_t yClip = yDiff;
			if (yDiff >= height)
			{
				yDiff -= (4 + (checkHeight * 2));
				yClip = -yDiff;
			}
			
			//If our horizontal difference is greater than the vertical difference (we're above / below the object)
			if (yClip <= xClip)
			{
				if (yDiff >= 0)
				{
					//Check our vertical difference
					if (yDiff < 16 || (yDiff < 20 /*&& object->function != ObjLauncherSpring*/))
					{
						//Check our horizontal range
						int16_t xDiff2 = (player->x.pos - lastXPos) + widthPixels;
						
						if (xDiff2 >= 0 && xDiff2 < widthPixels * 2 && player->yVel >= 0)
						{
							//Land on the object
							if (player->status.reverseGravity)
								player->y.pos -= yDiff - 4;
							else
								player->y.pos += yDiff - 4;
							player->AttachToObject((void*)this, (&playerContact[i].standing - (size_t)this));
							
							//Set top touch flag
							solidTouch->top[i] = true;
						}
						
						return;
					}
					
					//Clear our pushing
					SolidObjectClearPush(player, i);
					return;
				}
				else
				{
					if (player->yVel != 0)
					{
						//Clip us out of the top (why does it check if yOff is negative?)
						if (player->yVel < 0 && yDiff < 0)
						{
							if (player->status.reverseGravity)
								player->y.pos -= yDiff;
							else
								player->y.pos += yDiff;
							player->yVel = 0;
						}
						
						//Set bottom touch flag
						solidTouch->bottom[i] = true;
						return;
					}
					else
					{
						if (!player->status.inAir)
						{
							int16_t absXDiff = abs(xClip);
							
							if (absXDiff >= 16)
							{
								//Crush the player and set the bottom touch flag
								player->KillCharacter(SOUNDID_HURT);
								solidTouch->bottom[i] = true;
								return;
							}
							
							//Fallthrough into horizontal check
						}
						else
						{
							//Set bottom touch flag
							solidTouch->bottom[i] = true;
							return;
						}
					}
				}
			}
			
			//Horizontal checking
			if (yClip > 4)
			{
				//Hault our velocity if running into sides
				if (xDiff > 0)
				{
					if (player->xVel >= 0)
					{
						player->inertia = 0;
						player->xVel = 0;
					}
				}
				else if (xDiff < 0)
				{
					if (player->xVel < 0)
					{
						player->inertia = 0;
						player->xVel = 0;
					}
				}
				
				//Clip out of side
				player->x.pos -= xDiff;
				
				//Contact on ground: Set side touch and set pushing flags
				if (!player->status.inAir)
				{
					solidTouch->side[i] = true;
					playerContact[i].pushing = true;
					player->status.pushing = true;
					return;
				}
			}
			
			//Contact in mid-air: Set side touch and clear pushing flags
			solidTouch->side[i] = true;
			playerContact[i].pushing = false;
			player->status.pushing = false;
			return;
		}
	}
	
	//Clear out pushing
	SolidObjectClearPush(player, i);
}

void OBJECT::SolidObjectClearPush(PLAYER *player, int i)
{
	//Check we should stop pushing
	if (playerContact[i].pushing)
	{
		//Reset animation
		if (player->anim != PLAYERANIMATION_ROLL)
			player->anim = PLAYERANIMATION_RUN; //wrong animation id
		
		//Clear pushing flags
		playerContact[i].pushing = false;
		player->status.pushing = false;
	}
}

//Update and drawing objects
bool OBJECT::Update()
{
	//Clear drawing flag
	isDrawing = false;
	
	//Run our object code
	if (function != NULL)
		function(this);
	
	//Check if any of our assets failed to load
	if (texture != NULL && texture->fail != NULL)
		return Error(texture->fail);
	if (mappings != NULL && mappings->fail != NULL)
		return Error(mappings->fail);
	
	//Update children's code
	for (OBJECT *object = children; object != NULL; object = object->next)
	{
		if (object->Update())
			return true;
	}
	
	return false;
}

void OBJECT::Draw()
{
	//Set drawing flag
	isDrawing = true;
}

void OBJECT::DrawToScreen()
{
	//Don't draw if we don't have textures or mappings
	if (texture != NULL && mappings != NULL)
	{
		//Draw our sprite
		SDL_Rect *mapRect = &mappings->rect[mappingFrame];
		SDL_Point *mapOrig = &mappings->origin[mappingFrame];
		
		int origX = mapOrig->x;
		int origY = mapOrig->y;
		if (renderFlags.xFlip)
			origX = mapRect->w - origX;
		if (renderFlags.yFlip)
			origY = mapRect->h - origY;
		
		int alignX = renderFlags.alignPlane ? gLevel->camera->x : 0;
		int alignY = renderFlags.alignPlane ? gLevel->camera->y : 0;
		gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, mapRect, gLevel->GetObjectLayer(highPriority, priority), x.pos - origX - alignX, y.pos - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
	}
}

void OBJECT::DrawRecursive()
{
	if (isDrawing)
		DrawToScreen();
	for (OBJECT *object = children; object != NULL; object = object->next)
		object->DrawRecursive();
}
