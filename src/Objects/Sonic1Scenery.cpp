#include "../Game.h"
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
					object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
					object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZBridge.map");
					object->widthPixels = 16;
					object->heightPixels = 32;
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
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
