#include "../Game.h"
#include "../Audio.h"
#include "../Objects.h"

static const uint8_t animationCollect[] =	{0x05,0x04,0x05,0x06,0x07,ANICOMMAND_ADVANCE_ROUTINE};

static const uint8_t *animationList[] = {
	animationCollect,
};

void ObjAttractRing(OBJECT *object)
{
	switch (object->routine)
	{
		case 0: //Initialization
		{
			//Advance routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Generic.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/Ring.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 8;
			object->heightPixels = 8;
			object->priority = 2;
			
			//Collision box
			object->collisionType = COLLISIONTYPE_OTHER;
			object->touchWidth = 6;
			object->touchHeight = 6;
		}
	//Fallthrough
		case 1: //Move towards the player
		{
			//If player lost the lightning barrier, turn into a bouncing ring
			if (object->parentPlayer->barrier != BARRIER_LIGHTNING)
			{
				object->function = ObjBouncingRing;
				object->routine = 0;
				object->xRadius = 8;
				object->yRadius = 8;
			}
			
			//Horizontal pull
			int16_t pullX = 0x30;
			
			if (object->x.pos >= object->parentPlayer->x.pos)
			{
				pullX = -pullX;
				if (object->xVel >= 0)
					pullX *= 4;
			}
			else
			{
				if (object->xVel < 0)
					pullX *= 4;
			}
			
			object->xVel += pullX;
			
			//Vertical pull
			int16_t pullY = 0x30;
			
			if (object->y.pos >= object->parentPlayer->y.pos)
			{
				pullY = -pullY;
				if (object->yVel >= 0)
					pullY *= 4;
			}
			else
			{
				if (object->yVel < 0)
					pullY *= 4;
			}
			
			object->yVel += pullY;
			
			//Move and draw to the screen
			object->Move();
			
			object->mappingFrame = (gLevel->frameCounter >> 3) & 0x3;
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
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
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		case 4: //Deleting after sparkle
			object->deleteFlag = true;
			break;
	}
}
