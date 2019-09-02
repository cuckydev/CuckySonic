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
	player->chainPointCounter += 2;
	
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
			int16_t xDiff = player->x.pos - x.pos + width;
			
			if (!player->status.inAir && xDiff >= 0 && xDiff < width * 2)
			{
				//Move on the platform
				player->MoveOnPlatform((void*)this, height, lastXPos);
			}
			else
			{
				//We've jumped or fell off the platform
				ExitPlatform(player, i);
			}
		}
		else
		{
			//Check if we're going to land on the platform
			LandOnPlatform(player, i, width, height);
		}
		
		//Check next player
		i++;
	}
}

void OBJECT::LandOnPlatform(PLAYER *player, int i, int16_t width, int16_t height)
{
	//Check if we're in a state where we can enter onto the platform
	int16_t xDiff = (player->x.pos - x.pos) + width;
	if (player->yVel < 0 || xDiff < 0 || xDiff >= width * 2)
		return;
	
	//Land on platform
	int16_t playerBottom = (player->y.pos + player->yRadius) + 4;
	int16_t yThing = (y.pos - height) - playerBottom;
	
	//If we're on top of the platform, and not in an intangible state
	if (yThing > 0 || yThing < -16 || player->objectControl.disableObjectInteract || player->routine == PLAYERROUTINE_DEATH)
		return;
	
	//Land on top of the platform
	player->y.pos += yThing + 3;
	player->AttachToObject((void*)this, (&playerContact[i].standing - (size_t)this));
}

void OBJECT::ExitPlatform(PLAYER *player, int i)
{
	player->status.shouldNotFall = false;
	player->status.inAir = true;
	playerContact[i].standing = false;
}

//Update and drawing objects
bool OBJECT::Update()
{
	//Clear drawing flag
	isDrawing = false;
	
	//Run our object code
	if (function != NULL)
		function(this);
	
	//If there was an error RETURN TREUE NOOOOO
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
		texture->Draw((highPriority ? LEVEL_RENDERLAYER_OBJECT_HIGH_0 : LEVEL_RENDERLAYER_OBJECT_0) + ((OBJECT_LAYERS - 1) - priority), texture->loadedPalette, mapRect, x.pos - origX - alignX, y.pos - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
	}
}

void OBJECT::DrawRecursive()
{
	if (isDrawing)
		DrawToScreen();
	for (OBJECT *object = children; object != NULL; object = object->next)
		object->DrawRecursive();
}