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
	//Scratch
	enum SCRATCH
	{
		//U8
		SCRATCHU8_DEPRESS_POSITION =	0,
		SCRATCHU8_DEPRESS_FORCE =		1,
		SCRATCHU8_MAX =					2,
	};
	
	object->ScratchAllocU8(SCRATCHU8_MAX);
	
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
			int subtype = object->subtype;
			int bridgeRadius = (subtype / 2) * 16;
			int16_t bridgeLeft = object->x.pos - bridgeRadius;
			
			//Create our log segments
			for (int i = 0; i < subtype; i++)
			{
				OBJECT *newSegment = new OBJECT(&object->children, &ObjBridgeSegment);
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
			
			//Is a player standing on us?
			bool touching = false;
			for (int i = 0; i < OBJECT_PLAYER_REFERENCES; i++)
				if (object->playerContact[i].standing)
					touching = true;
			
			//Handle bridge depression stuff depending on players standing on us
			int16_t bridgeWidth = object->subtype * 8;
			int16_t bridgeWidthSecondary = bridgeWidth * 2;
			bridgeWidth += 8;
			
			if (!touching)
			{
				//Decrease force if no-one is standing on us
				if (object->scratchU8[SCRATCHU8_DEPRESS_FORCE] != 0)
					object->scratchU8[SCRATCHU8_DEPRESS_FORCE] -= 4;
			}
			else
			{
				//Check for any players standing on us and handle appropriately
				int i = 0;
				
				for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
				{
					//Check if this specific player is standing on us
					if (object->playerContact[i].standing)
					{
						//If a secondary player, pull the bridge position slightly towards us
						int16_t standingLog = ((player->x.pos - object->x.pos) + bridgeWidth) / 16;
						
						if (i != 0)
						{
							if (standingLog < object->scratchU8[SCRATCHU8_DEPRESS_POSITION])
								object->scratchU8[SCRATCHU8_DEPRESS_POSITION]--;
							else if (standingLog > object->scratchU8[SCRATCHU8_DEPRESS_POSITION])
								object->scratchU8[SCRATCHU8_DEPRESS_POSITION]++;
						}
					}
					
					//Check next player's contact
					i++;
				}
				
				//Increase force if someone is standing on us
				if (object->scratchU8[SCRATCHU8_DEPRESS_FORCE] != 0x40)
					object->scratchU8[SCRATCHU8_DEPRESS_FORCE] += 4;
			}
			
			//Handle depression
			int j = 0;
			
			for (OBJECT *child = object->children; child != nullptr; child = child->next)
			{
				//Get the angle of this log (go up to 0x40 from the left, and go back down to 0x00 to the right)
				uint8_t angle;
				if (j <= object->scratchU8[SCRATCHU8_DEPRESS_POSITION])
					angle = (0x40 * (j + 1)) / (object->scratchU8[SCRATCHU8_DEPRESS_POSITION] + 1); //To the left of the depress position
				else
					angle = (0x40 * (subtype - j)) / (subtype - object->scratchU8[SCRATCHU8_DEPRESS_POSITION]); //To the right of the depress position
				
				//Get the depression value from the value above, scaled by the force of the players on it (0x00 with no-one on it, 0x40 when someone is on it)
				int16_t depress;
				GetSine(object->scratchU8[SCRATCHU8_DEPRESS_FORCE] * angle / 0x40, &depress, nullptr);
				
				//Set our depression position, then check next log
				child->y.pos = object->y.pos + (depress * depressForce[object->scratchU8[SCRATCHU8_DEPRESS_POSITION]] / 0x100);
				j++;
			}
			
			//Act as a solid platform
			int i = 0;
			
			for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
			{
				if (object->playerContact[i].standing)
				{
					//Check if we're leaving the platform
					int16_t xDiff = (player->x.pos - object->x.pos) + bridgeWidth;
					
					if (player->status.inAir || xDiff < 0 || xDiff >= bridgeWidthSecondary)
					{
						//Leave the platform
						player->status.shouldNotFall = false;
						object->playerContact[i].standing = false;
					}
					else
					{
						//Set the depression position if we're the lead player on the bridge
						xDiff /= 16;
						if (i == 0)
							object->scratchU8[SCRATCHU8_DEPRESS_POSITION] = xDiff;
						
						//Set our y-position
						OBJECT *child = object->children;
						for (int k = 0; k < xDiff; k++)
							child = child->next;
						player->y.pos = child->y.pos - (8 + player->yRadius);
					}
				}
				else
				{
					//Check to land onto the bridge
					int16_t standingLog = ((player->x.pos - object->x.pos) + bridgeWidth) / 16;
					object->LandOnPlatform(player, i, bridgeWidth, bridgeWidthSecondary, 8, object->x.pos);
					
					if (object->playerContact[i].standing)
					{
						//If we're the lead, update depress position
						if (i == 0)
							object->scratchU8[SCRATCHU8_DEPRESS_POSITION] = standingLog;
					}
				}
				
				//Check next player's contact
				i++;
			}
			break;
		}
	}
}
