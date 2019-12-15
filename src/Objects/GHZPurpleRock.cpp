#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

void ObjGHZPurpleRock(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZPurpleRock.map");
			
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
			object->SolidObjectFull(27, 16, 16, object->x.pos, false, nullptr, false);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
