#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../MathUtil.h"
#include "../Log.h"

//Scratch structure
struct SCRATCH_GHZSwingingPlatform
{
	int16_t origX = 0;
	int16_t origY = 0;
};

void ObjGHZSwingingPlatform_Move_Individual(OBJECT *parent, OBJECT *child, int16_t sin, int16_t cos)
{
	//Get (but hopefully not allocate) our parent's scratch
	SCRATCH_GHZSwingingPlatform *scratch = parent->Scratch<SCRATCH_GHZSwingingPlatform>();
	
	//Get our next position
	int16_t origX = scratch->origX;
	int16_t origY = scratch->origY;
	
	int16_t pixelLength = child->subtype * 16;
	if (child == parent)
		pixelLength -= 8; //Platform (parent) is offset 8 pixels up
	
	child->x.pos = origX + (cos * pixelLength / 0x100);
	child->y.pos = origY + (sin * pixelLength / 0x100);
}

void ObjGHZSwingingPlatform_Move(OBJECT *object)
{
	//Get our swing angle
	int8_t oscillate = gLevel->oscillate[6][0] >> 8;
	if (object->status.xFlip)
		oscillate = (-oscillate) - 0x80;
	
	//Move all child objects (and self)
	int16_t sin = GetSin(oscillate), cos = GetCos(oscillate);
	for (size_t i = 0; i < object->children.size(); i++)
		ObjGHZSwingingPlatform_Move_Individual(object, object->children[i], sin, cos);
	ObjGHZSwingingPlatform_Move_Individual(object, object, sin, cos);
}

void ObjGHZSwingingPlatform(OBJECT *object)
{
	//Allocate scratch memory
	SCRATCH_GHZSwingingPlatform *scratch = object->Scratch<SCRATCH_GHZSwingingPlatform>();
	
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZSwingingPlatform.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			
			//Get our collision size
			object->widthPixels = 24;
			object->heightPixels = 32;
			object->yRadius = 8;
			
			//Remember our origin position
			scratch->origX = object->x.pos;
			scratch->origY = object->y.pos;
			
			//Create the chain
			uint8_t chains = object->subtype & 0xF;
			uint8_t yOff = 0;
			
			for (int i = 0; i <= chains; i++)
			{
				//Create a segment
				OBJECT *newSegment = new OBJECT(&ObjGHZSwingingPlatform);
				newSegment->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
				newSegment->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZSwingingPlatform.map");
				newSegment->renderFlags.alignPlane = true;
				newSegment->widthPixels = 8;
				newSegment->heightPixels = 32;
				newSegment->mappingFrame = 1;
				newSegment->priority = 4;
				
				//Set routine, offset position, and frame
				newSegment->subtype = yOff;
				newSegment->routine = 2;
				
				if (yOff++ == 0)
				{
					newSegment->mappingFrame = 2;
					newSegment->priority = 3;
				}
				
				//Link to children list
				object->children.link_back(newSegment);
			}
			
			//Set the Y-offset for the platform too
			object->subtype = yOff;
		}
	//Fallthrough
		case 1: //Platform and controller
		{
			//Move, act as a platform, and draw
			int16_t lastX = object->x.pos;
			ObjGHZSwingingPlatform_Move(object);
			object->SolidObjectTop(object->widthPixels, object->yRadius + 1, lastX, false, nullptr);
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(scratch->origX);
			break;
		}
		case 2:
		{
			//Draw
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
	}
}
