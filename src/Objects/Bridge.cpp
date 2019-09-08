#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../MathUtil.h"

void ObjBridgeSegment(OBJECT *object)
{
	if (object->routine == 0)
	{
		//Set our properties
		object->routine++;
		object->renderFlags.alignPlane = true;
		object->widthPixels = 0x40;
		object->priority = 3;
		
		//Load graphics
		switch (gLevel->zone)
		{
			case ZONEID_GHZ: //Green Hill Zone
				object->texture = gLevel->GetObjectTexture("data/Object/GHZBridge.bmp");
				break;
			case ZONEID_EHZ: //Emerald Hill Zone
				object->texture = gLevel->GetObjectTexture("data/Object/EHZBridge.bmp");
				break;
		}
		
		object->mappings = gLevel->GetObjectMappings("data/Object/Bridge.map");
	}
	
	//Draw this segment
	object->Draw();
}

void ObjBridge(OBJECT *object)
{
	enum SCRATCH
	{
		//S16
		SCRATCHS16_DEPRESS_POSITION = 0,
		SCRATCHS16_DEPRESS_FORCE = 1,
	};
	
	switch (object->routine)
	{
		case 0:
		{
			//Set our properties
			object->routine++;
			object->priority = 3;
			object->renderFlags.alignPlane = true;
			object->widthPixels = 0x80;
			
			//Get our type properties
			int *subtypePointer = &object->subtype;
			int subtype = *subtypePointer;
			int bridgeRadius = (subtype / 2) * 16;
			int16_t bridgeLeft = object->x.pos - bridgeRadius;
			
			//Create our log segments
			for (int i = 0; i < subtype; i++)
			{
				OBJECT *newSegment = new OBJECT(&object->children, &ObjBridgeSegment);
				newSegment->parent = (void*)object;
				newSegment->x.pos = bridgeLeft + 16 * i;
				newSegment->y.pos = object->y.pos;
			}
		}
//Fallthrough
		case 1:
		{
			//Handle our depression
			int subtype = object->subtype;
			
			//Get our depression force values (basically, it just increments by 2 to the middle, then decrements by 2)
			//ex. 2, 4, 6, 8, 8, 6, 4, 2 (at least it should produce results like this)
			int16_t depressForce[0x100];
			
			for (int i = 0, v = 0; i < subtype / 2; i++)
				depressForce[i] = (v += 2);
			for (int i = subtype - 1, v = 0; i >= subtype / 2; i--)
				depressForce[i] = (v += 2);
			
			//Get our depression value
			int depressCurrentPosition = 0;
			int depressCurrentForce = 0;
			int depressCurrentPlayers = 0;
			
			//Check players on the bridge
			int16_t bridgeLeft = (object->x.pos - (subtype * 8));
			
			for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
			{
				OBJECT *child = object->children;
				int log = 0;
				
				for (OBJECT *child = object->children; child != NULL; child = child->next)
				{
					//Check this log
					if (player->status.shouldNotFall && player->interact == (void*)child)
					{
						//Depress at this log
						depressCurrentPosition += log;
						depressCurrentForce += depressForce[log];
						depressCurrentPlayers++;
					}
					
					//Check next log
					log++;
				}
			}
			
			//Update our depression values
			if (depressCurrentPlayers != 0) //There are players standing on us
			{
				//Increment our angle / force, so there's a smooth transition when running on / landing on the bridge
				if (object->angle < 0x40)
					object->angle += 4;
				
				//Update our position and force
				object->scratchS16[SCRATCHS16_DEPRESS_POSITION] = depressCurrentPosition / depressCurrentPlayers;
				object->scratchS16[SCRATCHS16_DEPRESS_FORCE] = depressCurrentForce / depressCurrentPlayers;
			}
			else
			{
				//Decrement our angle / force, so it transitions back when jumping off / running off the bridge
				if (object->angle > 0)
					object->angle -= 4;
			}
			
			//Depress the log children
			int16_t depressPosition = object->scratchS16[SCRATCHS16_DEPRESS_POSITION];
			
			int i = 0;
			for (OBJECT *child = object->children; child != NULL; child = child->next)
			{
				//Get the angle of this log (go up to 0x40 from the left, and go back down to 0x00 to the right)
				uint8_t angle;
				if (i <= depressPosition)
					angle = (0x40 * (i + 1)) / (depressPosition + 1); //To the left of the depress position
				else
					angle = (0x40 * (subtype - i)) / (subtype - depressPosition); //To the right of the depress position
				
				//Get the depression value from the value above, scaled by the force of the players on it (0x00 with no-one on it, 0x40 when someone is on it)
				int16_t depress;
				GetSine(object->angle * angle / 0x40, &depress, NULL);
				
				//Set our depression position
				child->y.pos = object->y.pos + (depress * object->scratchS16[SCRATCHS16_DEPRESS_FORCE] / 0x100);
				child->PlatformObject(8, 8, child->x.pos);
				
				//Increment our log index
				i++;
			}
			break;
		}
	}
}
