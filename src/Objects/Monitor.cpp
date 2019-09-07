#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

void ObjMonitor(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine and set collision size
			object->routine++;
			object->xRadius = 14;
			object->yRadius = 14;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Monitor.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Monitor.map");
			
			//Set render properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			object->widthPixels = 15;
		}
	//Fallthrough
		case 1:
		{
			object->SolidObject(26, 16, object->x.pos);
			object->Draw();
			break;
		}
	}
}
