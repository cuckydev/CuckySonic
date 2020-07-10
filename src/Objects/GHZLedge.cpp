#include "../Game.h"
#include "../MathUtil.h"

//#define FIX_PLAYER_RELEASE //In the original (and shockingly, even the Sonic 2 port of the object), the ledge sets you to fall if you're standing on *any* object, not just the actual ledge

const int8_t ledgeSlope[] = {
	 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,
	 3,  3,  3,  3,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,
	 9,  9,  9,  9, 10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 14, 14, 14, 14,
	15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

const OBJECT_FRAGMENTMAP ledgeFragment[] = {
	{{80,  0, 16, 20}, 28},
	{{64,  0, 16, 20}, 24},
	{{48,  4, 16, 16}, 20},
	{{32,  4, 16, 16}, 16},
	
	{{80, 20, 16, 16}, 26},
	{{64, 20, 16, 16}, 22},
	{{48, 20, 16, 16}, 18},
	{{32, 20, 16, 16}, 14},
	{{16, 12, 16, 24}, 10},
	{{ 0, 12, 16, 24},  6},
	
	{{80, 36, 16, 16}, 24},
	{{64, 36, 16, 16}, 20},
	{{48, 36, 16, 16}, 16},
	{{32, 36, 16, 16}, 12},
	{{16, 36, 16, 16},  8},
	{{ 0, 36, 16, 16},  4},
	
	{{80, 52, 16, 16}, 22},
	{{64, 52, 16, 16}, 18},
	{{48, 52, 16, 16}, 14},
	{{32, 52, 16, 16}, 10},
	{{16, 52, 16, 16},  6},
	{{ 0, 52, 16, 16},  2},
	
	{{80, 68, 16, 16}, 26},
	{{64, 68, 16, 16}, 16},
	{{48, 68, 16, 16}, 10},
};

void ObjGHZLedge_Fragment(OBJECT *object)
{
	//Wait the given amount of time to start falling
	if (object->subtype == 0)
	{
		//Move, fall, and delete once off-screen
		object->MoveAndFall();
		if (!object->renderFlags.isOnscreen)
			object->deleteFlag = true;
	}
	else
		object->subtype--;
	
	//Draw to screen
	object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
}

void ObjGHZLedge(OBJECT *object)
{
	//Define and allocate our scratch
	struct SCRATCH
	{
		uint8_t flag = 0;
		uint8_t delay = 7;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
	switch (object->routine)
	{
		case 0:
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZLedge.map");
			
			//Initialize render properties
			object->renderFlags.alignPlane = true;
			object->renderFlags.xFlip = object->status.xFlip;
			object->widthPixels = 100;
			object->heightPixels = 56;
			object->mappingFrame = object->subtype;
			object->priority = 4;
	//Fallthrough
		case 1:
		{
			//Set collapse flag if a player standing on us
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				if (object->playerContact[i].standing && scratch->flag == 0)
				{
					scratch->flag = 1;
					break;
				}
			}
			
			//Handle collapsing
			if (scratch->flag == 1)
			{
				//Wait 8 frames before collapsing
				if (scratch->delay == 0)
				{
					//Fragment and go to deletion routine
					object->Fragment(25, ledgeFragment, &ObjGHZLedge_Fragment);
					scratch->delay = ledgeFragment[0].delay;
					scratch->flag = 2;
					break;
				}
				
				//Decrement delay
				scratch->delay--;
			}
			else if (scratch->flag != 0)
			{
				//Wait some time before releasing collision
				if (scratch->delay == 0)
				{
					//Release players standing
					for (size_t i = 0; i < gLevel->playerList.size(); i++)
					{
						PLAYER *player = gLevel->playerList[i];
					#ifndef FIX_PLAYER_RELEASE
						if (player->status.shouldNotFall)
					#else
						if (object->playerContact[i].standing)
					#endif
						{
							player->status.shouldNotFall = false;
							player->status.pushing = false;
							player->prevAnim = (PLAYERANIMATION)1;
						}
					}
					
					//Delete us
					object->deleteFlag = true;
					gLevel->ReleaseObjectLoad(object);
					break;
				}
				
				//Decrement delay
				scratch->delay--;
			}
			
			//Draw and act as solid
			object->SolidObjectTop(48, 32, object->x.pos, false, ledgeSlope);
			if (scratch->flag <= 1)
			{
				object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
				object->UnloadOffscreen(object->x.pos);
			}
			break;
		}
	}
}
