#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../Audio.h"

static const uint8_t animationCollect[] =	{0x05,0x04,0x05,0x06,0x07,0xFC};

static const uint8_t *animationList[] = {
	animationCollect,
};

void ObjRing(OBJECT *object)
{
	switch (object->routine)
	{
		case 0: //Initialization
		{
			//Advance routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Generic.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/Ring.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 8;
			object->heightPixels = 8;
			object->priority = 2;
			
			//Collision box
			object->collisionType = COLLISIONTYPE_OTHER;
			object->touchWidth = 6;
			object->touchHeight = 6;
		}
	//Fallthrough
		case 1: //Waiting for contact, just animate
			object->mappingFrame = (gLevel->frameCounter >> 3) & 0x3;
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(object->x.pos);
			break;
		case 2: //Touched player, collect a ring
			//Increment routine
			object->routine++;
			
			//Clear collision and change priority
			object->collisionType = COLLISIONTYPE_NULL;
			object->priority = 1;
			
			//Collect the ring
			AddToRings(1);
	//Fallthrough
		case 3: //Sparkling
			object->Animate(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		case 4: //Deleting after sparkle
			object->deleteFlag = true;
			gLevel->ReleaseObjectLoad(object);
			break;
	}
}

//Used for Sonic 1 levels
static const int8_t posData[16][2] = {
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
	int ringsToMake = (object->subtype & 0x7);
	if (ringsToMake == 7)
		ringsToMake = 6;
	
	for (int i = 0; i <= ringsToMake; i++)
	{
		//Create ring object
		OBJECT *newObject = new OBJECT(&ObjRing);
		newObject->x.pos = xPos;
		newObject->y.pos = yPos;
		gLevel->objectList.link_back(newObject);
		
		gLevel->LinkObjectLoad(newObject);
		
		//Get next position
		xPos += posData[object->subtype >> 4][0];
		yPos += posData[object->subtype >> 4][1];
	}
	
	//Delete us
	gLevel->ReleaseObjectLoad(object);
	object->deleteFlag = true;
}