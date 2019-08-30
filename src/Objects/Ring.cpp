#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"

void ObjRing(OBJECT *object)
{
	switch (object->routine)
	{
		case 0: //Initialization
		{
			//Advance routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Ring.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Ring.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 8;
			object->priority = 2;
		}
	//Fallthrough
		case 1: //Waiting for contact, just animate
			object->mappingFrame = (gLevel->frameCounter >> 3) & 0x3;
			object->Draw();
			break;
	}
}

void ObjRingSpawner(OBJECT *object)
{
	int16_t xPos = object->x.pos;
	int16_t yPos = object->y.pos;
	
	for (int i = 0; i <= (object->subtype & 0xF); i++)
	{
		OBJECT *newObject = new OBJECT(&gLevel->objectList, &ObjRing);
		newObject->x.pos = xPos;
		newObject->y.pos = yPos;
		
		switch (object->subtype & 0xF0)
		{
			case 0x00:
				xPos += 0x10;
				break;
			case 0x10:
				xPos += 0x18;
				break;
			case 0x20:
				xPos += 0x20;
				break;
			case 0x30:
				yPos += 0x10;
				break;
			case 0x40:
				yPos += 0x18;
				break;
			case 0x50:
				yPos += 0x20;
				break;
			case 0x60:
				xPos += 0x10;
				yPos += 0x10;
				break;
			case 0x70:
				xPos += 0x18;
				yPos += 0x18;
				break;
			case 0x80:
				xPos += 0x20;
				yPos += 0x20;
				break;
			case 0x90:
				xPos -= 0x10;
				yPos += 0x10;
				break;
			case 0xA0:
				xPos -= 0x18;
				yPos += 0x18;
				break;
			case 0xB0:
				xPos -= 0x20;
				yPos += 0x20;
				break;
			case 0xC0:
				xPos += 0x10;
				yPos += 0x08;
				break;
			case 0xD0:
				xPos += 0x18;
				yPos += 0x0C;
				break;
			case 0xE0:
				xPos -= 0x10;
				yPos += 0x08;
				break;
			case 0xF0:
				xPos -= 0x18;
				yPos += 0x0C;
				break;
		}
	}
	
	//Delete
	object->deleteFlag = true;
}