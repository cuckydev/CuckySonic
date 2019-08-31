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
			object->renderFlags.alignBackground = false;
			object->renderFlags.assumePixelHeight = false;
			object->renderFlags.onScreen = false;
			object->priority = 1;
			
			//Clear collision
			object->collisionType = COLLISIONTYPE_ENEMY;
			object->touchWidth = 0;
			object->touchHeight = 0;
			
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
				object->animFrameDuration = 7;
				
				//Advance frame, and delete once finished
				if (++object->mappingFrame >= 5)
				{
					object->deleteFlag = true;
					break;
				}
			}
			
			//Draw
			object->Draw();
			break;
		}
	}
}
