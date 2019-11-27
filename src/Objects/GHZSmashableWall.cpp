#include "../Level.h"
#include "../Game.h"
#include "../Objects.h"
#include "../MathUtil.h"

static const OBJECT_SMASHMAP smashmapRight[8] = {
	{{ 0,  0, 16, 16},  0x400, -0x500},
	{{ 0, 16, 16, 16},  0x600, -0x100},
	{{ 0, 32, 16, 16},  0x600,  0x100},
	{{ 0, 48, 16, 16},  0x400,  0x500},
	{{16,  0, 16, 16},  0x600, -0x600},
	{{16, 16, 16, 16},  0x800, -0x200},
	{{16, 32, 16, 16},  0x800,  0x200},
	{{16, 48, 16, 16},  0x600,  0x600},
};

static const OBJECT_SMASHMAP smashmapLeft[8] = {
	{{ 0,  0, 16, 16}, -0x400, -0x500},
	{{ 0, 16, 16, 16}, -0x600, -0x100},
	{{ 0, 32, 16, 16}, -0x600,  0x100},
	{{ 0, 48, 16, 16}, -0x400,  0x500},
	{{16,  0, 16, 16}, -0x600, -0x600},
	{{16, 16, 16, 16}, -0x800, -0x200},
	{{16, 32, 16, 16}, -0x800,  0x200},
	{{16, 48, 16, 16}, -0x600,  0x600},
};

//Fragment
void ObjGHZWallFragment(OBJECT *object)
{
	//Move, fall, and delete once off-screen
	object->Move();
	object->yVel += 0x70;
	object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
	if (!object->renderFlags.isOnscreen)
		object->deleteFlag = true;
}

//Smashable wall object
void ObjGHZSmashableWall(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/GHZGeneric.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/GHZSmashableWall.map");
			
			//Initialize other render properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 16;
			object->heightPixels = 32;
			object->priority = 4;
			object->mappingFrame = object->subtype;
		}
	//Fallthrough
		case 1:
		{
			size_t i = 0;
			PLAYER *player = gLevel->playerList[i];
			
			//Act as solid, and check if we're going into the wall
			int16_t oldXVel = player->xVel;
			OBJECT_SOLIDTOUCH touch = object->SolidObjectFull(27, 32, 32, object->x.pos, false, nullptr, false);
			
			if (touch.side[i])
			{
				//Make sure we meet the smashing conditions, and smash depending on our direction
				if (player->characterType == CHARACTERTYPE_KNUCKLES || player->super || (player->barrier == BARRIER_FLAME && player->jumpAbility == 1) || (player->anim == PLAYERANIMATION_ROLL && mabs(oldXVel) >= 0x480 && !player->status.inAir))
				{
					//Move towards wall and smash
					player->x.pos += 4; //Why is this done before position checking?
					
					const OBJECT_SMASHMAP *smashmap;
					if (player->x.pos >= object->x.pos)
					{
						//Smash from the right
						smashmap = smashmapRight;
					}
					else
					{
						//Smash from the left
						player->x.pos -= 8;
						smashmap = smashmapLeft;
					}
					
					//Smash
					player->xVel = oldXVel;
					player->inertia = player->xVel;
					player->status.pushing = false;
					
					object->playerContact[i].pushing = false;
					object->Smash(8, smashmap, &ObjGHZWallFragment);
				}
			}
			break;
		}
	}
	
	//Draw and unload once off-screen
	object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
	object->UnloadOffscreen(object->x.pos);
}
