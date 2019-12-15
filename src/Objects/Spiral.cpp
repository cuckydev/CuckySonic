#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../MathUtil.h"

//#define SPIRAL_OFFSET_FIX //Fixes the radius offset to scale properly

static const uint8_t FlipAngleTable[] = {
	0x00,0x00,
	0x01,0x01,0x16,0x16,0x16,0x16,0x2C,0x2C,
	0x2C,0x2C,0x42,0x42,0x42,0x42,0x58,0x58,
	0x58,0x58,0x6E,0x6E,0x6E,0x6E,0x84,0x84,
	0x84,0x84,0x9A,0x9A,0x9A,0x9A,0xB0,0xB0,
	0xB0,0xB0,0xC6,0xC6,0xC6,0xC6,0xDC,0xDC,
	0xDC,0xDC,0xF2,0xF2,0xF2,0xF2,0x01,0x01,
	0x00,0x00,
};

static const int8_t CosineTable[] = {
	32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 32, 32, 32, 32,

	32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 32, 32, 31, 31,
	31, 31, 31, 31, 31, 31, 31, 31,
	31, 31, 31, 31, 31, 30, 30, 30,

	30, 30, 30, 30, 30, 30, 29, 29,
	29, 29, 29, 28, 28, 28, 28, 27,
	27, 27, 27, 26, 26, 26, 25, 25,
	25, 24, 24, 24, 23, 23, 22, 22,

	21, 21, 20, 20, 19, 18, 18, 17,
	16, 16, 15, 14, 14, 13, 12, 12,
	11, 10, 10,  9,  8,  8,  7,  6,
	 6,  5,  4,  4,  3,  2,  2,  1,

	 0, -1, -2, -2, -3, -4, -4, -5,
	-6, -7, -7, -8, -9, -9,-10,-10,
	-11,-11,-12,-12,-13,-14,-14,-15,
	-15,-16,-16,-17,-17,-18,-18,-19,

	-19,-19,-20,-21,-21,-22,-22,-23,
	-23,-24,-24,-25,-25,-26,-26,-27,
	-27,-28,-28,-28,-29,-29,-30,-30,
	-30,-31,-31,-31,-32,-32,-32,-33,

	-33,-33,-33,-34,-34,-34,-35,-35,
	-35,-35,-35,-35,-35,-35,-36,-36,
	-36,-36,-36,-36,-36,-36,-36,-37,
	-37,-37,-37,-37,-37,-37,-37,-37,

	-37,-37,-37,-37,-37,-37,-37,-37,
	-37,-37,-37,-37,-37,-37,-37,-37,
	-37,-37,-37,-37,-36,-36,-36,-36,
	-36,-36,-36,-35,-35,-35,-35,-35,

	-35,-35,-35,-34,-34,-34,-33,-33,
	-33,-33,-32,-32,-32,-31,-31,-31,
	-30,-30,-30,-29,-29,-28,-28,-28,
	-27,-27,-26,-26,-25,-25,-24,-24,

	-23,-23,-22,-22,-21,-21,-20,-19,
	-19,-18,-18,-17,-16,-16,-15,-14,
	-14,-13,-12,-11,-11,-10, -9, -8,
	-7, -7, -6, -5, -4, -3, -2, -1,

	 0,  1,  2,  3,  4,  5,  6,  7,
	 8,  8,  9, 10, 10, 11, 12, 13,
	13, 14, 14, 15, 15, 16, 16, 17,
	17, 18, 18, 19, 19, 20, 20, 21,

	21, 22, 22, 23, 23, 24, 24, 24,
	25, 25, 25, 25, 26, 26, 26, 26,
	27, 27, 27, 27, 28, 28, 28, 28,
	28, 28, 29, 29, 29, 29, 29, 29,

	29, 30, 30, 30, 30, 30, 30, 30,
	31, 31, 31, 31, 31, 31, 31, 31,
	31, 31, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 32, 32, 32, 32,

	32, 32, 32, 32, 32, 32, 32, 32,
	32, 32, 32, 32, 32, 32, 32, 32,
};

void ObjSpiral(OBJECT *object) //Also MTZ cylinder
{
	//Set our routine based off of our subtype
	if (object->routine == 0)
	{
		object->routine = (object->subtype < 0x80) ? 1 : 2;
		object->widthPixels = 208;
	}
	
	switch (object->routine)
	{
		case 1: //EHZ Spiral
		{
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player
				PLAYER *player = gLevel->playerList[i];
				
				if (object->playerContact[i].standing == false) //Not already on the spiral
				{
					if (player->status.inAir) //Don't run on corkscrew if in mid-air
						continue;
					
					//Check if we're at the sides of the spiral
					if (!player->status.shouldNotFall) //If not standing on an object (already on a spiral)
					{
						//Check if we're at the sides of the spiral
						int16_t xOff = player->x.pos - object->x.pos;
						if (player->xVel >= 0) //Moving in from the left
						{
							if (xOff < -0xD0 || xOff > -0xC0)
								continue;
						}
						else //Moving in from the right
						{
							if (xOff < 0xC0 || xOff > 0xD0)
								continue;
						}
					}
					else
					{
						//Check if we're at the sides of the spiral
						int16_t xOff = player->x.pos - object->x.pos;
						if (player->xVel >= 0) //Moving in from the left
						{
							if (xOff < -0xC0 || xOff > -0xB0)
								continue;
						}
						else //Moving in from the right
						{
							if (xOff < 0xB0 || xOff > 0xC0)
								continue;
						}
					}
					
					//Check if we're near the bottom and not already on an object controlling us
					int16_t yOff = player->y.pos - object->y.pos - 0x10;
					if (yOff < 0 || yOff >= 0x30 || player->objectControl.disableOurMovement)
						continue;
					
					//Set the player to run on the spiral
					object->AttachPlayer(player, i);
				}
				else
				{
					//Running on the corkscrew
					if (!(abs(player->inertia) < 0x600 || player->status.inAir)) //If not slowed down or jumped off
					{
						int16_t xOff = player->x.pos - object->x.pos + 0xD0;
						if (xOff >= 0 && xOff < 0x1A0) //If still on the spiral
						{
							//Move across the spiral
							if (player->status.shouldNotFall) //If still running on the spiral
							{
								//Set our Y-position
								int8_t cosine = CosineTable[xOff];
								
								#ifndef SPIRAL_OFFSET_FIX
									int16_t yOff = player->yRadius - 19;
								#else
									int16_t yOff = ((player->yRadius - 19) * cosine) / (cosine < 0 ? 37 : 32);
								#endif
								
								player->y.pos = (object->y.pos + cosine) - yOff;
								
								//Set our flip angle
								player->flipAngle = FlipAngleTable[(xOff / 8) & 0x3F];
							}
							
							continue;
						}
					}
					
					//Fall off
					player->status.shouldNotFall = false;
					object->playerContact[i].standing = false;
					player->flipsRemaining = false;
					player->flipSpeed = 4;
				}
			}
			break;
		}
		case 2: //MTZ Cylinder
		{
			//None
			break;
		}
	}
	
	object->UnloadOffscreen(object->x.pos);
}
