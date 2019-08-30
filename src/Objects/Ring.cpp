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

//Used for Sonic 1 levels
static int16_t posData[16][2] = {
	{ 0x10, 0x00},
	{ 0x18, 0x00},
	{ 0x20, 0x00},
	{ 0x00, 0x10},
	{ 0x00, 0x18},
	{ 0x00, 0x20},
	{ 0x10, 0x10},
	{ 0x18, 0x18},
	{ 0x20, 0x20},
	{-0x10, 0x10},
	{-0x18, 0x18},
	{-0x20, 0x20},
	{ 0x10, 0x08},
	{ 0x18, 0x10},
	{-0x10, 0x08},
	{-0x18, 0x10},
};
		
void ObjRingSpawner(OBJECT *object)
{
	int16_t xPos = object->x.pos;
	int16_t yPos = object->y.pos;
	
	//Create rings (lowest nibble of subtype)
	for (int i = 0; i <= (object->subtype & 0xF); i++)
	{
		//Create ring object
		OBJECT *newObject = new OBJECT(&gLevel->objectList, &ObjRing);
		newObject->x.pos = xPos;
		newObject->y.pos = yPos;
		
		//Get next position
		xPos += posData[(object->subtype & 0xF0) >> 4][0];
		yPos += posData[(object->subtype & 0xF0) >> 4][1];
	}
	
	//Delete us
	object->deleteFlag = true;
}