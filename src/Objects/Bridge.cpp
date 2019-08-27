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
		switch (gLevel->levelId)
		{
			case 0: //Green Hill Zone
				object->texture = gLevel->GetObjectTexture("data/Object/GHZBridge.bmp");
				break;
			case 1: //Emerald Hill Zone
				object->texture = gLevel->GetObjectTexture("data/Object/EHZBridge.bmp");
				break;
		}
		object->mappings = new MAPPINGS("data/Object/Bridge.map");
	}
}

void ObjBridge(OBJECT *object)
{
	enum SCRATCH
	{
		SCRATCH_ORIGINY = 0,	// 2 bytes
		SCRATCH_FORCE = 0,		// 2 bytes
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
			//Get our depression
			int subtype = object->subtype;
			int16_t depress[0x100];
			
			int depressPlayers = 0;
			int depressPosition = 0;
			int depressForce = 0;
			
			int i = 0;
			for (OBJECT *child = object->children; child != NULL; child = child->next)
				child->y.pos = object->scratchU16[SCRATCH_ORIGINY] + depress[i++];
			break;
		}
	}
}
