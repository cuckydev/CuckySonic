#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../Audio.h"

void ObjExplosion(OBJECT *object)
{
	switch (object->routine)
	{
		case 0: //Explosion with an animal
		{
			//Advance routine
			object->routine++;
			
			//TODO: Create an animal
		}
	//Fallthrough
		case 1: //Explosion without an animal
		{
			//Advance routine
			object->routine++;
			
			//Play pop sound
			PlaySound(SOUNDID_POP);
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Explosion.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Explosion.map");
			
			//Initialize other properties
			object->renderFlags.xFlip = false;
			object->renderFlags.yFlip = false;
			object->renderFlags.alignPlane = true;
			object->priority = 1;
			
			//Clear collision
			object->collisionType = COLLISIONTYPE_NULL;
			
			//Other stuff
			object->widthPixels = 12;
			
			//Initialize animation
			object->animFrameDuration = 3;
			object->mappingFrame = 0;
		}
	//Fallthrough
		case 2:
		{
			//Animate
			if (object->animFrameDuration-- == 0)
			{
				object->animFrameDuration = 3;
				
				//Advance frame, and delete once finished
				if (++object->mappingFrame == 5)
				{
					object->deleteFlag = true;
					break;
				}
			}
			
			//Draw
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
	}
}
