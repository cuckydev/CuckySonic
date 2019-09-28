#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"

const uint16_t switchRadius[] = {0x20, 0x40, 0x80, 0x100};

void ObjPathSwitcher(OBJECT *object)
{
	//Masks
	#define MASK_RADIUS			0x03
	#define MASK_VERTICAL		0x04
	#define MASK_LU_PATH		0x10
	#define MASK_RD_PATH		0x08
	#define MASK_LU_PRIORITY	0x40
	#define MASK_RD_PRIORITY	0x20
	#define MASK_GROUND_ONLY	0x80
	
	switch (object->routine)
	{
		case 0:
		{
			//Advance to next routine
			object->routine++;
			
			//Check each player to see if they're to the right / below us
			int i = 0;
			
			for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
			{
				if (object->subtype & MASK_VERTICAL)
					object->playerContact[i].extraBit = player->y.pos >= object->y.pos;
				else
					object->playerContact[i].extraBit = player->x.pos >= object->x.pos;
				
				//Increment index
				i++;
			}
		}
//Fallthrough
		case 1:
		{
			//Check each player to see if they're to the right / below us and change their priority and path
			int i = 0;
			
			for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
			{
				//Don't check if in debug mode
				if (player->debug)
				{
					i++;
					continue;
				}
				
				//Get which side we're on
				bool newSide = object->playerContact[i].extraBit;
				if (object->subtype & MASK_VERTICAL)
				{
					if (player->y.pos > object->y.pos)
						newSide = true;
					else if (player->y.pos < object->y.pos)
						newSide = false;
				}
				else
				{
					if (player->x.pos > object->x.pos)
						newSide = true;
					else if (player->x.pos < object->x.pos)
						newSide = false;
				}
				
				//Have we changed sides
				if (newSide != object->playerContact[i].extraBit)
				{
					//Set our side, path, and priority
					object->playerContact[i].extraBit = newSide;
					
					//Check if we're grounded (ground-only?)
					if ((object->subtype & MASK_GROUND_ONLY) == 0 || !player->status.inAir)
					{
						//Check if we're within radius
						if (object->subtype & MASK_VERTICAL)
						{
							if (player->x.pos <  object->x.pos - switchRadius[object->subtype & MASK_RADIUS]
							 || player->x.pos >= object->x.pos + switchRadius[object->subtype & MASK_RADIUS])
							{
								//Skip us, we're not within the radius
								i++;
								continue;
							}
						}
						else
						{
							if (player->y.pos <  object->y.pos - switchRadius[object->subtype & MASK_RADIUS]
							 || player->y.pos >= object->y.pos + switchRadius[object->subtype & MASK_RADIUS])
							{
								//Skip us, we're not within the radius
								i++;
								continue;
							}
						}
						
						//Change our path
						if (!object->renderFlags.xFlip)
						{
							if (object->subtype & (newSide ? MASK_RD_PATH : MASK_LU_PATH))
							{
								player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
								player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
							}
							else
							{
								player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
								player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
							}
						}
						
						//Change our priority
						player->highPriority = (object->subtype & (newSide ? MASK_RD_PRIORITY : MASK_LU_PRIORITY)) != 0;
					}
				}
				
				//Increment index
				i++;
			}
			break;
		}
	}
}
