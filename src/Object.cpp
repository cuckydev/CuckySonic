#include <stdlib.h>
#include <string.h>

#include "Object.h"
#include "Objects.h"

#include "Game.h"
#include "Log.h"
#include "Error.h"
#include "Player.h"

//Object class
OBJECT::OBJECT(std::deque<OBJECT*> *linkedList, OBJECTFUNCTION objectFunction)
{
	//Clear memory
	memset(this, 0, sizeof(OBJECT));
	
	//Set function
	function = objectFunction;
	prevFunction = objectFunction;
	
	//Reset children and draw instance linked lists
	drawInstances = std::deque<OBJECT_DRAWINSTANCE*>(0);
	children = std::deque<OBJECT*>(0);
	
	//Attach to linked list (if applicable)
	if (linkedList != nullptr)
		linkedList->push_back(this);
}

OBJECT::~OBJECT()
{
	//Free all scratch memory
	free(scratchU8);
	free(scratchS8);
	free(scratchU16);
	free(scratchS16);
	free(scratchU32);
	free(scratchS32);
	
	//Destroy draw instances
	drawInstances.clear();
	drawInstances.shrink_to_fit();
	
	//Destroy children
	children.clear();
	children.shrink_to_fit();
}

//Scratch allocation
#define SCRATCH_ALLOC(name, type, max) if (name == nullptr) { name = (type*)malloc(max * sizeof(type)); memset(name, 0, max * sizeof(type)); }

void  OBJECT::ScratchAllocU8(int max) { SCRATCH_ALLOC(scratchU8,   uint8_t, max) }
void  OBJECT::ScratchAllocS8(int max) { SCRATCH_ALLOC(scratchS8,    int8_t, max) }
void OBJECT::ScratchAllocU16(int max) { SCRATCH_ALLOC(scratchU16, uint16_t, max) }
void OBJECT::ScratchAllocS16(int max) { SCRATCH_ALLOC(scratchS16,  int16_t, max) }
void OBJECT::ScratchAllocU32(int max) { SCRATCH_ALLOC(scratchU32, uint32_t, max) }
void OBJECT::ScratchAllocS32(int max) { SCRATCH_ALLOC(scratchS32,  int32_t, max) }

//Movement and gravity
void OBJECT::Move()
{
	xPosLong += xVel << 8;
	yPosLong += yVel << 8;
}

void OBJECT::MoveAndFall()
{
	xPosLong += xVel << 8;
	yPosLong += yVel << 8;
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
	if (outAngle != nullptr)
		*outAngle = ((*outAngle) & 1) ? 0 : (*outAngle);
	return distance;
}

//Object collision functions
void OBJECT::PlatformObject(int16_t width, int16_t height, int16_t lastXPos)
{
	int i = 0;
	for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
	{
		//If the player is already standing on us
		if (playerContact[i].standing == true)
		{
			//Check if we're still on the platform
			int16_t xDiff = player->x.pos - lastXPos + width;
			
			if (!player->status.inAir && xDiff >= 0 && xDiff < width * 2)
				player->MoveOnPlatform(this, height, lastXPos);
			else
				ExitPlatform(player, i);
		}
		else
		{
			//Check if we're going to land on the platform
			LandOnPlatform(player, i, width, width * 2, height, lastXPos);
		}
		
		//Check next player's contact
		i++;
	}
}

bool OBJECT::LandOnPlatform(PLAYER *player, int i, int16_t width1, int16_t width2, int16_t height, int16_t lastXPos)
{
	//Check if we're in a state where we can enter onto the platform
	int16_t xDiff = (player->x.pos - lastXPos) + width1;
	if (player->yVel < 0 || xDiff < 0 || xDiff >= width2)
		return false;
	
	if (player->status.reverseGravity)
	{
		//Land on platform
		int16_t playerBottom = (player->y.pos - player->yRadius) - 4;
		int16_t yThing = -((y.pos + height) - playerBottom);
		
		//If we're on top of the platform, and not in an intangible state
		if (yThing > 0 || yThing < -16 || player->objectControl.disableObjectInteract || player->routine == PLAYERROUTINE_DEATH)
			return false;
		
		//Land on top of the platform
		player->y.pos += yThing + 4;
		player->AttachToObject(this, (&playerContact[i].standing - (size_t)this));
		return true;
	}
	else
	{
		//Land on platform
		int16_t playerBottom = (player->y.pos + player->yRadius) + 4;
		int16_t yThing = (y.pos - height) - playerBottom;
		
		//If we're on top of the platform, and not in an intangible state
		if (yThing > 0 || yThing < -16 || player->objectControl.disableObjectInteract || player->routine == PLAYERROUTINE_DEATH)
			return false;
		
		//Land on top of the platform
		player->y.pos += yThing + 4;
		player->AttachToObject(this, (&playerContact[i].standing - (size_t)this));
		return true;
	}
}

void OBJECT::ExitPlatform(PLAYER *player, int i)
{
	player->status.shouldNotFall = false;
	playerContact[i].standing = false;
	player->status.inAir = true;
}

//Solid object and its variants
OBJECT_SOLIDTOUCH OBJECT::SolidObject(int16_t width, int16_t height_air, int16_t height_standing, int16_t lastXPos)
{
	//Get cleared solid touch
	OBJECT_SOLIDTOUCH solidTouch;
	memset(&solidTouch, 0, sizeof(OBJECT_SOLIDTOUCH));
	
	//Check all players
	int i = 0;
	for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
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
			player->MoveOnPlatform(this, height_standing, lastXPos);
			
			//Check next player
			i++;
			continue;
		}
		else
		{
			//Check for collisions, then check next player
			SolidObjectCont(&solidTouch, player, i++, width, height_air, lastXPos);
			continue;
		}
	}
	
	return solidTouch;
}

void OBJECT::SolidObjectCont(OBJECT_SOLIDTOUCH *solidTouch, PLAYER *player, int i, int16_t width, int16_t height, int16_t lastXPos)
{
	//Check if we're within range
	int16_t xDiff = (player->x.pos - x.pos) + width; //d0
	
	int16_t yDiff; //d3
	if (player->status.reverseGravity)
		yDiff = (-(player->y.pos - y.pos) + 4) + (height += player->yRadius);
	else
		yDiff = (player->y.pos - y.pos + 4) + (height += player->yRadius);
	
	//Perform main collision checks if within range and not under the influence of another object
	if (!(player->objectControl.disableObjectInteract || (xDiff < 0 || xDiff > (width * 2)) || (yDiff < 0 || yDiff > (height * 2))))
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
				yDiff -= (4 + (height * 2));
				yClip = -yDiff;
			}
			
			//If our horizontal difference is greater than the vertical difference (we're above / below the object)
			if (yClip <= xClip)
			{
				if (yDiff >= 0)
				{
					//Check our vertical difference
					if (yDiff < 16)
					{
						//Check our horizontal range
						int16_t xDiff2 = (player->x.pos - lastXPos) + widthPixels;
						
						if (xDiff2 >= 0 && xDiff2 < widthPixels * 2 && player->yVel >= 0)
						{
							//Land on the object
							yDiff -= 4;
							if (player->status.reverseGravity)
								player->y.pos += (yDiff + 1);
							else
								player->y.pos -= (yDiff + 1);
							
							player->AttachToObject(this, (&playerContact[i].standing - (size_t)this));
							
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
								yDiff = -yDiff;
							
							player->y.pos -= yDiff;
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
			
			//Horizontal checking (if yClip is greater than xClip or it fell-through to here)
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
	
	//Clear our pushing
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

//Common solid functions
void OBJECT::ClearSolidContact()
{
	//Check all players and clear all contact
	int i = 0;
	
	for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
	{
		//Clear pushing and standing
		if (playerContact[i].standing || playerContact[i].pushing)
		{
			//Clear our contact flags
			player->status.shouldNotFall = false;
			player->status.pushing = false;
			playerContact[i].standing = false;
			playerContact[i].pushing = false;
			
			//Place the player in mid-air, this is weird and stupid god damn
			player->status.inAir = true;
		}
		
		//Check next player's contact
		i++;
	}
}

//Update and drawing objects
bool OBJECT::Update()
{
	//If our function has changed, free any allocated scratch memory
	if (function != prevFunction)
	{
		//Free all scratch memory
		free(scratchU8);	scratchU8 =  nullptr;
		free(scratchS8);	scratchS8 =  nullptr;
		free(scratchU16);	scratchU16 = nullptr;
		free(scratchS16);	scratchS16 = nullptr;
		free(scratchU32);	scratchU32 = nullptr;
		free(scratchS32);	scratchS32 = nullptr;
		
		//Remember this as our last function
		prevFunction = function;
	}
	
	//Destroy draw instances
	drawInstances.clear();
	drawInstances.shrink_to_fit();
	
	//Run our object code
	if (function != nullptr)
		function(this);
	
	//Check if any of our assets failed to load
	if (texture != nullptr && texture->fail != nullptr)
		return Error(texture->fail);
	if (mappings != nullptr && mappings->fail != nullptr)
		return Error(mappings->fail);
	
	//Check for deletion
	if (deleteFlag)
	{
		//Remove player references to us
		for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
			if (player->interact == this)
				player->interact = nullptr;
	}
	else
	{
		//Update children's code
		for (size_t i = 0; i < children.size(); i++)
		{
			if (children[i]->Update())
				return true;
		}
		
		CHECK_LINKEDLIST_DELETE(children);
	}
	
	return false;
}

void OBJECT::DrawInstance(OBJECT_RENDERFLAGS iRenderFlags, TEXTURE *iTexture, MAPPINGS *iMappings, bool iHighPriority, uint8_t iPriority, uint16_t iMappingFrame, int16_t iXPos, int16_t iYPos)
{
	//Create a draw instance with the properties given
	OBJECT_DRAWINSTANCE *newInstance = new OBJECT_DRAWINSTANCE(&drawInstances);
	newInstance->renderFlags = iRenderFlags;
	newInstance->texture = iTexture;
	newInstance->mappings = iMappings;
	newInstance->highPriority = iHighPriority;
	newInstance->priority = iPriority;
	newInstance->mappingFrame = iMappingFrame;
	newInstance->xPos = iXPos;
	newInstance->yPos = iYPos;
}

void OBJECT::Draw()
{
	//Draw our draw instances, then draw child objects
	for (size_t i = 0; i < drawInstances.size(); i++)
		drawInstances[i]->Draw();
	for (size_t i = 0; i < children.size(); i++)
		children[i]->Draw();
}

//Object drawing instance class
OBJECT_DRAWINSTANCE::OBJECT_DRAWINSTANCE(std::deque<OBJECT_DRAWINSTANCE*> *linkedList)
{
	//Clear memory
	memset(this, 0, sizeof(OBJECT_DRAWINSTANCE));
	
	//Attach to linked list (if applicable)
	if (linkedList != nullptr)
		linkedList->push_back(this);
}

OBJECT_DRAWINSTANCE::~OBJECT_DRAWINSTANCE()
{
	return;
}

void OBJECT_DRAWINSTANCE::Draw()
{
	//Don't draw if we don't have textures or mappings
	if (texture != nullptr && mappings != nullptr)
	{
		//Draw our sprite
		RECT *mapRect = &mappings->rect[mappingFrame];
		POINT *mapOrig = &mappings->origin[mappingFrame];
		
		int origX = mapOrig->x;
		int origY = mapOrig->y;
		if (renderFlags.xFlip)
			origX = mapRect->w - origX;
		if (renderFlags.yFlip)
			origY = mapRect->h - origY;
		
		//Draw to screen at the given position
		int alignX = renderFlags.alignPlane ? gLevel->camera->x : 0;
		int alignY = renderFlags.alignPlane ? gLevel->camera->y : 0;
		gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, mapRect, gLevel->GetObjectLayer(highPriority, priority), xPos - origX - alignX, yPos - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
	}
}
