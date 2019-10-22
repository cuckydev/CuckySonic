#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

int ObjGHZEdgeWall_Solid2(PLAYER *player, OBJECT *object, int16_t width, int16_t height, int16_t *retXDiff, int16_t *retYDiff)
{
	//Get our position differences and return 0 if out of range
	int16_t xDiff = (player->x.pos - object->x.pos) + width;
	if (xDiff < 0 || xDiff > (width * 2))
		return 0;
	
	int16_t yDiff = (player->y.pos - object->y.pos) + (height += player->yRadius);
	if (yDiff < 0 || yDiff > (height * 2))
		return 0;
	
	//Check if player is tangible
	if (player->routine == PLAYERROUTINE_DEATH || player->debug)
		return 0;
	
	//Get our clip values
	int16_t xClip = xDiff;
	if (xDiff >= width)
	{
		xDiff -= width * 2;
		xClip = -xDiff;
	}
	
	int16_t yClip = yDiff;
	if (yDiff >= height)
	{
		yDiff -= height * 2;
		yClip = -yDiff;
	}
	
	//Copy our clip results
	*retXDiff = xDiff;
	*retYDiff = yDiff;
	
	if (xClip > yClip)
		return -1;
	else
		return 1;
}

void ObjGHZEdgeWall_Solid_Individual(PLAYER *player, int i, OBJECT *object, int16_t width, int16_t height)
{
	//Check for collisions
	int16_t xDiff, yDiff;
	int solidResult = ObjGHZEdgeWall_Solid2(player, object, width, height, &xDiff, &yDiff);
	
	if (solidResult > 0)
	{
		//Side collision
		//Hault our velocity if running into sides
		if (xDiff > 0)
		{
			if (player->xVel >= 0)
			{
				player->x.pos -= xDiff;
				player->inertia = 0;
				player->xVel = 0;
			}
		}
		else if (xDiff < 0)
		{
			if (player->xVel < 0)
			{
				player->x.pos -= xDiff;
				player->inertia = 0;
				player->xVel = 0;
			}
		}
		
		//Clip out of wall and start pushing
		if (!player->status.inAir)
		{
			//On ground, set pushing
			object->playerContact[i].pushing = true;
			player->status.pushing = true;
		}
		else
		{
			//In mid-air, clear pushing
			object->playerContact[i].pushing = false;
			player->status.pushing = false;
		}
	}
	else if (solidResult < 0)
	{
		//Bottom collision
		if (player->yVel >= 0 || yDiff >= 0)
			return;
		
		//Clip out of bottom
		player->y.pos -= yDiff;
		player->yVel = 0;
	}
	else
	{
		//No collision, clear pushing flags
		if (object->playerContact[i].pushing)
		{
			if (player->anim != PLAYERANIMATION_ROLL)
				player->anim = PLAYERANIMATION_RUN; //wrong animation again
			object->playerContact[i].pushing = false;
			player->status.pushing = false;
		}
	}
}

void ObjGHZEdgeWall_Solid(OBJECT *object, int16_t width, int16_t height)
{
	for (size_t i = 0; i < gLevel->playerList.size(); i++)
	{
		PLAYER *player = &gLevel->playerList[i];
		ObjGHZEdgeWall_Solid_Individual(player, i, object, width, height); //Handle collision
	}
}

void ObjGHZEdgeWall(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/GHZEdgeWall.map");
			
			//Set other render properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 8;
			object->priority = 6;
			object->mappingFrame = object->subtype & (~0x10);
		}
	//Fallthrough
		case 1:
		{
			//Handle collisions if bit 4 isn't set
			if (!(object->subtype & 0x10))
				ObjGHZEdgeWall_Solid(object, 19, 40);
			
			//Draw
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
	}
}
