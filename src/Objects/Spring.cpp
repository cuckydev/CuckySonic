#include <stdint.h>
#include "../Level.h"
#include "../LevelCollision.h"
#include "../Game.h"
#include "../Log.h"

//Animations
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

//Slope arrays
static const int8_t upDiagonalSlope[] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,  -1,  -2,
	 -3,  -4,  -5,  -6,  -7,  -8,  -9, -10,
	-11, -12, -13, -14, -15, -16, -17, -18,
	-19, -20, -20, -20, -20, -20, -20, -20,
	-20, -20, -20, -20, -20, -20,
};

static const int8_t downDiagonalSlope[] = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  1,  2,
	 3,  4,  5,  6,  7,  8,  9, 10,
	11, 12, 13, 14, 15, 16, 17, 18,
	19, 20, 20, 20, 20, 20, 20, 20,
	20, 20, 20, 20, 20, 20,
};

//Spring function
void ObjSpring(OBJECT *object)
{
	#define MASK_TWIRL				0x01
	#define MASK_IS_YELLOW			0x02
	#define MASK_LAYER				0x0C
	#define MASK_CANCEL_TRAJECTORY	0x80
	
	//Get our force
	int16_t force = -0x1000; //Red spring's force
	if (object->subtype & MASK_IS_YELLOW)
		force = -0x0A00; //Yellow spring's force
	
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
			object->texture = gLevel->GetObjectTexture("data/Object/Generic.bmp");
			if (object->subtype & MASK_IS_YELLOW)
				object->mapping.mappings = gLevel->GetObjectMappings("data/Object/YellowSpring.map");
			else
				object->mapping.mappings = gLevel->GetObjectMappings("data/Object/RedSpring.map");
			
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
			break;
		}
		case ROUTINE_UP:
		{
			//Act as solid
			object->SolidObjectFull(27, 8, 16, object->x.pos, true, nullptr, false);
			
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player and check if we touched the spring
				PLAYER *player = gLevel->playerList[i];
				
				if (object->playerContact[i].standing)
				{
					//Play bouncing animation
					object->anim = 1;
					object->prevAnim = 0;
					
					//Launch player
					if (player->status.reverseGravity)
						player->y.pos -= 8;
					else
						player->y.pos += 8;
					player->yVel = force;
					player->anim = PLAYERANIMATION_SPRING;
					
					//Make us airborne
					player->status.jumping = false;
					player->spindashing = false;
					player->status.pinballMode = false;
					player->status.inAir = true;
					player->status.shouldNotFall = false;
					player->routine = PLAYERROUTINE_CONTROL;
					
					//Cancel trajectory if flag is set
					if (object->subtype & MASK_CANCEL_TRAJECTORY)
						player->yVel = 0;
					
					//Twirl if flag is set
					if (object->subtype & MASK_TWIRL)
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & MASK_IS_YELLOW) ? 0 : 1; //Do an extra flip if a red spring
						player->flipSpeed = 4;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & MASK_LAYER) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & MASK_LAYER) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//Play spring sound
					PlaySound(SOUNDID_SPRING);
				}
			}
			break;
		}
		case ROUTINE_HORIZONTAL:
		{
			//Act as solid
			OBJECT_SOLIDTOUCH touch = object->SolidObjectFull(19, 14, 15, object->x.pos, true, nullptr, false);
			
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player and check if we touched the spring
				PLAYER *player = gLevel->playerList[i];
				
				if (touch.side[i])
				{
					//Make sure we're on the right side
					int16_t xDiff = object->x.pos - player->x.pos;
					if (xDiff >= 0 ? !object->status.xFlip : object->status.xFlip)
						continue;
					
					//Play bouncing animation
					object->anim = 3;
					object->prevAnim = 0;
					
					//Launch player
					if (object->status.xFlip)
					{
						player->x.pos += 8;
						player->xVel = force;
						player->status.xFlip = true;
					}
					else
					{
						player->x.pos -= 8;
						player->xVel = -force;
						player->status.xFlip = false;
					}
					
					//Handle ground movement and animation
					player->moveLock = 15;
					player->inertia = player->xVel;
					if (0) //(!object->status.bit2)
						player->anim = PLAYERANIMATION_WALK;
					
					//Cancel trajectory if flag is set
					if (object->subtype & MASK_CANCEL_TRAJECTORY)
						player->xVel = 0;
					
					//Twirl if flag is set
					if (object->subtype & MASK_TWIRL)
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & MASK_IS_YELLOW) ? 1 : 3; //Do some extra flips if a red spring
						player->flipSpeed = 8;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & MASK_LAYER) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & MASK_LAYER) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//Play spring sound
					PlaySound(SOUNDID_SPRING);
				}
			}
			break;
		}
		case ROUTINE_DOWN:
		{
			//Act as solid
			OBJECT_SOLIDTOUCH touch = object->SolidObjectFull(27, 8, 9, object->x.pos, true, nullptr, false);
			
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player and check if we touched the spring
				PLAYER *player = gLevel->playerList[i];
				
				if (touch.bottom[i])
				{
					//Offset player position
					if (player->status.reverseGravity)
						player->y.pos += 8;
					else
						player->y.pos -= 8;
					
					//Play bouncing animation
					object->anim = 1;
					object->prevAnim = 0;
					
					//Launch player
					player->yVel = -force;
					if (player->yVel != 0x1000)
						player->yVel = 0xD00; //WTF?
					
					//Cancel trajectory if flag is set
					if (object->subtype & MASK_CANCEL_TRAJECTORY)
						player->yVel = 0;
					
					//Twirl if flag is set
					if (object->subtype & MASK_TWIRL)
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & MASK_IS_YELLOW) ? 0 : 1; //Do an extra flip if a red spring
						player->flipSpeed = 4;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & MASK_LAYER) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & MASK_LAYER) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//Play spring sound
					PlaySound(SOUNDID_SPRING);
				}
			}
			break;
		}
		case ROUTINE_DIAGONALLY_UP:
		{
			//Act as solid
			object->SolidObjectFull(27, 16, 16, object->x.pos, true, upDiagonalSlope, false);
			
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player and check if we touched the spring
				PLAYER *player = gLevel->playerList[i];
				
				if (object->playerContact[i].standing)
				{
					//Make sure we're on the sloping / spring part
					if (!object->status.xFlip)
					{
						if ((object->x.pos - 4) >= player->x.pos)
							continue;
					}
					else
					{
						if ((object->x.pos + 4) <= player->x.pos)
							continue;
					}
					
					//Play bouncing animation
					object->anim = 5;
					object->prevAnim = 0;
					
					//Launch player
					player->yVel = force;
					player->y.pos += 6;
					
					if (object->status.xFlip)
					{
						player->status.xFlip = true;
						player->x.pos += 6;
						player->xVel = force;
					}
					else
					{
						player->status.xFlip = false;
						player->x.pos -= 6;
						player->xVel = -force;
					}
					
					//Make us airborne
					player->status.inAir = true;
					player->status.shouldNotFall = false;
					player->status.jumping = false;
					player->anim = PLAYERANIMATION_SPRING;
					player->routine = PLAYERROUTINE_CONTROL;
					
					//Twirl if flag is set
					if (object->subtype & MASK_TWIRL)
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & MASK_IS_YELLOW) ? 1 : 3; //Do an extra flip if a red spring
						player->flipSpeed = 8;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & MASK_LAYER) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & MASK_LAYER) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//Play spring sound
					PlaySound(SOUNDID_SPRING);
				}
			}
			break;
		}
		case ROUTINE_DIAGONALLY_DOWN:
		{
			//Act as solid
			OBJECT_SOLIDTOUCH touch = object->SolidObjectFull(27, 16, 16, object->x.pos, true, downDiagonalSlope, false);
			
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player and check if we touched the spring
				PLAYER *player = gLevel->playerList[i];
				
				if (touch.bottom[i])
				{
					//Play bouncing animation
					object->anim = 5;
					object->prevAnim = 0;
					
					//Launch player
					player->yVel = -force;
					player->y.pos -= 6;
					
					if (object->status.xFlip)
					{
						player->status.xFlip = true;
						player->x.pos += 6;
						player->xVel = force;
					}
					else
					{
						player->status.xFlip = false;
						player->x.pos -= 6;
						player->xVel = -force;
					}
					
					//Make us airborne
					player->status.inAir = true;
					player->status.shouldNotFall = false;
					player->status.jumping = false;
					player->routine = PLAYERROUTINE_CONTROL;
					
					//Twirl if flag is set
					if (object->subtype & MASK_TWIRL)
					{
						//Put the player into the flipping animation
						player->inertia = 1;
						player->flipAngle = 1;
						player->anim = PLAYERANIMATION_WALK;
						player->flipsRemaining = (object->subtype & MASK_IS_YELLOW) ? 1 : 3; //Do an extra flip if a red spring
						player->flipSpeed = 8;
						
						//Reverse if facing left
						if (player->status.xFlip)
						{
							player->flipAngle = ~player->flipAngle;
							player->inertia = -player->inertia;
						}
					}
					
					//Switch player to different layers if set
					if ((object->subtype & MASK_LAYER) == 4)
					{
						player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
					}
					
					if ((object->subtype & MASK_LAYER) == 8)
					{
						player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
						player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
					}
					
					//Play spring sound
					PlaySound(SOUNDID_SPRING);
				}
			}
			break;
		}
	}
	
	//Draw and animate
	object->Animate(animationList);
	object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
	object->UnloadOffscreen(object->x.pos);
}
