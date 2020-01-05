#include "../Game.h"

//#define REV01_SPIKEBUG	//Until Sonic 2 / Sonic 1 REV02, you could take damage from spikes even during invulnerability

static const uint8_t spikeVar[][2] = {
	//mapping frame, width
	{0, 20},
	{1, 16},
	{2,  4},
	{3, 28},
	{4, 64},
	{5, 16},
};

void ObjGHZSpikes_Hurt(OBJECT *object, PLAYER *player)
{
	//Make sure player isn't invincible
	if (!player->item.isInvincible && player->routine < PLAYERROUTINE_HURT)
	{
		#ifndef REV01_SPIKEBUG
			//Don't hurt if invulnerable
			if (player->invulnerabilityTime)
				return;
		#endif
		
		//Offset player position and hurt them
		player->yLong -= (player->yVel << 8);
		player->Hurt(object);
	}
}

void ObjGHZSpikes(OBJECT *object)
{
	//Define and allocate our scratch
	struct SCRATCH
	{
		int16_t origX = 0;
		int16_t origY = 0;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
	switch (object->routine)
	{
		case 0:
		{
			//Increment our routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZSpikes.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->priority = 4;
			object->heightPixels = 32;
			
			//Handle subtype specific stuff
			uint8_t subtypeIndex = (object->subtype & 0xF0) >> 4;
			object->subtype &= 0xF;
			
			object->mappingFrame = spikeVar[subtypeIndex][0];
			object->widthPixels = spikeVar[subtypeIndex][1];
			
			//Remember original position
			scratch->origX = object->x.pos;
			scratch->origY = object->y.pos;
		}
	//Fallthrough
		case 1:
		{
			//Move according to our subtype
			//ObjGHZSpikes_Move(object);
			
			//Handle solidity and damage checking
			switch (object->mappingFrame)
			{
				//Horizontal
				case 1:
				case 5:
				{
					//Handle solid collision
					int16_t height = (object->mappingFrame == 5) ? 4 : 20;
					object->SolidObjectFull(27, height, height + 1, object->x.pos, false, nullptr, false);
					
					//Check for players touching us and getting hurt
					for (size_t i = 0; i < gLevel->playerList.size(); i++)
					{
						if (object->playerContact[i].standing == false && object->playerContact[i].pushing == true)
							ObjGHZSpikes_Hurt(object, gLevel->playerList[i]);
					}
					break;
				}
				//Vertical
				default:
				{
					//Handle solid collision
					object->SolidObjectFull(object->widthPixels + 11, 16, 17, object->x.pos, false, nullptr, false);
					
					//Check for players touching us and getting hurt
					for (size_t i = 0; i < gLevel->playerList.size(); i++)
					{
						if (object->playerContact[i].standing == true)
							ObjGHZSpikes_Hurt(object, gLevel->playerList[i]);
					}
					break;
				}
			}
			
			//Draw and check for unloading
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->UnloadOffscreen(scratch->origX);
			break;
		}
	}
}
