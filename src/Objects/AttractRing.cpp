#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../Audio.h"
#include "../Objects.h"

static const uint8_t animationCollect[] =	{0x05,0x04,0x05,0x06,0x07,0xFC};

static const uint8_t *animationList[] = {
	animationCollect,
};

void ObjAttractRing(OBJECT *object)
{
	const PLAYER *player = (PLAYER*)object->parent;
	
	switch (object->routine)
	{
		case 0: //Initialization
		{
			//Advance routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Ring.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Ring.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 8;
			object->priority = 2;
			
			//Collision box
			object->collisionType = COLLISIONTYPE_OTHER;
			object->touchWidth = 6;
			object->touchHeight = 6;
		}
	//Fallthrough
		case 1: //Move towards the player
		{
			//If lightning shield, turn into a bouncing ring
			if (player->shield != SHIELD_ELECTRIC)
			{
				object->function = ObjBouncingRing;
				object->scratchU8[0] = 0xFF; //Ring_spill_anim_counter
				object->scratchU16[1] = 0x00; //Ring_spill_anim_accum
				object->xRadius = 8;
				object->yRadius = 8;
				object->Draw();
			}
			
			//Horizontal pull
			int16_t pullX = 0x30;
			
			if (object->x.pos >= player->x.pos)
			{
				pullX = -pullX;
				if (object->xVel >= 0)
					pullX *= 2;
			}
			else
			{
				if (object->xVel < 0)
					pullX *= 2;
			}
			
			object->xVel += pullX;
			
			//Vertical pull
			int16_t pullY = 0x30;
			
			if (object->y.pos >= player->y.pos)
			{
				pullY = -pullY;
				if (object->yVel >= 0)
					pullY *= 2;
			}
			else
			{
				if (object->yVel < 0)
					pullY *= 2;
			}
			
			object->yVel += pullY;
			
			//Move and draw to the screen
			object->Move();
			
			object->mappingFrame = (gLevel->frameCounter >> 3) & 0x3;
			object->Draw();
			break;
		}
		case 2: //Touched player, collect a ring
			//Increment routine
			object->routine++;
			
			//Clear collision and change priority
			object->collisionType = COLLISIONTYPE_NULL;
			object->priority = 1;
			
			//Collect the ring
			AddToRings(1);
	//Fallthrough
		case 3: //Sparkling
			object->Animate(animationList);
			object->Draw();
			break;
		case 4: //Deleting after sparkle
			object->deleteFlag = true;
			break;
	}
}
