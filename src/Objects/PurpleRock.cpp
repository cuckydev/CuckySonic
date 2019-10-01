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
			object->priority = 4;
		}
	//Fallthrough
		case 1:
		{
			//Act as solid and draw to screen
			object->SolidObject(27, 15, 16, object->x.pos);
			object->Draw();
			break;
		}
	}
}
