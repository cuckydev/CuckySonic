#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"
#include "../Objects.h"
#include "../Audio.h"

#define MONITOR_WIDTH	26
#define MONITOR_HEIGHT	16

//Solid functions
void ObjMonitor_ChkOverEdge(OBJECT *object, int i, PLAYER *player)
{
	//Check if we're still on the platform
	int xDiff = (player->x.pos - object->x.pos) + MONITOR_WIDTH;
	
	if (!player->status.inAir && xDiff >= 0 && xDiff < MONITOR_WIDTH * 2)
	{
		player->MoveOnPlatform((void*)object, MONITOR_HEIGHT, object->x.pos);
		return;
	}
	
	//Leave the platform
	player->status.shouldNotFall = false;
	player->status.inAir = true;
	object->playerContact[i].standing = false;
	return;
}

void ObjMonitor_SolidObject_Lead(OBJECT *object, OBJECT_SOLIDTOUCH *solidTouch, int i, PLAYER *player)
{
	//Basically, act as a solid if we're either already on top of the monitor, or not in ball form
	if (object->playerContact[i].standing)
		ObjMonitor_ChkOverEdge(object, i, player);
	else if (player->anim != PLAYERANIMATION_ROLL)
		object->SolidObjectCont(solidTouch, player, i, MONITOR_WIDTH, MONITOR_HEIGHT, object->x.pos);
}

void ObjMonitor_SolidObject_Follower(OBJECT *object, OBJECT_SOLIDTOUCH *solidTouch, int i, PLAYER *player)
{
	//There's a 2-player check in Sonic 2 here
	object->SolidObjectCont(solidTouch, player, i, MONITOR_WIDTH, MONITOR_HEIGHT, object->x.pos);
}

OBJECT_SOLIDTOUCH ObjMonitor_SolidObject(OBJECT *object)
{
	//Check for contact with all players (followers cannot break monitors)
	OBJECT_SOLIDTOUCH solidTouch;
	int i = 0;
	
	for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
	{
		if (player == gLevel->playerList)
			ObjMonitor_SolidObject_Lead(object, &solidTouch, i++, player);
		else
			ObjMonitor_SolidObject_Follower(object, &solidTouch, i++, player);
	}
	
	return solidTouch;
}

//Animation and enum
enum MONITOR_ITEM
{
	MONITOR_ITEM_STATIC,
	MONITOR_ITEM_EXTRA_LIFE,
	MONITOR_ITEM_EGGMAN,
	MONITOR_ITEM_RING,
	MONITOR_ITEM_SPEED_SHOES,
	MONITOR_ITEM_SHIELD,
	MONITOR_ITEM_INVINCIBILITY,
	MONITOR_ITEM_BROKEN,
};

const MONITOR_ITEM zoneItemSonic1[] = {MONITOR_ITEM_STATIC, MONITOR_ITEM_EGGMAN, MONITOR_ITEM_EXTRA_LIFE, MONITOR_ITEM_SPEED_SHOES, MONITOR_ITEM_SHIELD, MONITOR_ITEM_INVINCIBILITY, MONITOR_ITEM_RING};
const MONITOR_ITEM zoneItemSonic2[] = {MONITOR_ITEM_STATIC, MONITOR_ITEM_EXTRA_LIFE, MONITOR_ITEM_EXTRA_LIFE, MONITOR_ITEM_EGGMAN, MONITOR_ITEM_RING, MONITOR_ITEM_SPEED_SHOES, MONITOR_ITEM_SHIELD, MONITOR_ITEM_INVINCIBILITY};

const MONITOR_ITEM *zoneItemByZone[] = {
	zoneItemSonic1, //ZONEID_GHZ
	zoneItemSonic2, //ZONEID_EHZ
};

static const uint8_t animationStatic[] =		{0x01,0x00,0x01,0xFF}; //static 1, static 2
static const uint8_t animationExtraLife[] =		{0x07,0x00,0x02,0x02,0x01,0x02,0x02,0xFF}; //static 1, item, item, static 2, item, item
static const uint8_t animationEggman[] =		{0x07,0x00,0x03,0x03,0x01,0x03,0x03,0xFF}; //vvv
static const uint8_t animationRing[] =			{0x07,0x00,0x04,0x04,0x01,0x04,0x04,0xFF};
static const uint8_t animationSpeedShoes[] =	{0x07,0x00,0x05,0x05,0x01,0x05,0x05,0xFF};
static const uint8_t animationShield[] =		{0x07,0x00,0x06,0x06,0x01,0x06,0x06,0xFF};
static const uint8_t animationInvincibility[] =	{0x07,0x00,0x07,0x07,0x01,0x07,0x07,0xFF};
static const uint8_t animationBroken[] =		{0x02,0x00,0x01,0x08,0xFE,0x01};

static const uint8_t *animationList[] = {
	animationStatic,
	animationExtraLife,
	animationEggman,
	animationRing,
	animationSpeedShoes,
	animationShield,
	animationInvincibility,
	animationBroken,
};

//Contents code
void ObjMonitorContents(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Monitor.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/MonitorContents.map");
			
			//Set render properties and velocity
			object->renderFlags.alignPlane = true;
			object->highPriority = true;
			object->priority = 3;
			object->widthPixels = 8;
			object->yVel = -0x300;
			
			//Get our mapping frame (anim + 1 stupid dumb)
			object->mappingFrame = object->anim + 1;
		}
	//Fallthrough
		case 1: //Rising
		{
			//If still moving up, rise and slow down
			if (object->yVel < 0)
			{
				object->Move();
				object->yVel += 0x18;
			}
			else
			{
				//Stop rising and increment to the routine where we wait to be deleted
				object->routine++;
				object->animFrameDuration = 29;
				
				//Handle our item
				PLAYER *breakPlayer = (PLAYER*)((OBJECT*)object->parent)->parent;
				MUSICSPEC itemMusicSpec;
				
				switch (object->anim)
				{
					case MONITOR_ITEM_EXTRA_LIFE:
						//Give us one extra life and play the jingle
						AddToLives(1);
						
						//Play jingle
						itemMusicSpec = {"ExtraLifeJingle", 0, 1.0f};
						gLevel->PlayJingle(itemMusicSpec);
						break;
					case MONITOR_ITEM_STATIC:
					case MONITOR_ITEM_EGGMAN:
						//Hurt the player
						if (breakPlayer != NULL)
							breakPlayer->CheckHurt((void*)object);
						break;
					case MONITOR_ITEM_RING:
						//Give us 10 rings
						AddToRings(10);
						break;
					case MONITOR_ITEM_SPEED_SHOES:
						//Give the player who hit us speed shoes
						if (breakPlayer != NULL)
							breakPlayer->GiveSpeedShoes();
						break;
					case MONITOR_ITEM_SHIELD:
						//Give the player who hit us a shield
						if (breakPlayer != NULL)
						{
							PlaySound(SOUNDID_GET_BLUE_SHIELD);
							breakPlayer->GiveShield(SOUNDID_NULL, SHIELD_BLUE);
						}
						break;
					case MONITOR_ITEM_INVINCIBILITY:
						break;
					default:
						break;
				}
			}
			
			object->Draw();
			break;
		}
		case 2: //Waiting for deletion
		{
			//Once our 30 frame timer runs out, delete us, otherwise, draw
			if (--object->animFrameDuration < 0)
				object->deleteFlag = true;
			else
				object->Draw();
		}
	}
}

//Main object code
void ObjMonitor(OBJECT *object)
{
	switch (object->routine)
	{
		case 0:
		{
			//Increment routine and set collision size
			object->routine++;
			object->xRadius = 14;
			object->yRadius = 14;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Monitor.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Monitor.map");
			
			//Set render properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			object->widthPixels = 15;
			
			//Setup our collision
			object->collisionType = COLLISIONTYPE_MONITOR;
			object->touchWidth = 16;
			object->touchHeight = 16;
			
			//Set our animation
			object->anim = zoneItemByZone[gLevel->zone][object->subtype];
		}
	//Fallthrough
		case 1: //Not broken
		{
			//If hit from below, fall to the ground
			if (object->routineSecondary)
			{
				//Fall and check for the floor
				object->MoveAndFall();
				
				int16_t distance = object->CheckFloorEdge(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos, NULL);
				if (distance < 0)
				{
					//Land on the ground and stop falling
					object->y.pos += distance;
					object->yVel = 0;
					object->routineSecondary = 0;
				}
			}
			
			//Act as solid, draw and animate
			ObjMonitor_SolidObject(object);
			object->Draw();
			object->Animate(animationList);
			break;
		}
		case 2: //Breaking from contact
		{
			//Clear player contact
			object->ClearSolidContact();
			
			//Break the monitor
			object->routine++;
			object->collisionType = COLLISIONTYPE_NULL;
			
			//Create the item content thing
			OBJECT *content = new OBJECT(&gLevel->objectList, &ObjMonitorContents);
			content->x.pos = object->x.pos;
			content->y.pos = object->y.pos;
			content->anim = object->anim;
			content->parent = (void*)object;
			
			//Create the explosion
			OBJECT *explosion = new OBJECT(&gLevel->objectList, &ObjExplosion);
			explosion->x.pos = object->x.pos;
			explosion->y.pos = object->y.pos;
			explosion->routine++; //Don't create animal or score
			
			//Set to broken animation and draw
			object->anim = MONITOR_ITEM_BROKEN;
			object->Draw();
			break;
		}
		case 3: //Broken
		{
			//Draw and animate
			object->Draw();
			object->Animate(animationList);
			break;
		}
	}
}
