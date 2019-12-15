#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../MathUtil.h"

//#define FIX_DEPRESS_DELAY

void ObjBridgeSegment(OBJECT *object)
{
	if (object->routine == 0)
	{
		//Set our properties
		object->routine++;
		object->renderFlags.alignPlane = true;
		object->widthPixels = 8;
		object->heightPixels = 32;
		object->priority = 3;
		
		//Load graphics
		switch (gLevel->zone)
		{
			case ZONEID_GHZ:
				object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
				object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZBridge.map");
				break;
			case ZONEID_EHZ:
				object->texture = gLevel->GetObjectTexture("data/Object/EHZGeneric.bmp");
				object->mapping.mappings = gLevel->GetObjectMappings("data/Object/EHZBridge.map");
				break;
		}
	}
	
	//Draw this segment
	object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
}

void ObjBridge(OBJECT *object)
{
	//Define and allocate our scratch
	struct SCRATCH
	{
		uint8_t depressPosition = 0;
		uint8_t depressForce = 0;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Initialize render properties
			object->widthPixels = 128;
			object->heightPixels = 32;
			
			//Create our log segments
			int16_t bridgeLeft = object->x.pos - (object->subtype * 8);
			for (unsigned int i = 0; i < object->subtype; i++)
			{
				OBJECT *newSegment = new OBJECT(&ObjBridgeSegment);
				newSegment->x.pos = bridgeLeft + 16 * i;
				newSegment->y.pos = object->y.pos;
				object->children.link_back(newSegment);
			}
		}
//Fallthrough
		case 1:
		{
			//Get our depression force values (basically, it just increments by 2 to the middle, then the same from the other side)
			//ex. 2, 4, 6, 8, 8, 6, 4, 2
			uint8_t depressForce[0x100];
			
			for (unsigned int i = 0, v = 0; i < object->subtype / 2; i++)
				depressForce[i] = (v += 2);
			for (unsigned int i = object->subtype - 1, v = 0; i >= object->subtype / 2; i--)
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
				if (scratch->depressForce != 0x00)
					scratch->depressForce -= 0x04;
			}
			else
			{
				//Check for any players standing on us and handle appropriately
				for (size_t i = 0; i < gLevel->playerList.size(); i++)
				{
					//Get the player
					PLAYER *player = gLevel->playerList[i];
					
					//Check if this specific player is standing on us
					if (object->playerContact[i].standing)
					{
						//If a secondary player, pull the bridge position slightly towards us
						int16_t standingLog = ((player->x.pos - object->x.pos) + bridgeWidth) / 16;
						
						if (i != 0)
						{
							if (standingLog < scratch->depressPosition)
								scratch->depressPosition--;
							else if (standingLog > scratch->depressPosition)
								scratch->depressPosition++;
						}
					#ifdef FIX_DEPRESS_DELAY
						else
							scratch->depressPosition = standingLog;
					#endif
					}
				}
				
				//Increase force if someone is standing on us
				if (scratch->depressForce != 0x40)
					scratch->depressForce += 4;
			}
			
			//Handle depression
			for (size_t j = 0; j < object->children.size(); j++)
			{
				//Get the angle of this log (go up to 0x40 from the left, and go back down to 0x00 to the right)
				uint8_t angle;
				if (j < scratch->depressPosition)
					angle = (0x40 * (j + 1)) / (scratch->depressPosition + 1); //To the left of the depress position
				else
					angle = (0x40 * (object->subtype - j)) / (object->subtype - scratch->depressPosition); //To the right of the depress position
				
				//Set our depression position according to the force of a player above us and the angle of the log as gotten above
				object->children[j]->y.pos = object->y.pos + (GetSin(scratch->depressForce * angle / 0x40) * depressForce[scratch->depressPosition] / 0x100);
			}
			
			//Act as a solid platform
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player
				PLAYER *player = gLevel->playerList[i];
				
				if (object->playerContact[i].standing)
				{
					//Check if we're leaving the platform
					int16_t xDiff = (player->x.pos - object->x.pos) + bridgeWidth;
					
					if (player->status.inAir || xDiff < 0 || xDiff >= bridgeWidthSecondary)
					{
						//Leave the platform (don't set us to be inAir so walking or rolling off the bridge doesn't not work)
						player->status.shouldNotFall = false;
						object->playerContact[i].standing = false;
					}
					else
					{
						//Set the depression position if we're the lead player on the bridge
						xDiff /= 16;
						if (i == 0)
							scratch->depressPosition = xDiff;
						
						//Set our y-position
						player->y.pos = object->children[xDiff]->y.pos - (8 + player->yRadius);
					}
				}
				else
				{
					//Check to land onto the bridge
					int16_t standingLog = ((player->x.pos - object->x.pos) + bridgeWidth) / 16;
					object->LandOnTopSolid(player, i, bridgeWidth, bridgeWidthSecondary, 8, object->x.pos, nullptr);
					
					if (object->playerContact[i].standing)
					{
						//If we're the lead, update depress position
						if (i == 0)
							scratch->depressPosition = standingLog;
					}
				}
			}
			
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
