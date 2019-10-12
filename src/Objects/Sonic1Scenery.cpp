#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../Audio.h"

void ObjSonic1Scenery(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Advance routine
			object->routine++;
			
			switch (object->subtype)
			{
				case 3:
					//Load graphics
					object->mappingFrame = 1;
					object->texture = gLevel->GetObjectTexture("data/Object/GHZBridge.bmp");
					object->mappings = gLevel->GetObjectMappings("data/Object/Bridge.map");
					object->widthPixels = 16;
					object->priority = 1;
					break;
			}
			
			//Initialize other properties
			object->renderFlags.xFlip = object->status.xFlip;
			object->renderFlags.yFlip = object->status.yFlip;
			object->renderFlags.alignPlane = true;
		}
	//Fallthrough
		case 1:
		{
			object->Draw();
			break;
		}
	}
}
