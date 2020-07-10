#include "../Game.h"

void ObjGHZSpikeLog_Segment(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZSpikeLog.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			object->widthPixels = 8;
			object->heightPixels = 32;
		}
	//Fallthrough
		case 1:
		{
			//Rotate and check if we should hurt
			if ((object->mappingFrame = ((gLevel->frameCounter / -12) + object->subtype) & 0x7) == 0)
			{
				object->collisionType = COLLISIONTYPE_HURT;
				object->touchWidth = 4;
				object->touchHeight = 16;
			}
			else
			{
				object->collisionType = COLLISIONTYPE_NULL;
				object->touchWidth = 0;
				object->touchHeight = 0;
			}
			
			//Draw us
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
		}
	}
}

void ObjGHZSpikeLog(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Create our log segments
			int16_t logLeft = object->x.pos - (object->subtype * 8);
			for (unsigned int i = 0; i < object->subtype; i++)
			{
				OBJECT *newSegment = new OBJECT(&ObjGHZSpikeLog_Segment);
				newSegment->x.pos = logLeft + 16 * i;
				newSegment->y.pos = object->y.pos;
				newSegment->subtype = (i & 0x7);
				object->children.link_back(newSegment);
			}
		}
	//Fallthrough
		case 1:
		{
			//Unload once off screen
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
