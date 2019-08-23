#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../MathUtil.h"

void ObjBridgeLogSegment(OBJECT *object)
{
	if (object->routine == 0)
	{
		//Set our properties
		object->routine++;
		object->renderFlags.alignPlane = true;
		object->widthPixels = 0x40;
		object->priority = 3;
		
		//Load graphics
		object->texture = gLevel->GetObjectTexture("data/Object/LogBridge.bmp");
		object->mappings = new MAPPINGS("data/Object/LogBridge.map");
	}
}

void ObjBridge(OBJECT *object)
{
	enum SCRATCH
	{
		SCRATCH_ORIGINY = 0,	// 2 bytes
	};
	
	switch (object->routine)
	{
		case 0:
		{
			//Set our properties
			object->routine++;
			object->priority = 3;
			object->renderFlags.alignPlane = true;
			object->widthPixels = 0x80;
			
			//Get our type properties
			int16_t yPos = object->y.pos;
			object->scratchU16[SCRATCH_ORIGINY] = yPos;
			int16_t xPos = object->x.pos;
			
			int *subtypePointer = &object->subtype;
			int subtype = *subtypePointer;
			int bridgeRadius = (subtype / 2) * 16;
			int16_t bridgeLeft = object->x.pos - bridgeRadius;
			
			//Create our log segments
			for (int i = 0; i < subtype; i++)
			{
				OBJECT *newSegment = new OBJECT(&object->children, &ObjBridgeLogSegment);
				newSegment->parent = (void*)object;
				newSegment->x.pos = bridgeLeft + 16 * i;
				newSegment->y.pos = object->scratchU16[SCRATCH_ORIGINY];
			}
		}
//Fallthrough
		case 1:
		{
			
			break;
		}
		case 2:
		{
			
			break;
		}
	}
}
