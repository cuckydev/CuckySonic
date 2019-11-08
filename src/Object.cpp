#include <stdlib.h>
#include <string.h>

#include "Object.h"
#include "Objects.h"

#include "CommonMacros.h"
#include "Game.h"
#include "Log.h"
#include "Error.h"
#include "MathUtil.h"
#include "Player.h"

//Object class
OBJECT::OBJECT(OBJECTFUNCTION objectFunction)
{
	//Reset memory
	memset(this, 0, sizeof(OBJECT));
	
	//Reset children and draw instance linked lists
	drawInstances = LINKEDLIST<OBJECT_DRAWINSTANCE*>();
	children = LINKEDLIST<OBJECT*>();
	
	//Set function
	function = objectFunction;
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
	
	//Destroy draw instances and children
	CLEAR_INSTANCE_LINKEDLIST(drawInstances);
	CLEAR_INSTANCE_LINKEDLIST(children);
}

//Scratch allocation functions
#define SCRATCH_ALLOC(name, type, max) if (name == nullptr) { name = (type*)malloc(max * sizeof(type)); memset(name, 0, max * sizeof(type)); }

void  OBJECT::ScratchAllocU8(size_t max) { SCRATCH_ALLOC(scratchU8,   uint8_t, max) }
void  OBJECT::ScratchAllocS8(size_t max) { SCRATCH_ALLOC(scratchS8,    int8_t, max) }
void OBJECT::ScratchAllocU16(size_t max) { SCRATCH_ALLOC(scratchU16, uint16_t, max) }
void OBJECT::ScratchAllocS16(size_t max) { SCRATCH_ALLOC(scratchS16,  int16_t, max) }
void OBJECT::ScratchAllocU32(size_t max) { SCRATCH_ALLOC(scratchU32, uint32_t, max) }
void OBJECT::ScratchAllocS32(size_t max) { SCRATCH_ALLOC(scratchS32,  int32_t, max) }

//Generic object functions
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
				animFrame++;
				return;
			case 0xFB:	//Reset animation and clear secondary routine
				animFrame = 0;
				routineSecondary = 0;
				return;
			case 0xFA:	//Increment secondary routine
				routineSecondary++;
				return;
		}
	}
	
	//Set our mapping frame and flip
	mappingFrame = animation[1 + animFrame] & 0x7F;
	renderFlags.xFlip = status.xFlip;
	renderFlags.yFlip = status.yFlip;
	animFrame++;
}

void OBJECT::Animate_S1(const uint8_t **animationList)
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
				return;
			case 0xFB:	//Reset animation and clear secondary routine
				animFrame = 0;
				routineSecondary = 0;
				return;
			case 0xFA:	//Increment secondary routine
				routineSecondary++;
				return;
		}
	}
	
	//Set our mapping frame and flip
	uint8_t frame = animation[1 + animFrame];
	mappingFrame = frame & 0x1F;
	renderFlags.xFlip = status.xFlip ^ ((frame & 0x20) != 0);
	renderFlags.yFlip = status.yFlip ^ ((frame & 0x40) != 0);
	animFrame++;
}

int16_t OBJECT::CheckFloorEdge(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle)
{
	int16_t distance = FindFloor(xPos, yPos + yRadius, layer, false, outAngle);
	if (outAngle != nullptr)
		*outAngle = ((*outAngle) & 1) ? 0 : (*outAngle);
	return distance;
}

void OBJECT::DrawInstance(OBJECT_RENDERFLAGS iRenderFlags, TEXTURE *iTexture, MAPPINGS *iMappings, bool iHighPriority, uint8_t iPriority, uint16_t iMappingFrame, int16_t iXPos, int16_t iYPos)
{
	//Create a draw instance with the properties given
	OBJECT_DRAWINSTANCE *newInstance = new OBJECT_DRAWINSTANCE();
	newInstance->renderFlags = iRenderFlags;
	newInstance->texture = iTexture;
	newInstance->mappings = iMappings;
	newInstance->highPriority = iHighPriority;
	newInstance->priority = iPriority;
	newInstance->mappingFrame = iMappingFrame;
	newInstance->xPos = iXPos;
	newInstance->yPos = iYPos;
	drawInstances.link_back(newInstance);
}

void OBJECT::UnloadOffscreen(int16_t xPos)
{
	//Check if we're within loaded range
	uint16_t xOff = (xPos & 0xFF80) - ((gLevel->camera->x - 0x80) & 0xFF80);
	if (xOff > upperRound(0x80 + gRenderSpec.width + 0x80, 0x80))
		deleteFlag = true;
}

//Object interaction functions
const uint16_t enemyPoints[] = {100, 200, 500, 1000};

bool OBJECT::Hurt(PLAYER *player)
{
	//Release if set to release when destroyed
	if (status.releaseDestroyed)
		gLevel->ReleaseObjectLoad(this);
	
	//Handle chain point bonus
	uint16_t lastCounter = player->chainPointCounter;
	player->chainPointCounter++;
	
	//Cap our actual bonus at 3
	if (lastCounter >= 3)
		lastCounter = 3;
	subtype = lastCounter;
	
	//Get our bonus points
	uint16_t bonus = enemyPoints[lastCounter];
	
	//If destroyed 16 enemies, give us 10000 points
	if (player->chainPointCounter >= 16)
	{
		bonus = 10000;
		subtype = 5;
	}
	
	//Give us the points bonus
	AddToScore(bonus);
	
	//Turn us into an explosion
	function = &ObjExplosion;
	routine = 0; //Initialize routine, spawn score and animal
	
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

void OBJECT::ClearSolidContact()
{
	//Check all players and clear all contact
	for (size_t i = 0; i < gLevel->playerList.size(); i++)
	{
		//Get the player
		PLAYER *player = gLevel->playerList[i];
		
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
	}
}


//Object collision functions
void OBJECT::PlatformObject(int16_t width, int16_t height, int16_t lastXPos, bool setAirOnExit, const int8_t *slope)
{
	for (size_t i = 0; i < gLevel->playerList.size(); i++)
	{
		//Get the player
		PLAYER *player = gLevel->playerList[i];
		
		//If the player is already standing on us
		if (playerContact[i].standing == true)
		{
			//Check if we're still on the platform
			int16_t xDiff = player->x.pos - lastXPos + width;
			
			if (!player->status.inAir && xDiff >= 0 && xDiff < width * 2)
				player->MoveOnPlatform(this, width, height, lastXPos, slope, false);
			else
				ExitPlatform(player, i, setAirOnExit);
		}
		else
		{
			//Check if we're going to land on the platform
			LandOnPlatform(player, i, width, width * 2, height, lastXPos, slope);
		}
	}
}

bool OBJECT::LandOnPlatform(PLAYER *player, size_t i, int16_t width1, int16_t width2, int16_t height, int16_t lastXPos, const int8_t *slope)
{
	//Check if we're in a state where we can enter onto the platform
	int16_t xDiff = (player->x.pos - lastXPos) + width1;
	if (player->yVel < 0 || xDiff < 0 || xDiff >= width2)
		return false;
	
	//Handle slope
	if (slope != nullptr)
	{
		//Flip xDiff if xFlip (of renderflags for some reason) is set and get our slope
		if (renderFlags.xFlip)
			xDiff = (~xDiff) + width2;
		height += slope[xDiff];
	}
	
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
		player->AttachToObject(this, i);
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
		player->AttachToObject(this, i);
		return true;
	}
}

void OBJECT::ExitPlatform(PLAYER *player, size_t i, bool setAirOnExit)
{
	player->status.shouldNotFall = false;
	playerContact[i].standing = false;
	if (setAirOnExit)
		player->status.inAir = true;
}

//Solid object and its variants
OBJECT_SOLIDTOUCH OBJECT::SolidObject(int16_t width, int16_t height_air, int16_t height_standing, int16_t lastXPos, bool setAirOnExit, const int8_t *slope, bool doubleSlope)
{
	//Initialize solid touch
	OBJECT_SOLIDTOUCH solidTouch;
	memset(&solidTouch, 0, sizeof(OBJECT_SOLIDTOUCH));
	
	//Check all players
	for (size_t i = 0; i < gLevel->playerList.size(); i++)
	{
		//Get the player
		PLAYER *player = gLevel->playerList[i];
		
		//Check if we're still standing on the object
		if (playerContact[i].standing)
		{
			//Check if we're to exit the top of the object
			int16_t xDiff = (player->x.pos - lastXPos) + width;
			if (player->status.inAir || xDiff < 0 || xDiff >= width * 2)
			{
				//Exit top as platform, then check next player
				ExitPlatform(player, i, setAirOnExit);
				continue;
			}
			
			//Move with the object
			player->MoveOnPlatform(this, width, height_standing, lastXPos, slope, doubleSlope);
			continue;
		}
		else
		{
			//Check for collisions, then check next player
			SolidObjectCont(&solidTouch, player, i, width, height_air, lastXPos, slope, doubleSlope);
			continue;
		}
	}
	
	return solidTouch;
}

void OBJECT::SolidObjectCont(OBJECT_SOLIDTOUCH *solidTouch, PLAYER *player, size_t i, int16_t width, int16_t height, int16_t lastXPos, const int8_t *slope, bool doubleSlope)
{
	//Check if we're within horizontal range
	int16_t xDiff = (player->x.pos - x.pos) + width; //d0
	if (xDiff >= 0 && xDiff <= (width * 2))
	{
		//Offset by our slope
		int8_t slopeOff = 0, slopeHOff = 0;
		
		if (slope != nullptr)
		{
			//Get our x-offset flipped
			int16_t xOff = xDiff;
			if (renderFlags.xFlip)
				xOff = (~xOff) + (width * 2);
			
			if (!doubleSlope)
			{
				//Apply our slope (single slope)
				slopeOff = slope[xOff] - *slope;
			}
			else
			{
				//Apply our slope (double slope)
				slopeOff = slope[xOff * 2] - *slope;
				slopeHOff = slope[xOff * 2];
			}
		}
		
		//Check if we're within vertical range
		int16_t yDiff; //d3
		if (player->status.reverseGravity)
			yDiff = (-(player->y.pos - (y.pos - slopeOff)) + 4) + (height += player->yRadius);
		else
			yDiff = (player->y.pos - (y.pos - slopeOff) + 4) + (height += player->yRadius);
		
		if (yDiff >= 0 && yDiff <= ((height += slopeHOff) * 2))
		{
			//Perform main collision checks if within range and not under the influence of another object
			if (!player->objectControl.disableObjectInteract)
			{
				//Check if we're intangible
				if (player->routine == PLAYERROUTINE_DEATH || player->debug)
					return;
				
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
								
								player->AttachToObject(this, i);
								
								//Set top touch flag
								if (solidTouch != nullptr)
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
							if (solidTouch != nullptr)
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
									if (solidTouch != nullptr)
										solidTouch->bottom[i] = true;
									return;
								}
								
								//Fallthrough into horizontal check
							}
							else
							{
								//Set bottom touch flag
								if (solidTouch != nullptr)
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
						if (solidTouch != nullptr)
							solidTouch->side[i] = true;
						playerContact[i].pushing = true;
						player->status.pushing = true;
						return;
					}
				}
				
				//Contact in mid-air: Set side touch and clear pushing flags
				if (solidTouch != nullptr)
					solidTouch->side[i] = true;
				playerContact[i].pushing = false;
				player->status.pushing = false;
				return;
			}
		}
	}
	
	//Clear our pushing
	SolidObjectClearPush(player, i);
}

void OBJECT::SolidObjectClearPush(PLAYER *player, size_t i)
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

//Main update and draw functions
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
	
	//Destroy draw instances from last update
	CLEAR_INSTANCE_LINKEDLIST(drawInstances);
	
	//Run our object code
	if (function != nullptr)
		function(this);
	else
		deleteFlag = true; //Delete, we're useless!
	
	//Check if any of our assets failed to load
	if (texture != nullptr && texture->fail != nullptr)
		return Error(fail = texture->fail);
	if (mappings != nullptr && mappings->fail != nullptr)
		return Error(fail = mappings->fail);
	
	//Check for regular deletion
	if (deleteFlag)
	{
		//Remove player references to us
		for (size_t i = 0; i < gLevel->playerList.size(); i++)
		{
			PLAYER *player = gLevel->playerList[i];
			if (player->interact == this)
				player->interact = nullptr;
		}
		
		//Remove object load references to us
		gLevel->UnrefObjectLoad(this);
	}
	else
	{
		//Update children's code
		for (size_t i = 0; i < children.size(); i++)
			if (children[i]->Update())
				return true;
		CHECK_LINKEDLIST_OBJECTDELETE(children);
	}
	
	return false;
}

void OBJECT::Draw()
{
	if (drawInstances.size() > 0)
	{
		//On-screen check (checks the first draw instance, which is basically how the original does it)
		int alignX = renderFlags.alignPlane ? gLevel->camera->x : 0;
		int alignY = renderFlags.alignPlane ? gLevel->camera->y : 0;
		int16_t xPos = drawInstances[0]->xPos;
		int16_t yPos = drawInstances[0]->yPos;
		
		renderFlags.isOnscreen = false;
		
		if (!(xPos - alignX < -widthPixels || xPos - alignX > gRenderSpec.width + widthPixels) &&
			!(yPos - alignY < -heightPixels || yPos - alignY > gRenderSpec.height + heightPixels))
		{
			//Draw our draw instances if on-screen and set flag
			for (size_t i = 0; i < drawInstances.size(); i++)
				drawInstances[i]->Draw();
			renderFlags.isOnscreen = true;
		}
	}
	
	for (size_t i = 0; i < children.size(); i++)
		children[i]->Draw();
}

//Object drawing instance class
OBJECT_DRAWINSTANCE::OBJECT_DRAWINSTANCE()
{
	//Reset memory
	memset(this, 0, sizeof(OBJECT_DRAWINSTANCE));
}

OBJECT_DRAWINSTANCE::~OBJECT_DRAWINSTANCE()
{
	return;
}

void OBJECT_DRAWINSTANCE::Draw()
{
	//Don't draw if we don't have textures or mappings
	if (texture != nullptr && mappings != nullptr && mappingFrame < mappings->size)
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
