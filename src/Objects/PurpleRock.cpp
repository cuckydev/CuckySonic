#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

void ObjPurpleRock(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/PurpleRock.map");
			
			//Set render properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 19;
			object->heightPixels = 32;
			object->priority = 4;
		}
	//Fallthrough
		case 1:
		{
			//Act as solid and draw to screen
			object->SolidObject(27, 16, 16, object->x.pos, false, nullptr, false);
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
