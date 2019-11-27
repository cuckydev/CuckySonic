#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../MathUtil.h"
#include "../Log.h"

const int8_t ledgeSlope[] = {
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,
	 3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,
	 9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14,
	15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

void ObjGHZLedge(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//U8
		SCRATCHU8_FLAG =	0,
		SCRATCHU8_DELAY =	1,
		SCRATCHU8_MAX =		2,
	};
	
	object->ScratchAllocU8(SCRATCHU8_MAX);
	
	switch (object->routine)
	{
		case 0:
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZLedge.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->renderFlags.xFlip = object->status.xFlip;
			object->widthPixels = 100;
			object->heightPixels = 56;
			object->mappingFrame = object->subtype;
			object->priority = 4;
	//Fallthrough
		case 1:
		{
			if (object->scratchU8[SCRATCHU8_FLAG] == 0 || (object->scratchU8[SCRATCHU8_DELAY] != 0 && object->scratchU8[SCRATCHU8_DELAY]-- != 0))
			{
				//Draw and act as solid
				object->SolidObjectTop(48, 32, object->x.pos, false, ledgeSlope);
				object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
				object->UnloadOffscreen(object->x.pos);
				break;
			}
		}
	//Fallthrough (potentially)
		case 2:
		{
			
		}
	}
}
