#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../Audio.h"

//Animals
enum ANIMAL
{
	ANIMAL_CUCKY = 1, //😍 (Also, set to 1 because 0 needs to be null)
	ANIMAL_FLICKY,
	ANIMAL_PECKY,
	ANIMAL_PICKY,
	ANIMAL_POCKY,
	ANIMAL_RICKY,
	ANIMAL_ROCKY,
	ANIMAL_BECKY,
	ANIMAL_LOCKY,
	ANIMAL_MICKY,
	ANIMAL_TOCKY,
	ANIMAL_WOCKY,
};

const ANIMAL animalByZone[][2] = {
	{ANIMAL_FLICKY, ANIMAL_POCKY},	//ZONEID_GHZ
	{ANIMAL_FLICKY, ANIMAL_RICKY},	//ZONEID_EHZ
};

void ObjAnimal(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
			break;
	}
}

//The score that comes up from an explosion
void ObjScore(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Score.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Score.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->priority = 1;
			object->widthPixels = 8;
			
			object->yVel = -0x300;
			object->routine++;
	//Fallthrough
		case 1:
			//Fall and delete once stopped or going down
			if (object->yVel >= 0)
			{
				object->deleteFlag = true;
				break;
			}
			
			//Move, fall, and draw to screen
			object->Move();
			object->yVel += 0x18;
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
	}
}

//Explosion from a badnik or something else
void ObjExplosion(OBJECT *object)
{
	switch (object->routine)
	{
		case 0: //Explosion with an animal
		{
			//Advance routine
			object->routine++;
			
			//Create an animal and the score
			//OBJECT *newAnimal = new OBJECT(&ObjAnimal);
			//newAnimal->x.pos = object->x.pos;
			//newAnimal->y.pos = object->y.pos;
			//gLevel->objectList.link_back(newAnimal);
			
			OBJECT *newScore = new OBJECT(&ObjScore);
			newScore->x.pos = object->x.pos;
			newScore->y.pos = object->y.pos;
			newScore->mappingFrame = object->subtype;
			gLevel->objectList.link_back(newScore);
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
				object->animFrameDuration = 7;
				
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
