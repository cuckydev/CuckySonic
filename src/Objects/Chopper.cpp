#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

static const uint8_t animationSlow[] =	{0x07,0x00,0x01,0xFF};
static const uint8_t animationFast[] =	{0x03,0x00,0x01,0xFF};
static const uint8_t animationStill[] =	{0x07,0x00,0xFF};

static const uint8_t *animationList[] = {
	animationSlow,
	animationFast,
	animationStill,
};

void ObjChopper(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//S16
		SCRATCHS16_ORIG_Y = 0,
		SCRATCHS16_MAX = 1,
	};
	
	object->ScratchAllocS16(SCRATCHS16_MAX);
	
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Sonic1Badnik.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Chopper.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->priority = 4;
			
			//Collision
			object->collisionType = COLLISIONTYPE_ENEMY;
			object->touchWidth = 12;
			object->touchHeight = 16;
			
			//Initialize other properties
			object->widthPixels = 16;
			object->yVel = -0x700;
			object->scratchS16[SCRATCHS16_ORIG_Y] = object->y.pos;
		}
	//Fallthrough
		case 1:
		{
			//Animate
			object->Animate(animationList);
			
			//Move and fall
			object->Move();
			object->yVel += 0x18;
			
			//Jump back up once back at the original Y position
			int16_t origY = object->scratchS16[SCRATCHS16_ORIG_Y];
			if (object->y.pos >= origY)
			{
				object->y.pos = object->scratchS16[SCRATCHS16_ORIG_Y];
				object->yVel = -0x700;
			}
			
			//Change animation
			object->anim = 1;
			
			if (object->y.pos >= origY - 192)
			{
				if (object->yVel < 0)
					object->anim = 1;
				else
					object->anim = 2;
			}
			
			//Draw
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
	}
}
