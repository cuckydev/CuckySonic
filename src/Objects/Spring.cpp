#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

#define HORIZONTAL_UPSIDEDOWN_FIX	//In the originals, horizontal springs wouldn't bounce you correctly if you were upside down (on a ceiling), this fixes that
#define SOLIDOBJECT_CONTACT_CHECK	//In S3K, springs were modified to use the contact information returned from SolidObject rather than the player contact flags, the only thing changed gameplay-wise by this is springs bounce you even in mid-air

static const uint8_t animationUpIdle[] =				{0x0F,0x00,0xFF};
static const uint8_t animationUpBounce[] =				{0x00,0x01,0x00,0x00,0x02,0x02,0x02,0x02,0x02,0x02,0xFD,0x00};
static const uint8_t animationHorizontalIdle[] =		{0x0F,0x03,0xFF};
static const uint8_t animationHorizontalBounce[] =		{0x00,0x04,0x03,0x03,0x05,0x05,0x05,0x05,0x05,0x05,0xFD,0x02};
static const uint8_t animationDiagonalUpIdle[] =		{0x0F,0x06,0xFF};
static const uint8_t animationDiagonalUpBounce[] =		{0x00,0x07,0x06,0x06,0x08,0x08,0x08,0x08,0x08,0x08,0xFD,0x04};
static const uint8_t animationDiagonalDownIdle[] =		{0x0F,0x09,0xFF};
static const uint8_t animationDiagonalDownBounce[] =	{0x00,0x0A,0x09,0x09,0x0B,0x0B,0x0B,0x0B,0x0B,0x0B,0xFD,0x06};

static const uint8_t *animationList[] = {
	animationUpIdle,
	animationUpBounce,
	animationHorizontalIdle,
	animationHorizontalBounce,
	animationDiagonalUpIdle,
	animationDiagonalUpBounce,
	animationDiagonalDownIdle,
	animationDiagonalDownBounce,
};

void ObjSpring(OBJECT *object)
{
	//Scratch
	enum SCRATCH
	{
		//S16
		SCRATCHS16_FORCE =	0,
		SCRATCHS16_MAX =	1,
	};
	
	object->ScratchAllocS16(SCRATCHS16_MAX);
	
	//Routines
	enum ROUTINE
	{
		ROUTINE_INIT,
		ROUTINE_UP,
		ROUTINE_HORIZONTAL,
		ROUTINE_DOWN,
		ROUTINE_DIAGONALLY_UP,
		ROUTINE_DIAGONALLY_DOWN,
	};
	
	switch (object->routine)
	{
		case 0:
		{
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Spring.bmp");
			if (object->subtype & 0x2)
				object->mappings = gLevel->GetObjectMappings("data/Object/YellowSpring.map");
			else
				object->mappings = gLevel->GetObjectMappings("data/Object/RedSpring.map");
			
			//Set render properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 16;
			object->heightPixels = 16;
			object->priority = 4;
			
			//Subtype specific initialization
			switch ((object->subtype >> 4) & 0x7)
			{
				default: //Up
					object->routine = ROUTINE_UP;
					break;
				case 1: //Horizontal
					object->routine = ROUTINE_HORIZONTAL;
					object->anim = 2;
					object->widthPixels = 8;
					break;
				case 2: //Down
					object->routine = ROUTINE_DOWN;
					object->status.yFlip = true;
					break;
				case 3: //Diagonally up
					object->routine = ROUTINE_DIAGONALLY_UP;
					object->anim = 4;
					break;
				case 4: //Diagonally down
					object->routine = ROUTINE_DIAGONALLY_DOWN;
					object->anim = 4;
					object->status.yFlip = true;
					break;
			}
			
			//Get our spring force
			if (object->subtype & 0x2)
				object->scratchS16[SCRATCHS16_FORCE] = -0x0A00; //Yellow spring's force
			else
				object->scratchS16[SCRATCHS16_FORCE] = -0x1000; //Red spring's force
			break;
		}
		case ROUTINE_UP: //Upwards
		case ROUTINE_DOWN: //Downwards
		{
			//Act as solid
			OBJECT_SOLIDTOUCH touch = object->SolidObject(27, 8, 16, object->x.pos);
			
			//Check if any players touched the top of us
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				PLAYER *player = &gLevel->playerList[i];
				bool isUp = (object->routine == ROUTINE_UP) ^ player->status.reverseGravity;
				
			#ifdef SOLIDOBJECT_CONTACT_CHECK
				if (isUp ? touch.top[i] : touch.bottom[i])
			#else
				if (isUp ? object->playerContact[i].standing : touch.bottom[i])
			#endif
				{
					//Play bouncing animation
					object->anim = 1;
					object->prevAnim = 0;
					
					//Launch player
					if (isUp)
					{
						player->y.pos += 8;
						player->yVel = object->scratchS16[SCRATCHS16_FORCE];
						player->anim = PLAYERANIMATION_SPRING;
					}
					else
					{
						player->y.pos -= 8;
						player->yVel = -object->scratchS16[SCRATCHS16_FORCE];
					}
					
					player->status.jumping = false;
					player->spindashing = false;
					
					player->status.inAir = true;
					player->status.shouldNotFall = false;
					player->routine = PLAYERROUTINE_CONTROL;
					
					if (object->subtype & 0x80) //Unused flag?
						player->xVel = 0;
					
					if (object->subtype & 0x1) //Twirl flag
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & 0x2) ? 0 : 1; //Do 1 extra flip if a red spring
						player->flipSpeed = 4;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & 0xC) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & 0xC) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					PlaySound(SOUNDID_SPRING);
				}
			}
			
			object->Animate(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
		case 2: //Horizontal
		{
			//Act as solid
			OBJECT_SOLIDTOUCH touch = object->SolidObject(19, 14, 15, object->x.pos);
			
			//Check if any players touched our sides
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				PLAYER *player = &gLevel->playerList[i];
			#ifdef SOLIDOBJECT_CONTACT_CHECK
				if (touch.side[i])
			#else
				if (object->playerContact[i].pushing)
			#endif
				{
					//Reverse flip if to the left
					bool launchInvert = object->status.xFlip;
					if ((object->x.pos - player->x.pos) >= 0)
						launchInvert ^= 1;
					
					#ifdef SOLIDOBJECT_CONTACT_CHECK
						//Drop collision if on wrong side
						if (launchInvert)
						{
							i++;
							continue;
						}
					#endif
					
					//Play bouncing animation
					object->anim = 3;
					object->prevAnim = 0;
					
					//Launch player
					if (object->status.xFlip != launchInvert)
					{
						player->xVel = object->scratchS16[SCRATCHS16_FORCE];
						player->x.pos += 8;
						player->status.xFlip = true;
					}
					else
					{
						player->xVel = -object->scratchS16[SCRATCHS16_FORCE];
						player->x.pos -= 8;
						player->status.xFlip = false;
					}
					
					//Handle player launching on the ground (lock controls for a bit, play walk animation)
					player->moveLock = 15;
					player->inertia = player->xVel;
					if (!player->status.inBall)
						player->anim = PLAYERANIMATION_WALK;
					
					#ifdef HORIZONTAL_UPSIDEDOWN_FIX
						if ((player->angle + 0x40) & 0x80)
						{
							//If upside down, invert inertia and xFlip
							player->inertia = -player->inertia;
							player->status.xFlip = !player->status.xFlip;
						}
					#endif
					
					if (object->subtype & 0x80) //Unused flag?
						player->yVel = 0;
					
					if (object->subtype & 0x1) //Twirl flag
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & 0x2) ? 1 : 3; //Do 2 extra flips if a red spring
						player->flipSpeed = 8;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & 0xC) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & 0xC) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//Clear pushing and play spring sound
					player->status.pushing = false;
					object->playerContact[i].pushing = false;
					PlaySound(SOUNDID_SPRING);
				}
			}
			
			object->Animate(animationList);
			object->DrawInstance(object->renderFlags, object->texture, object->mappings, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
			break;
		}
	}
}
