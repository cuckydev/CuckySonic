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
			object->SolidObject(27, 16, (object->xPosLong - object->xVel * 0x100) / 0x10000);
			object->Draw();
			break;
		}
	}
}