#include "../Game.h"
#include "../Objects.h"
#include "../Audio.h"

//Bugfixes
//#define MONITOR_FIX_PUSHING	//There's a bit of an issue with pushing on monitors, if you jump while pushing, it won't clear the pushing bit, and this will screw up stuff like the super transformation, and spindashing into the sides

//Size constants
#define MONITOR_WIDTH	26
#define MONITOR_HEIGHT	15

//Solid functions
void ObjMonitor_ChkOverEdge(OBJECT *object, int i, PLAYER *player)
{
	//Check if we're still on the platform
	int xDiff = (player->x.pos - object->x.pos) + MONITOR_WIDTH;
	
	if (!player->status.inAir && xDiff >= 0 && xDiff < MONITOR_WIDTH * 2)
	{
		//Move on top of the monitor
		object->MovePlayer(player, MONITOR_WIDTH, MONITOR_HEIGHT + 1, object->x.pos, nullptr, false);
	}
	else
	{
		//Leave the top of the monitor
		player->status.shouldNotFall = false;
		player->status.inAir = true;
		object->playerContact[i].standing = false;
	}
}

void ObjMonitor_SolidObject_Lead(OBJECT *object, int i, PLAYER *player)
{
	//Basically, act as a solid if we're either already on top of the monitor, or not in ball form
	if (object->playerContact[i].standing)
		ObjMonitor_ChkOverEdge(object, i, player);
	else if (player->anim != PLAYERANIMATION_ROLL && player->anim != PLAYERANIMATION_DROPDASH)
		object->SolidObjectFull_Cont(nullptr, player, i, MONITOR_WIDTH, MONITOR_HEIGHT, object->x.pos, nullptr, false);
#ifdef MONITOR_FIX_PUSHING
	else if (object->playerContact[i].pushing)
	{
		//Clear pushing
		player->status.pushing = false;
		object->playerContact[i].pushing = false;
	}
#endif
}

void ObjMonitor_SolidObject_Follower(OBJECT *object, int i, PLAYER *player)
{
	//There's a 2-player check in Sonic 2 here
	if (object->playerContact[i].standing)
		ObjMonitor_ChkOverEdge(object, i, player);
	else
		object->SolidObjectFull_Cont(nullptr, player, i, MONITOR_WIDTH, MONITOR_HEIGHT, object->x.pos, nullptr, false);
}

void ObjMonitor_SolidObject(OBJECT *object)
{
	//Check for contact with all players (followers cannot break monitors)
	for (size_t i = 0; i < gLevel->playerList.size(); i++)
	{
		if (i == 0)
			ObjMonitor_SolidObject_Lead(object, i, gLevel->playerList[i]);
		else
			ObjMonitor_SolidObject_Follower(object, i, gLevel->playerList[i]);
	}
}

//Animation and enum
enum MONITOR_ITEM
{
	MONITOR_ITEM_STATIC,
	MONITOR_ITEM_EXTRA_LIFE,
	MONITOR_ITEM_EGGMAN,
	MONITOR_ITEM_RING,
	MONITOR_ITEM_SPEED_SHOES,
	MONITOR_ITEM_BARRIER,
	MONITOR_ITEM_INVINCIBILITY,
	MONITOR_ITEM_BROKEN,
};

const MONITOR_ITEM zoneItemSonic1[] = {MONITOR_ITEM_STATIC, MONITOR_ITEM_EGGMAN, MONITOR_ITEM_EXTRA_LIFE, MONITOR_ITEM_SPEED_SHOES, MONITOR_ITEM_BARRIER, MONITOR_ITEM_INVINCIBILITY, MONITOR_ITEM_RING};
const MONITOR_ITEM zoneItemSonic2[] = {MONITOR_ITEM_STATIC, MONITOR_ITEM_EXTRA_LIFE, MONITOR_ITEM_EXTRA_LIFE, MONITOR_ITEM_EGGMAN, MONITOR_ITEM_RING, MONITOR_ITEM_SPEED_SHOES, MONITOR_ITEM_BARRIER, MONITOR_ITEM_INVINCIBILITY};

const MONITOR_ITEM *zoneItemByZone[] = {
	zoneItemSonic1, //ZONEID_GHZ
	zoneItemSonic2, //ZONEID_EHZ
};

static const uint8_t animationStatic[] =		{0x01,0x00,0x01,ANICOMMAND_RESTART}; //static 1, static 2
static const uint8_t animationExtraLife[] =		{0x07,0x00,0x02,0x02,0x01,0x02,0x02,ANICOMMAND_RESTART}; //static 1, item, item, static 2, item, item
static const uint8_t animationEggman[] =		{0x07,0x00,0x03,0x03,0x01,0x03,0x03,ANICOMMAND_RESTART}; //vvv
static const uint8_t animationRing[] =			{0x07,0x00,0x04,0x04,0x01,0x04,0x04,ANICOMMAND_RESTART};
static const uint8_t animationSpeedShoes[] =	{0x07,0x00,0x05,0x05,0x01,0x05,0x05,ANICOMMAND_RESTART};
static const uint8_t animationBarrier[] =		{0x07,0x00,0x06,0x06,0x01,0x06,0x06,ANICOMMAND_RESTART};
static const uint8_t animationInvincibility[] =	{0x07,0x00,0x07,0x07,0x01,0x07,0x07,ANICOMMAND_RESTART};
static const uint8_t animationBroken[] =		{0x02,0x00,0x01,0x08,ANICOMMAND_GO_BACK_FRAMES,0x01}; //static 1, static 2, > broken <

static const uint8_t *animationList[] = {
	animationStatic,
	animationExtraLife,
	animationEggman,
	animationRing,
	animationSpeedShoes,
	animationBarrier,
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
			object->texture = gLevel->GetObjectTexture("data/Object/Generic.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/MonitorContents.map");
			
			//Set render properties and velocity
			object->renderFlags.alignPlane = true;
			object->highPriority = true;
			object->priority = 3;
			object->widthPixels = 8;
			object->heightPixels = 32;
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
				PLAYER *breakPlayer = object->parentObject->parentPlayer;
				
				switch (object->anim)
				{
					case MONITOR_ITEM_EXTRA_LIFE:
						//Give us one extra life and play the jingle
						AddToLives(1);
						break;
					case MONITOR_ITEM_STATIC:
					case MONITOR_ITEM_EGGMAN:
						//Hurt the player
						if (breakPlayer != nullptr)
							breakPlayer->HurtFromObject(object);
						break;
					case MONITOR_ITEM_RING:
						//Give us 10 rings
						AddToRings(10);
						break;
					case MONITOR_ITEM_SPEED_SHOES:
						//Give the player who hit us speed shoes
						if (breakPlayer != nullptr)
							breakPlayer->GiveSpeedShoes();
						break;
					case MONITOR_ITEM_BARRIER:
						//Give the player who hit us a barrier
						if (breakPlayer != nullptr)
							breakPlayer->GiveBarrier(SOUNDID_GET_BLUE_BARRIER, BARRIER_BLUE);
						break;
					case MONITOR_ITEM_INVINCIBILITY:
						//Give the player who hit us invincibility
						if (breakPlayer != nullptr)
							breakPlayer->GiveInvincibility();
						break;
					default:
						break;
				}
			}
			
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 2: //Waiting for deletion
		{
			//Once our 30 frame timer runs out, delete us, otherwise, draw
			if (--object->animFrameDuration < 0)
				object->deleteFlag = true;
			else
				object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
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
			object->texture = gLevel->GetObjectTexture("data/Object/Generic.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/Monitor.map");
			
			//Set render properties
			object->renderFlags.alignPlane = true;
			object->priority = 3;
			object->widthPixels = 15;
			object->heightPixels = 14;
			
			//Set our animation / state
			if (gLevel->GetObjectLoad(object)->specificBit)
			{
				//Broken
				object->routine = 3;
				object->anim = MONITOR_ITEM_BROKEN;
				return;
			}
			else
			{
				//Setup our collision
				object->collisionType = COLLISIONTYPE_MONITOR;
				object->touchWidth = 16;
				object->touchHeight = 16;
				
				//Use subtype animation
				object->anim = zoneItemByZone[gLevel->zone][object->subtype];
			}
		}
	//Fallthrough
		case 1: //Not broken
		{
			//If hit from below, fall to the ground
			if (object->routineSecondary)
			{
				//Fall and check for the floor
				object->MoveAndFall();
				
				int16_t distance = object->CheckCollisionDown_1Point(COLLISIONLAYER_NORMAL_TOP, object->x.pos, object->y.pos + object->yRadius, nullptr);
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
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->Animate(animationList);
			object->UnloadOffscreen(object->x.pos);
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
			OBJECT *content = new OBJECT(&ObjMonitorContents);
			content->x.pos = object->x.pos;
			content->y.pos = object->y.pos;
			content->anim = object->anim;
			content->parentObject = object;
			gLevel->objectList.link_back(content);
			
			//Create the explosion
			OBJECT *explosion = new OBJECT(&ObjExplosion);
			explosion->x.pos = object->x.pos;
			explosion->y.pos = object->y.pos;
			explosion->routine++; //Don't create animal or score
			gLevel->objectList.link_back(explosion);
			
			//Set to broken animation and draw
			gLevel->GetObjectLoad(object)->specificBit = true;
			object->anim = MONITOR_ITEM_BROKEN;
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 3: //Broken
		{
			//Draw and animate
			object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			object->Animate(animationList);
			object->UnloadOffscreen(object->x.pos);
			break;
		}
	}
}
