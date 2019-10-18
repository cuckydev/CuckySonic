#include <string.h>

#include "Player.h"
#include "MathUtil.h"
#include "Audio.h"
#include "Filesystem.h"
#include "Log.h"
#include "Error.h"
#include "Level.h"
#include "Game.h"
#include "GameConstants.h"

#include "Object.h"
#include "Objects.h"

//Bug-fixes
//#define FIX_SPINDASH_JUMP      //When you jump the frame after you spindash, you'll jump straight upwards
//#define FIX_HORIZONTAL_WRAP    //In the originals, for some reason, the LevelBound uses unsigned checks, meaning if you go off to the left, you'll be sent to the right boundary
//#define FIX_DUCK_CONDITION     //In Sonic and Knuckles, the conditions for ducking are so loose, you can duck (and spindash) in unexpected situations.
//#define FIX_ROLL_YSHIFT        //In the originals, when you roll, you're always shifted up / down globally, this can cause weird behaviour such as falling off of ceilings
//#define FIX_ROLLJUMP_COLLISION //In the originals, for some reason, jumping from a roll will maintain Sonic's regular collision hitbox, rather than switching to the smaller hitbox, which causes weird issues.

//Bug-fix macros
#ifdef FIX_ROLL_YSHIFT
	#define YSHIFT_ON_FLOOR(shift)	\
		uint8_t offAngle2 = angle;	\
		if (offAngle2 < 0x80)	\
			offAngle2--;	\
			\
		switch ((angle + 0x20) & 0xC0)	\
		{	\
			case 0x00:	\
				if (status.reverseGravity)	\
					y.pos -= shift;	\
				else	\
					y.pos += shift;	\
				break;	\
			case 0x40:	\
				x.pos -= shift;	\
				break;	\
			case 0x80:	\
				if (status.reverseGravity)	\
					y.pos += shift;	\
				else	\
					y.pos -= shift;	\
				break;	\
			case 0xC0:	\
				x.pos += shift;	\
				break;	\
		}
#else
	#define YSHIFT_ON_FLOOR(shift)	\
		if (status.reverseGravity)	\
			y.pos -= shift;	\
		else	\
			y.pos += shift;
#endif

//Game differences
//#define SONIC1_SLOPE_ANGLE          //In Sonic 2+, the floor's angle will be replaced with the player's cardinal floor angle if there's a 45+ degree difference
//#define SONIC12_PUSH_CHECK          //In Sonic 3, it was changed so that you have to be facing towards a wall in order to start pushing into it
//#define SONIC12_SANE_AIRCOLLISION   //For some reason, in Sonic 3 there was a weird modification to the airborne collision code... can't understand the purpose
//#define SONIC123_WALL_COLLISION     //In Sonic and Knuckles, the wall collision on the ground was changed to have collision even on walls and ceilings (as long as they're cardinal directions)
//#define SONIC12_ROLLJUMP_LAND       //In Sonic 3, they fixed the roll jump landing bug, where you'd land 5 pixels above the ground after jumping from a roll

//#define SONIC1_WALK_ANIMATION       //For some reason, in Sonic 2+, the animation code was messed up, making the first frame of the walk animation last only one frame
//#define SONIC1_SLOPE_ROTATION       //In Sonic 2+, a few lines were added to the animation code to make the floor rotation more consistent

//#define SONIC12_SLOPE_RESIST        //In Sonic 3, they made it so you're always affected by slope gravity unless you're on a shallow floor
//#define SONIC12_SLOPE_REPEL         //In Sonic 3, the code to make it so you fall off of walls and ceilings when going too slow was completely redone
//#define SONIC1_GROUND_CAP           //In Sonic 1, your speed on the ground is capped to your top speed when above it, even if you're already above it
//#define SONIC12_AIR_CAP             //In Sonic 1 and 2, your speed in the air is capped to your top speed when above it, even if you're already above it
//#define SONIC123_ROLL_DUCK          //In Sonic and Knuckles, they added a greater margin of speed for ducking and rolling, so you can duck while moving
//#define SONICCD_ROLLING             //In Sonic CD, rolling to the right is weird
//#define SONICCD_ROLLJUMP            //In Sonic CD, rolljumping was *partially* removed, the below "CONTROL_NO_ROLLJUMP_LOCK" would act differently

//#define SONIC1_NO_SPINDASH          //The spindash, it needs no introduction
//#define SONICCD_SPINDASH            //CD spindash
//#define SONICCD_PEELOUT             //CD super-peelout
//#define SONIC1_NO_SUPER             //Super Sonic wasn't in Sonic 1
//#define SONIC123_NO_HYPER           //DOES NOTHING, UNIMPLEMENTED! - Hyper Sonic wasn't introduced until S3K
//#define SONIC2_SUPER_AT_PEAK        //In Sonic 2, you'd turn super at the peak of a jump, no matter what, while in Sonic 3, this was moved to the jump ability code
//#define SONIC12_NO_INSTASHIELD      //Insta-shield
//#define SONIC12_NO_SHIELD_ABILITIES //Other shield abilities

//#define SONIC1_DEATH_BOUNDARY       //In Sonic 2, the death boundary code was fixed so that it doesn't use the camera's boundary but the level boundary, so that you don't die while the camera boundary is scrolling
//#define SONIC12_DEATH_RESPAWN       //In Sonic 3, it was changed so that death respawns you once you go off-screen, not when you leave the level boundaries, since this was a very buggy check
//#define SONIC2_SPINDASH_ANIM_BUG    //In Sonic 3, the bug where landing on the ground while spindashing plays the walk animation was fixed

//Other control options
//#define CONTROL_NO_ROLLJUMP_LOCK          //In the originals, jumping from a roll will lock your controls
//#define CONTROL_JA_DONT_CLEAR_ROLLJUMP    //When you use a jump ability in the original, it clears the roll-jump flag

//Animation data
#define WALK_FRAMES			8
#define RUN_FRAMES			4

#define MAPPINGFRAME_FLIP1 95

static const uint8_t animationWalk[] =			{0xFF,0x0F,0x10,0x11,0x12,0x13,0x14,0x0D,0x0E,0xFF}; //Walk and run must match in length (run is padded with 0xFF, here)
static const uint8_t animationRun[] =			{0xFF,0x2D,0x2E,0x2F,0x30,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t animationRoll[] =			{0xFE,0x3D,0x41,0x3E,0x41,0x3F,0x41,0x40,0x41,0xFF}; //Roll and roll2 must match in length
static const uint8_t animationRoll2[] =			{0xFE,0x3D,0x41,0x3E,0x41,0x3F,0x41,0x40,0x41,0xFF};
static const uint8_t animationPush[] =			{0xFD,0x48,0x49,0x4A,0x4B,0xFF,0xFF,0xFF,0xFF,0xFF}; //Push must also match the length of run and walk (padded with 0xFF here)
static const uint8_t animationIdle[] =			{0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
												 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
												 0x01,0x02,0x03,0x03,0x03,0x03,0x03,0x04,0x04,0x04,0x05,0x05,0x05,0x04,0x04,
												 0x04,0x05,0x05,0x05,0x04,0x04,0x04,0x05,0x05,0x05,0x04,0x04,0x04,0x05,0x05,
												 0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x04,0x04,0x04,0x05,
												 0x05,0x05,0x04,0x04,0x04,0x05,0x05,0x05,0x04,0x04,0x04,0x05,0x05,0x05,0x04,
												 0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,
												 0x04,0x04,0x04,0x05,0x05,0x05,0x04,0x04,0x04,0x05,0x05,0x05,0x04,0x04,0x04,
												 0x05,0x05,0x05,0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,0x06,0x06,0x06,0x06,
												 0x06,0x06,0x06,0x06,0x04,0x04,0x04,0x05,0x05,0x05,0x04,0x04,0x04,0x05,0x05,
												 0x05,0x04,0x04,0x04,0x05,0x05,0x05,0x04,0x04,0x04,0x05,0x05,0x05,0x06,0x06,
												 0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x08,0x08,0x08,0x09,0x09,0x09,
												 0xFE,0x06};
static const uint8_t animationBalance1[] =		{0x09,0xCC,0xCD,0xCE,0xCD,0xFF};
static const uint8_t animationLookUp[] =		{0x05,0x0B,0x0C,0xFE,0x01};
static const uint8_t animationDuck[] =			{0x05,0x4C,0x4D,0xFE,0x01};
static const uint8_t animationSpindash[] =		{0x00,0x42,0x43,0x42,0x44,0x42,0x45,0x42,0x46,0x42,0x47,0xFF};
static const uint8_t animationBlink[] =			{0x01,0x02,0xFD,PLAYERANIMATION_WALK};
static const uint8_t animationGetUp[] =			{0x03,0x0A,0xFD,PLAYERANIMATION_WALK};
static const uint8_t animationBalance2[] =		{0x03,0xC8,0xC9,0xCA,0xCB,0xFF};
static const uint8_t animationSkid[] =			{0x05,0xD2,0xD3,0xD4,0xD5,0xFD,PLAYERANIMATION_WALK};
static const uint8_t animationFloat1[] =		{0x07,0x54,0x59,0xFF};
static const uint8_t animationFloat2[] =		{0x07,0x54,0x55,0x56,0x57,0x58,0xFF};
static const uint8_t animationSpring[] =		{0x2F,0x5B,0xFD,PLAYERANIMATION_WALK};
static const uint8_t animationHang1[] =			{0x01,0x50,0x51,0xFF};
static const uint8_t animationDash2[] =			{0x0F,0x43,0x43,0x43,0xFE,0x01};
static const uint8_t animationDash3[] =			{0x0F,0x43,0x44,0xFE,0x01};
static const uint8_t animationHang2[] =			{0x01,0x50,0x51,0xFF};
static const uint8_t animationBubble[] =		{0x0B,0x5A,0x5A,0x11,0x12,0xFD,PLAYERANIMATION_WALK};
static const uint8_t animationBurnt[] =			{0x20,0x5E,0xFF};
static const uint8_t animationDrown[] =			{0x20,0x5D,0xFF};
static const uint8_t animationDeath[] =			{0x20,0x5C,0xFF};
static const uint8_t animationHurt[] =			{0x40,0x4E,0xFF};
static const uint8_t animationSlide[] =			{0x09,0x4E,0x4F,0xFF};
static const uint8_t animationNull[] =			{0x77,0x00,0xFD,PLAYERANIMATION_WALK};
static const uint8_t animationBalance3[] =		{0x13,0xD0,0xD1,0xFF};
static const uint8_t animationBalance4[] =		{0x03,0xCF,0xC8,0xC9,0xCA,0xCB,0xFE,0x04};
static const uint8_t animationLying[] =			{0x09,0x08,0x09,0xFF};
static const uint8_t animationLieDown[] =		{0x03,0x07,0xFD,PLAYERANIMATION_WALK};

//Super specific animation
#define SUPER_FLY_FRAMES	1

static const uint8_t animationSuperWalk[] =			{0xFF,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x75,0x76,0xFF}; //Walk and run must match in length (run is padded with 0xFF, here)
static const uint8_t animationSuperRun[] =			{0xFF,0xB5,0xB9,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
static const uint8_t animationSuperPush[] =			{0xFD,0xBD,0xBE,0xBF,0xC0,0xFF,0xFF,0xFF,0xFF,0xFF}; //Push must also match the length of run and walk (padded with 0xFF here)
static const uint8_t animationSuperIdle[] =			{0x07,0x72,0x73,0x74,0x73,0xFF};
static const uint8_t animationSuperBalance[] =		{0x09,0xC2,0xC3,0xC4,0xC3,0xC5,0xC6,0xC7,0xC6,0xFF};
static const uint8_t animationSuperDuck[] =			{0x05,0xC1,0xFF};
static const uint8_t animationSuperTransform[] =	{0x02,0x6D,0x6D,0x6E,0x6E,0x6F,0x70,0x71,0x70,0x71,0x70,0x71,0x70,0x71,0xFD,PLAYERANIMATION_WALK};

//Animation lists
static const uint8_t* animationList[] = {
	animationWalk,
	animationRun,
	animationRoll,
	animationRoll2,
	animationPush,
	animationIdle,
	animationBalance1,
	animationLookUp,
	animationDuck,
	animationSpindash,
	animationBlink,
	animationGetUp,
	animationBalance2,
	animationSkid,
	animationFloat1,
	animationFloat2,
	animationSpring,
	animationHang1,
	animationDash2,
	animationDash3,
	animationHang2,
	animationBubble,
	animationBurnt,
	animationDrown,
	animationDeath,
	animationHurt,
	animationSlide,
	animationNull,
	animationBalance3,
	animationBalance4,
	animationLying,
	animationLieDown,
	animationSuperTransform,
};

static const uint8_t* animationListSuper[] = {
	animationSuperWalk,
	animationSuperRun,
	animationRoll,
	animationRoll2,
	animationSuperPush,
	animationSuperIdle,
	animationSuperBalance,
	animationLookUp,
	animationSuperDuck,
	animationSpindash,
	animationBlink,
	animationGetUp,
	animationBalance2,
	animationSkid,
	animationFloat1,
	animationFloat2,
	animationSpring,
	animationHang1,
	animationDash2,
	animationDash3,
	animationHang2,
	animationBubble,
	animationBurnt,
	animationDrown,
	animationDeath,
	animationHurt,
	animationSlide,
	animationNull,
	animationBalance3,
	animationBalance4,
	animationLying,
	animationLieDown,
	animationSuperTransform,
};

//Spindash dust
static const uint8_t animationSpindashDustNull[] =	{0x1F,0x00,0xFF};
static const uint8_t animationSpindashDust[] =		{0x01,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0xFF};

static const uint8_t* animationListSpindashDust[] = {
	animationSpindashDustNull,
	animationSpindashDust,
};

void ObjSpindashDust(OBJECT *object)
{
	PLAYER *player = (PLAYER*)object->parent;
	if (player == nullptr)
		return;
	
	switch (object->routine)
	{
		case 0:
			//Initialize render properties
			object->texture = gLevel->GetObjectTexture("data/Object/SpindashDust.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/SpindashDust.map");
			
			object->priority = 1;
			object->widthPixels = 24;
			object->renderFlags.alignPlane = true;
			
			//Increment routine
			object->routine = 1;
//Fallthrough
		case 1:
			//If we're active
			if (object->anim == 1)
			{
				//Is the player still spindashing?
				if (player->routine != PLAYERROUTINE_CONTROL || player->spindashing == false)
				{
					object->anim = 0;
					break;
				}
				
				//Copy the player's position
				object->status.xFlip = player->status.xFlip;
				object->status.yFlip = player->status.reverseGravity;
				object->highPriority = player->highPriority;
				
				object->x.pos = player->x.pos;
				object->y.pos = player->y.pos;
				
				//Offset if our height is atypical (for a short character like Tails)
				int heightDifference = 19 - player->defaultYRadius;
				
				if (player->status.reverseGravity)
					object->y.pos += heightDifference;
				else
					object->y.pos -= heightDifference;
			}
			break;
	}
	
	//Draw and animate
	object->Animate(animationListSpindashDust);
	object->Draw();
}

//Skid dust
static const uint8_t animationSkidDustNull[] =	{0x1F,0x00,0xFF};
static const uint8_t animationSkidDust[] =		{0x03,0x01,0x02,0x03,0x04,0xFC};

static const uint8_t* animationListSkidDust[] = {
	animationSkidDustNull,
	animationSkidDustNull,
	animationSkidDust,
};

void ObjSkidDust(OBJECT *object)
{
	PLAYER *player = (PLAYER*)object->parent;
	
	switch (object->routine)
	{
		case 0:
			//Initialize render properties
			object->texture = gLevel->GetObjectTexture("data/Object/SkidDust.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/SkidDust.map");
			
			object->priority = 1;
			object->widthPixels = 4;
			object->renderFlags.alignPlane = true;
			
			//Set our routine 
			object->routine = (object->anim == 2) ? 2 : 1;
			break;
		case 1: //Dust controller
			//Don't run if there's no parent
			if (player == nullptr)
				break;
			
			//If we're active
			if (object->anim == 1)
			{
				//Is the player still skidding?
				if (player->anim != PLAYERANIMATION_SKID)
				{
					object->anim = 0;
					break;
				}
				
				//Spawn skid dust every 4 frames
				if (--object->animFrameDuration < 0)
				{
					//Reset timer
					object->animFrameDuration = 3;
					
					//Create a new dust object at the player's feet
					OBJECT *dust = new OBJECT(&gLevel->objectList, &ObjSkidDust);
					dust->x.pos = player->x.pos;
					dust->y.pos = player->y.pos + (player->status.reverseGravity ? -16 : 16);
					dust->highPriority = player->highPriority;
					dust->anim = 2;
					
					//Offset if our height is atypical (for a short character like Tails)
					int heightDifference = 19 - player->defaultYRadius;
					
					if (player->status.reverseGravity)
						object->y.pos += heightDifference;
					else
						object->y.pos -= heightDifference;
				}
			}
			break;
		case 2: //Dust instance
			object->Animate(animationListSkidDust);
			object->Draw();
			break;
		case 3: //Dust deleting
			object->deleteFlag = true;
			break;
	}
}

//Shield animation (insta-shield)
static const uint8_t animationInstaShieldNull[] =	{0x1F,0x06,0xFF};
static const uint8_t animationInstaShieldUse[] =	{0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x06,0x06,0x06,0x06,0x06,0x06,0x07,0xFD,0x00};

static const uint8_t* animationListInstaShield[] = {
	animationInstaShieldNull,
	animationInstaShieldUse,
};

//Shield animation (blue shield)
static const uint8_t animationBlueShield[] =			{0x00,0x05,0x00,0x05,0x01,0x05,0x02,0x05,0x03,0x05,0x04,0xFF};

static const uint8_t* animationListBlueShield[] = {
	animationBlueShield,
};

//Shield animation (fire shield)
static const uint8_t animationFireShieldIdle[] =		{0x01,0x00,0x0F,0x01,0x10,0x02,0x11,0x03,0x12,0x04,0x13,0x05,0x14,0x06,0x15,0x07,0x16,0x08,0x17,0xFF};
static const uint8_t animationFireShieldUse[] =			{0x01,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0xFD,0x00,0x00};

static const uint8_t* animationListFireShield[] = {
	animationFireShieldIdle,
	animationFireShieldUse,
};

//Shield animation (electric shield)
static const uint8_t animationElectricShieldIdle[] =	{0x01,0x00,0x00,0x01,0x01,0x02,0x02,0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,0x07,0x07,0x08,0x08,0x09,0x0A,0x0B,0x16,0x16,0x15,0x15,0x14,0x14,0x13,0x13,0x12,0x12,0x11,0x11,0x10,0x10,0x0F,0x0F,0x0E,0x0E,0x09,0x0A,0x0B,0xFF};
static const uint8_t animationElectricShieldUse[] =		{0x00,0x0C,0x0D,0x17,0x0C,0x0D,0x17,0x0C,0x0D,0x17,0x0C,0x0D,0x17,0x0C,0x0D,0x17,0x0C,0x0D,0x17,0x0C,0x0D,0xFC,0xFF};
static const uint8_t animationElectricShield2[] =		{0x03,0x00,0x01,0x02,0xFC,0xFF,0x00};

static const uint8_t* animationListElectricShield[] = {
	animationElectricShieldIdle,
	animationElectricShieldUse,
	animationElectricShield2,
};

//Shield animation (bubble shield)
static const uint8_t animationBubbleShieldIdle[] =		{0x01,0x00,0x09,0x00,0x09,0x00,0x09,0x01,0x0A,0x01,0x0A,0x01,0x0A,0x02,0x09,0x02,0x09,0x02,0x09,0x03,0x0A,0x03,0x0A,0x03,0x0A,0x04,0x09,0x04,0x09,0x04,0x09,0x05,0x0A,0x05,0x0A,0x05,0x0A,0x06,0x09,0x06,0x09,0x06,0x09,0x07,0x0A,0x07,0x0A,0x07,0x0A,0x08,0x09,0x08,0x09,0x08,0x09,0xFF};
static const uint8_t animationBubbleShieldUse[] =		{0x05,0x09,0x0B,0x0B,0x0B,0xFD,0x00};
static const uint8_t animationBubbleShieldSquish[] =	{0x05,0x0C,0x0C,0x0B,0xFD,0x00,0x00};

static const uint8_t* animationListBubbleShield[] = {
	animationBubbleShieldIdle,
	animationBubbleShieldUse,
	animationBubbleShieldSquish,
};

//Shield, invincibility stars, and super stars object
//#define FIX_WEIRD_SUPER_STAR_TRACKING   //For some reason, when the super stars first appear when you reach the required speed, they follow the player until it loops, where it then trails behind

void ObjShield(OBJECT *object)
{
	enum ROUTINE
	{
		ROUTINE_SUPERSTAR =			(uint8_t)-1,
		ROUTINE_INVINCIBILITYSTAR =	(uint8_t)-2,
	};
	
	//Scratch
	enum SCRATCH
	{
		//U8
		SCRATCHU8_MOVING =		0,
		SCRATCHU8_NO_COPY_POS =	1,
		SCRATCHU8_MAX =			2,
	};
	
	object->ScratchAllocU8(SCRATCHU8_MAX);
	
	//Get our parent player
	PLAYER *player = (PLAYER*)object->parent;
	if (player == nullptr)
		return;
	
	//Handle super stars
	if (player->super)
	{
		//Restart if wasn't super stars
		if (object->routine != ROUTINE_SUPERSTAR)
		{
			object->routine = ROUTINE_SUPERSTAR;
			object->mappingFrame = 0;
			object->scratchU8[SCRATCHU8_MOVING] = 0;
			object->scratchU8[SCRATCHU8_NO_COPY_POS] = 0;
		}
		
		//Load the given mappings and textures
		object->texture = gLevel->GetObjectTexture("data/Object/InvincibilitySuperStars.bmp");
		object->mappings = gLevel->GetObjectMappings("data/Object/SuperStars.map");
		
		//Set our render properties
		object->priority = 1;
		object->widthPixels = 24;
		object->heightPixels = 24;
		object->renderFlags.alignPlane = true;
		object->highPriority = player->highPriority;
		
		if (object->scratchU8[SCRATCHU8_MOVING])
		{
			//Run animation
			if (--object->animFrameDuration < 0)
			{
				//Run next animation frame, and check for looping (and handle it appropriately)
				object->animFrameDuration = 1;
				
				if (++object->mappingFrame >= 6)
				{
					//Loop to first frame and update our state
					object->mappingFrame = 0;
					object->scratchU8[SCRATCHU8_MOVING] = 0;
					object->scratchU8[SCRATCHU8_NO_COPY_POS] = 1;
				}
			}
			
			//Copy position (if SCRATCHU8_NO_COPY_POS is clear), and draw to screen
			if (!object->scratchU8[SCRATCHU8_NO_COPY_POS])
			{
				object->x.pos = player->x.pos;
				object->y.pos = player->y.pos;
			}
			
			object->Draw();
		}
		else
		{
			//Wait for the player to have a high enough ground velocity
			if (player->objectControl.disableOurMovement || player->objectControl.disableObjectInteract || abs(player->inertia) < 0x800)
			{
				//Stay inactive
				object->scratchU8[SCRATCHU8_MOVING] = 0;
				object->scratchU8[SCRATCHU8_NO_COPY_POS] = 0;
			}
			else
			{
				//Become active, and draw to screen at player position
				object->mappingFrame = 0;
				object->scratchU8[SCRATCHU8_MOVING] = 1;
				#ifdef FIX_WEIRD_SUPER_STAR_TRACKING
					object->scratchU8[SCRATCHU8_NO_COPY_POS] = 1;
				#endif
				object->x.pos = player->x.pos;
				object->y.pos = player->y.pos;
				object->Draw();
			}
		}
	}
	//TODO: Invincibility stars
	else if (player->item.isInvincible)
	{
		//Restart if wasn't invincibility stars
		if (object->routine != ROUTINE_INVINCIBILITYSTAR)
			object->routine = ROUTINE_INVINCIBILITYSTAR;
	}
	//Shield
	else
	{
		//Reset if wasn't the shield we are now
		if ((SHIELD)object->routine != player->shield)
		{
			//Copy our current shield
			object->routine = player->shield;
			
			//Reset animation state
			object->anim = 0;
			object->prevAnim = 0;
			object->animFrame = 0;
			object->animFrameDuration = 0;
		}
		
		//Copy player position and high priority flag
		object->highPriority = player->highPriority;
		object->x.pos = player->x.pos;
		object->y.pos = player->y.pos;
		
		//Do shield specific code (this includes getting our things)
		const char *useMapping = nullptr;
		const uint8_t **useAniList = nullptr;
		
		switch (player->shield)
		{
			case SHIELD_BLUE:
				//Use blue shield mappings and animations
				useMapping = "data/Object/BlueShield.map";
				useAniList = animationListBlueShield;
				
				//Set our render properties
				object->priority = 1;
				object->widthPixels = 24;
				object->heightPixels = 24;
				object->renderFlags.alignPlane = true;
				break;
			case SHIELD_FIRE:
				//Use fire shield mappings and animations
				useMapping = "data/Object/FireShield.map";
				useAniList = animationListFireShield;
				
				//Set our render properties
				object->priority = 1;
				object->widthPixels = 24;
				object->heightPixels = 24;
				object->renderFlags.alignPlane = true;
				
				//Copy orientation (if not in dash state)
				if (object->anim == 0)
				{
					object->status.xFlip = player->status.xFlip;
					object->status.yFlip = player->status.reverseGravity;
				}
				
				//Extinguish once in water
				if (player->status.underwater)
				{
					player->shield = SHIELD_NULL;
					player->item.shieldReflect = false;
					player->item.immuneFire = false;
					return;
				}
				break;
			case SHIELD_ELECTRIC:
				//Use electric shield mappings and animations
				useMapping = "data/Object/ElectricShield.map";
				useAniList = animationListElectricShield;
				
				//Set our render properties
				object->priority = 1;
				object->widthPixels = 24;
				object->heightPixels = 24;
				object->renderFlags.alignPlane = true;
				
				//Copy orientation
				object->status.xFlip = player->status.xFlip;
				object->status.yFlip = player->status.reverseGravity;
				
				//Clear animation if non-zero
				if (object->anim)
					object->anim = 0;
				
				//Extinguish once in water
				if (player->status.underwater)
				{
					player->shield = SHIELD_NULL;
					player->item.shieldReflect = false;
					player->item.immuneFire = false;
					return;
				}
				break;
			case SHIELD_BUBBLE:
				//Use bubble shield mappings and animations
				useMapping = "data/Object/BubbleShield.map";
				useAniList = animationListBubbleShield;
				
				//Set our render properties
				object->priority = 1;
				object->widthPixels = 24;
				object->heightPixels = 24;
				object->renderFlags.alignPlane = true;
				break;
			default: //Insta-shield
				//Use insta-shield mappings and animations
				useMapping = "data/Object/InstaShield.map";
				useAniList = animationListInstaShield;
				
				//Set our render properties
				object->priority = 1;
				object->widthPixels = 24;
				object->heightPixels = 24;
				object->renderFlags.alignPlane = true;
				
				//Copy orientation
				object->status.xFlip = player->status.xFlip;
				object->status.yFlip = player->status.reverseGravity;
				
				//When we reach the end of the animation, end our attack
				if (object->mappingFrame == 7 && player->jumpAbility == 1)
					player->jumpAbility = 2;
				break;
		}
		
		//Load the given mappings and textures
		object->texture = gLevel->GetObjectTexture("data/Object/Shield.bmp");
		object->mappings = gLevel->GetObjectMappings(useMapping);
		
		//Animate
		object->Animate(useAniList);
		
		//Do post-animation shield specific code (specifically priority stuff)
		switch (player->shield)
		{
			case SHIELD_FIRE:
				//Check if we should be drawn behind the player
				if (object->mappingFrame < 0x0F)
					object->priority = 1;
				else
					object->priority = 4;
				break;
			case SHIELD_ELECTRIC:
				//Check if we should be drawn behind the player
				if (object->mappingFrame < 0x0E)
					object->priority = 1;
				else
					object->priority = 4;
				break;
			default:
				break;
		}
		
		//Draw to screen
		object->Draw();
	}
}

//Player class
#define READ_SPEEDDEFINITION(definition)	definition.top = ReadFile_BE16(playerSpec); definition.acceleration = ReadFile_BE16(playerSpec); definition.deceleration = ReadFile_BE16(playerSpec); definition.rollDeceleration = ReadFile_BE16(playerSpec); definition.jumpForce = ReadFile_BE16(playerSpec); definition.jumpRelease = ReadFile_BE16(playerSpec);

PLAYER::PLAYER(PLAYER **linkedList, const char *specPath, PLAYER *myFollow, int myController)
{
	LOG(("Creating a player with spec %s and controlled by controller %d...\n", specPath, myController));
	memset(this, 0, sizeof(PLAYER));
	
	//Equivalent of routine 0
	routine = PLAYERROUTINE_CONTROL;
	
	//Load art and mappings
	char *texPath = AllocPath(specPath, ".bmp", nullptr);
	texture = gLevel->GetObjectTexture(texPath);
	delete texPath;
	
	if (texture->fail != nullptr)
	{
		Error(fail = texture->fail);
		return;
	}
	
	char *mapPath = AllocPath(specPath, ".map", nullptr);
	mappings = gLevel->GetObjectMappings(mapPath);
	delete mapPath;
	
	if (mappings->fail != nullptr)
	{
		Error(fail = mappings->fail);
		return;
	}
	
	//Read properties from the specifications
	char *plrSpecPath = AllocPath(gBasePath, specPath, ".psp");
	BACKEND_FILE *playerSpec = OpenFile(plrSpecPath, "rb");
	delete plrSpecPath;
	
	if (playerSpec == nullptr)
	{
		Error(fail = GetFileError());
		return;
	}
	
	xRadius = ReadFile_Byte(playerSpec);
	yRadius = ReadFile_Byte(playerSpec);
	
	defaultXRadius = xRadius;
	defaultYRadius = yRadius;
	rollXRadius = ReadFile_Byte(playerSpec);
	rollYRadius = ReadFile_Byte(playerSpec);
	
	characterType = (CHARACTERTYPE)ReadFile_BE16(playerSpec);
	
	READ_SPEEDDEFINITION(normalSD);
	READ_SPEEDDEFINITION(speedShoesSD);
	READ_SPEEDDEFINITION(superSD);
	READ_SPEEDDEFINITION(superSpeedShoesSD);
	READ_SPEEDDEFINITION(underwaterNormalSD);
	READ_SPEEDDEFINITION(underwaterSpeedShoesSD);
	READ_SPEEDDEFINITION(underwaterSuperSD);
	READ_SPEEDDEFINITION(underwaterSuperSpeedShoesSD);
	
	CloseFile(playerSpec);
	
	//Initialize speed
	if (!super)
	{
		if (!item.hasSpeedShoes)
			SetSpeedFromDefinition(status.underwater ? underwaterNormalSD : normalSD);
		else
			SetSpeedFromDefinition(status.underwater ? underwaterSpeedShoesSD : speedShoesSD);
	}
	else
	{
		if (!item.hasSpeedShoes)
			SetSpeedFromDefinition(status.underwater ? underwaterSuperSD : superSD);
		else
			SetSpeedFromDefinition(status.underwater ? underwaterSuperSpeedShoesSD : superSpeedShoesSD);
	}
	
	//Set render properties
	priority = 2;
	widthPixels = 24;
	heightPixels = 24;
	
	//Render flags
	renderFlags.alignPlane = true;
	
	//Collision
	topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
	lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
	
	//Flipping stuff
	flipsRemaining = 0;
	flipSpeed = 4;
	
	//Set our following person
	follow = myFollow;
	
	//Set the controller to use
	controller = myController;
	
	//Initialize our record arrays
	ResetRecords(x.pos - 0x20, y.pos - 0x04);
	
	//Load our objects
	spindashDust = new OBJECT(&gLevel->coreObjectList, ObjSpindashDust);
	spindashDust->parent = this;
	
	skidDust = new OBJECT(&gLevel->coreObjectList, ObjSkidDust);
	skidDust->parent = this;
	
	shieldObject = new OBJECT(&gLevel->coreObjectList, ObjShield);
	shieldObject->parent = this;
	
	//Attach to linked list (if applicable)
	if (linkedList != nullptr)
	{
		list = linkedList;
		
		//If linked list is unset, set us as the first 
		if (*linkedList == nullptr)
		{
			*linkedList = this;
			return;
		}
		
		//Attach us to the linked list
		for (PLAYER *player = *linkedList;; player = player->next)
		{
			if (player->next == nullptr)
			{
				player->next = this;
				break;
			}
		}
	}
	
	LOG(("Success!\n"));
}

PLAYER::~PLAYER()
{
	//Detach from linked list
	if (list != nullptr)
	{
		for (PLAYER **player = list; *player != nullptr; player = &(*player)->next)
		{
			if (*player == this)
			{
				*player = next;
				break;
			}
		}
	}
}

void PLAYER::SetSpeedFromDefinition(SPEEDDEFINITION definition)
{
	top = definition.top;
	acceleration = definition.acceleration;
	deceleration = definition.deceleration;
	rollDeceleration = definition.rollDeceleration;
	jumpForce = definition.jumpForce;
	jumpRelease = definition.jumpRelease;
}

//Generic collision functions
uint8_t PLAYER::AngleIn(uint8_t angleSide, int16_t *distance, int16_t *distance2)
{
	uint8_t outAngle = secondaryAngle;

	if (*distance2 > *distance)
	{
		outAngle = primaryAngle;

		int16_t temp = *distance; //Keep a copy because we're swapping the distances
		*distance = *distance2;
		*distance2 = temp;
	}

	//If the angle is a multi-side angled block, use our given angle side
	if (outAngle & 1)
		outAngle = angleSide;
	return outAngle;
}

void PLAYER::CheckFloor(COLLISIONLAYER layer, int16_t *distance, int16_t *distance2, uint8_t *outAngle)
{
	int16_t retDistance = FindFloor(x.pos + xRadius, y.pos + yRadius, layer, false, &primaryAngle);
	int16_t retDistance2 = FindFloor(x.pos - xRadius, y.pos + yRadius, layer, false, &secondaryAngle);

	uint8_t retAngle = AngleIn(0x00, &retDistance, &retDistance2);

	if (distance != nullptr)
		*distance = retDistance;

	if (distance2 != nullptr)
		*distance2 = retDistance2;

	if (outAngle != nullptr)
		*outAngle = retAngle;
}

void PLAYER::CheckCeiling(COLLISIONLAYER layer, int16_t *distance, int16_t *distance2, uint8_t *outAngle)
{
	int16_t retDistance = FindFloor(x.pos + xRadius, y.pos - yRadius, layer, true, &primaryAngle);
	int16_t retDistance2 = FindFloor(x.pos - xRadius, y.pos - yRadius, layer, true, &secondaryAngle);

	uint8_t retAngle = AngleIn(0x80, &retDistance, &retDistance2);

	if (distance != nullptr)
		*distance = retDistance;

	if (distance2 != nullptr)
		*distance2 = retDistance2;
	
	if (outAngle != nullptr)
		*outAngle = retAngle;
}

//Check floor distance
int16_t PLAYER::ChkFloorEdge(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle)
{
	//Clear primary angle
	primaryAngle = 0;
	
	//Get floor distance and angle
	int16_t distance = FindFloor(xPos, yPos + (status.reverseGravity ? -yRadius : yRadius), layer, status.reverseGravity, &primaryAngle);
	if (outAngle != nullptr)
		*outAngle = (primaryAngle & 1) ? 0 : primaryAngle;
	return distance;
}

//Get distance functions
uint8_t PLAYER::AngleSide(uint8_t angleSide)
{
	return (primaryAngle & 0x01) ? angleSide : primaryAngle;
}

int16_t PLAYER::CheckFloorDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindFloor(xPos, yPos + 10, layer, false, &primaryAngle);
	if (outAngle != nullptr)
		*outAngle = AngleSide(0x00);
	return distance;
}

int16_t PLAYER::CheckCeilingDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindFloor(xPos, yPos - 10, layer, true, &primaryAngle);
	if (outAngle != nullptr)
		*outAngle = AngleSide(0x80);
	return distance;
}

int16_t PLAYER::CheckLeftWallDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindWall(xPos - 10, yPos, layer, true, &primaryAngle);
	if (outAngle != nullptr)
		*outAngle = AngleSide(0x40);
	return distance;
}

int16_t PLAYER::CheckRightWallDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindWall(xPos + 10, yPos, layer, false, &primaryAngle);
	if (outAngle != nullptr)
		*outAngle = AngleSide(0xC0);
	return distance;
}

//Left and right ceiling distance functions
int16_t PLAYER::CheckLeftCeilingDist(COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindWall(x.pos - yRadius, y.pos - xRadius, layer, true, &primaryAngle);
	int16_t distance2 = FindWall(x.pos - yRadius, y.pos + xRadius, layer, true, &secondaryAngle);

	uint8_t angle = AngleIn(0x40, &distance, &distance2);

	if (outAngle)
		*outAngle = angle;

	return distance2;
}

int16_t PLAYER::CheckRightCeilingDist(COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindWall(x.pos + yRadius, y.pos - xRadius, layer, false, &primaryAngle);
	int16_t distance2 = FindWall(x.pos + yRadius, y.pos + xRadius, layer, false, &secondaryAngle);

	uint8_t angle = AngleIn(0xC0, &distance, &distance2);

	if (outAngle)
		*outAngle = angle;

	return distance2;
}

//Calculate room on top of us
int16_t PLAYER::CalcRoomOverHead(uint8_t upAngle)
{
	primaryAngle = upAngle;
	secondaryAngle = upAngle;

	int16_t distance = 0;
	switch ((upAngle + 0x20) & 0xC0)
	{
		case 0:
			CheckFloor(topSolidLayer, nullptr, &distance, nullptr);
			break;
		case 0x40:
			distance = CheckLeftCeilingDist(lrbSolidLayer, nullptr);
			break;
		case 0x80:
			CheckCeiling(lrbSolidLayer, nullptr, &distance, nullptr);
			break;
		case 0xC0:
			distance = CheckRightCeilingDist(lrbSolidLayer, nullptr);
			break;
	}

	return distance;
}

//Calculate room in front of us
int16_t PLAYER::CalcRoomInFront(uint8_t moveAngle)
{
	int16_t xPos = (xPosLong + (xVel << 8)) >> 16;
	int16_t yPos = (yPosLong + (yVel * (status.reverseGravity ? -0x100 : 0x100))) >> 16;

	primaryAngle = moveAngle;
	secondaryAngle = moveAngle;
	uint8_t offAngle = moveAngle;

	if (offAngle + 0x20 >= 0x80)
	{
		if (offAngle >= 0x80)
			--offAngle;

		offAngle += 0x20;
	}
	else
	{
		if (offAngle >= 0x80)
			++offAngle;

		offAngle += 0x1F;
	}

	offAngle &= 0xC0;

	if (offAngle == 0)
	{
		return CheckFloorDist(xPos, yPos, lrbSolidLayer, nullptr);
	}
	else if (offAngle == 0x80)
	{
		return CheckCeilingDist(xPos, yPos, lrbSolidLayer, nullptr);
	}
	else
	{
		//If at a low angle, offset the position down 8 pixels
		if ((angle & 0x38) == 0)
			yPos += 8;
		
		if (offAngle == 0x40)
			return CheckLeftWallDist(xPos, yPos, lrbSolidLayer, nullptr);
		else
			return CheckRightWallDist(xPos, yPos, lrbSolidLayer, nullptr);
	}
}

//Ground collision function
int16_t PLAYER::Angle(int16_t distance, int16_t distance2)
{
	//Get which distance is closer and use that for our calculation purposes
	int16_t outDistance = distance2;
	uint8_t thisAngle = secondaryAngle;
	
	if (distance2 > distance)
	{
		thisAngle = distance2 > distance ? primaryAngle : secondaryAngle;
		outDistance = distance;
	}

	#ifdef SONIC1_SLOPE_ANGLE
		//If the angle's least significant bit is set (a tile that has different angles for each side), use our cardinal floor angle
		if (thisAngle & 1)
			thisAngle = (angle + 0x20) & 0xC0;
	#else
		//Get our angle difference
		uint8_t angleDiff = thisAngle - angle;
		if (angleDiff >= 0x80)
			angleDiff = -angleDiff;
		
		//If the angle's least significant bit is set (a tile that has different angles for each side), or the angle difference is greater than 45 degrees, use our cardinal floor angle
		if (thisAngle & 1 || angleDiff >= 0x20)
			thisAngle = (angle + 0x20) & 0xC0;
	#endif

	angle = thisAngle;
	return outDistance;
}

void PLAYER::AnglePos()
{
	//Invert angle if gravity is reversed
	bool reverseGravity = status.reverseGravity;
	if (reverseGravity)
		angle = -(angle + 0x40) - 0x40;
	
	if (status.shouldNotFall)
	{
		//Default to just standing on flat ground if we're standing on an object or something
		primaryAngle = 0;
		secondaryAngle = 0;
	}
	else
	{
		//Set primary and secondary angle to 3
		primaryAngle = 3;
		secondaryAngle = 3;
		
		//Get the angle to use for determining our ground orientation (floor, wall, or ceiling)
		uint8_t offAngle = angle;
		if (((angle + 0x20) & 0xFF) >= 0x80)
		{
			if (angle >= 0x80)
				--offAngle;
			offAngle += 0x20;
		}
		else
		{
			if (angle >= 0x80)
				++offAngle;
			offAngle += 0x1F;
		}
		
		//Handle our individual surface collisions
		switch (offAngle & 0xC0)
		{
			case 0x00: //Floor
			{
				int16_t distance = FindFloor(x.pos + xRadius, y.pos + yRadius, topSolidLayer, false, &primaryAngle);
				int16_t distance2 = FindFloor(x.pos - xRadius, y.pos + yRadius, topSolidLayer, false, &secondaryAngle);
				int16_t nearestDifference = Angle(distance, distance2);
				
				if (nearestDifference < 0)
				{
					//I'm not sure why this checks for a specific range, if the distance is negative, it will be colliding with a floor for sure
					if (nearestDifference >= -14)
						y.pos += nearestDifference;
				}
				else if (nearestDifference > 0)
				{
					//Get how far we can clip down to the floor
					uint8_t clipLength = abs(xVel >> 8) + 4;
					if (clipLength >= 14)
						clipLength = 14;

					if (nearestDifference > clipLength && !status.stickToConvex)
					{
						//If we're running off of a ledge, enter air state
						status.inAir = true;
						status.pushing = false;
						prevAnim = PLAYERANIMATION_RUN;
					}
					else
					{
						//Move down to floor surface
						y.pos += nearestDifference;
					}
				}
				break;
			}
			
			case 0x40: //Wall to the left of us
			{
				int16_t distance = FindWall(x.pos - yRadius, y.pos - xRadius, topSolidLayer, true, &primaryAngle);
				int16_t distance2 = FindWall(x.pos - yRadius, y.pos + xRadius, topSolidLayer, true, &secondaryAngle);
				int16_t nearestDifference = Angle(distance, distance2);
				
				if (nearestDifference < 0)
				{
					//I'm not sure why this checks for a specific range, if the distance is negative, it will be colliding with a floor for sure
					if (nearestDifference >= -14)
						x.pos -= nearestDifference;
				}
				else if (nearestDifference > 0)
				{
					//Get how far we can clip down to the floor
					uint8_t clipLength = abs(yVel >> 8) + 4;
					if (clipLength >= 14)
						clipLength = 14;

					if (nearestDifference > clipLength && !status.stickToConvex)
					{
						//If we're running off of a ledge, enter air state
						status.inAir = true;
						status.pushing = false;
						prevAnim = PLAYERANIMATION_RUN;
					}
					else
					{
						//Move down to floor surface
						x.pos -= nearestDifference;
					}
				}
				break;
			}
			
			case 0x80: //Ceiling
			{
				int16_t distance = FindFloor(x.pos + xRadius, y.pos - yRadius, topSolidLayer, true, &primaryAngle);
				int16_t distance2 = FindFloor(x.pos - xRadius, y.pos - yRadius, topSolidLayer, true, &secondaryAngle);
				int16_t nearestDifference = Angle(distance, distance2);
				
				if (nearestDifference < 0)
				{
					//I'm not sure why this checks for a specific range, if the distance is negative, it will be colliding with a floor for sure
					if (nearestDifference >= -14)
						y.pos -= nearestDifference;
				}
				else if (nearestDifference > 0)
				{
					//Get how far we can clip down to the floor
					uint8_t clipLength = abs(xVel >> 8) + 4;
					if (clipLength >= 14)
						clipLength = 14;

					if (nearestDifference > clipLength && !status.stickToConvex)
					{
						//If we're running off of a ledge, enter air state
						status.inAir = true;
						status.pushing = false;
						prevAnim = PLAYERANIMATION_RUN;
					}
					else
					{
						//Move down to floor surface
						y.pos -= nearestDifference;
					}
				}
				break;
			}
			
			case 0xC0: //Wall to the right of us
			{
				int16_t distance = FindWall(x.pos + yRadius, y.pos - xRadius, topSolidLayer, false, &primaryAngle);
				int16_t distance2 = FindWall(x.pos + yRadius, y.pos + xRadius, topSolidLayer, false, &secondaryAngle);
				int16_t nearestDifference = Angle(distance, distance2);
				
				if (nearestDifference < 0)
				{
					//I'm not sure why this checks for a specific range, if the distance is negative, it will be colliding with a floor for sure
					if (nearestDifference >= -14)
						x.pos += nearestDifference;
				}
				else if (nearestDifference > 0)
				{
					//Get how far we can clip down to the floor
					uint8_t clipLength = abs(yVel >> 8) + 4;
					if (clipLength >= 14)
						clipLength = 14;

					if (nearestDifference > clipLength && !status.stickToConvex)
					{
						//If we're running off of a ledge, enter air state
						status.inAir = true;
						status.pushing = false;
						prevAnim = PLAYERANIMATION_RUN;
					}
					else
					{
						//Move down to floor surface
						x.pos += nearestDifference;
					}
				}
				break;
			}
		}
	}
	
	//Revert angle if gravity is reversed
	if (reverseGravity)
		angle = -(angle + 0x40) - 0x40;
}

void PLAYER::CheckWallsOnGround()
{
	if (objectControl.disableWallCollision)
		return;
	
#ifndef SONIC123_WALL_COLLISION
	if (((angle & 0x3F) == 0 || ((angle + 0x40) & 0xFF) < 0x80) && inertia != 0)
#else
	if (((angle + 0x40) & 0xFF) < 0x80 && inertia != 0)
#endif
	{
		uint8_t faceAngle = angle + (inertia < 0 ? 0x40 : -0x40);
		int16_t distance = CalcRoomInFront(faceAngle);

		if (distance < 0)
		{
			distance *= 0x100;

			switch ((faceAngle + 0x20) & 0xC0)
			{
				case 0x00:
				{
					yVel += distance;
					break;
				}
				case 0x40:
				{
					xVel -= distance;
					inertia = 0;
				#ifndef SONIC12_PUSH_CHECK
					if (status.xFlip)
				#endif
						status.pushing = true;
					break;
				}
				case 0x80:
				{
					yVel -= distance;
					break;
				}
				case 0xC0:
				{
					xVel += distance;
					inertia = 0;
				#ifndef SONIC12_PUSH_CHECK
					if (!status.xFlip)
				#endif
						status.pushing = true;
					break;
				}
			}
		}
	}
}

//Air collision functions
void PLAYER::DoLevelCollision()
{
	//Get the primary angle we're moving in
	uint8_t moveAngle = GetAtan(xVel, yVel);
	
	switch ((moveAngle - 0x20) & 0xC0)
	{
		case 0x00: //Moving downwards
		{
			int16_t distance, distance2;
			
			//Check for wall collisions
			distance = CheckLeftWallDist(x.pos, y.pos, lrbSolidLayer, nullptr);
			if (distance < 0)
			{
				//Clip out and stop our velocity
				x.pos -= distance;
				xVel = 0;
			}
			
			distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, nullptr);
			if (distance < 0)
			{
				//Clip out and stop our velocity
				x.pos += distance;
				xVel = 0;
			}
			
			//Check for collision with the floor
			uint8_t floorAngle;
			
			if (!status.reverseGravity)
			{
				CheckFloor(topSolidLayer, &distance, &distance2, &floorAngle);
			}
			else
			{
				CheckCeiling(lrbSolidLayer, &distance, &distance2, &floorAngle);
				floorAngle = -(floorAngle + 0x40) - 0x40;
			}
			
			//Are we touching the floor (and within clip length)
			const int8_t clipLength = -((yVel >> 8) + 8);
			if (distance2 < 0 && (distance2 >= clipLength || distance >= clipLength))
			{
				//Inherit the floor's angle
				angle = floorAngle;
				
				//Clip out of floor
				if (!status.reverseGravity)
					y.pos += distance2;
				else
					y.pos -= distance2;
				
				//Get our inertia from our global speeds
				if ((angle + 0x20) & 0x40)
				{
					//If floor is greater than 45 degrees, use our full vertical velocity (capped at 0xFC0)
					xVel = 0;
					if (yVel > 0xFC0)
						yVel = 0xFC0;

					inertia = angle >= 0x80 ? -yVel : yVel;
				}
				else if ((angle + 0x10) & 0x20)
				{
					//If floor is greater than 22.5 degrees, use our halved vertical velocity
					yVel /= 2;
					inertia = angle >= 0x80 ? -yVel : yVel;
				}
				else
				{
					//If floor is less than 22.5 degrees, use our horizontal velocity
					yVel = 0;
					inertia = xVel;
				}
				
				//Land on floor
				LandOnFloor();
			}
			break;
		}
		
		case 0x40: //Moving to the left
		{
			//Collide with walls to the left of us
			int16_t distance = CheckLeftWallDist(x.pos, y.pos, lrbSolidLayer, nullptr);

			if (distance < 0)
			{
				//Clip out of the wall
				x.pos -= distance;
				
				//Stop our velocity
				xVel = 0;
				inertia = yVel; //This affects walk / run animations to make them usually appear slower
			}
			else
			{
				//Collide with ceilings
				if (!status.reverseGravity)
					CheckCeiling(lrbSolidLayer, nullptr, &distance, nullptr);
				else
					CheckFloor(topSolidLayer, nullptr, &distance, nullptr);
				
				if (distance < 0)
				{
				#ifndef SONIC12_SANE_AIRCOLLISION
					if (distance > -14)
					{
				#endif
						//Clip out of ceiling
						if (!status.reverseGravity)
							y.pos -= distance;
						else
							y.pos += distance;
						
						//Stop our vertical velocity
						if (yVel < 0)
							yVel = 0;
				#ifndef SONIC12_SANE_AIRCOLLISION
					}
					else
					{
						//Collide with walls to the right?
						int16_t distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, nullptr);
						
						if (distance < 0)
						{
							x.pos += distance;
							xVel = 0;
						}
					}
				#endif
				}
				else if (status.windTunnel || yVel >= 0)
				{
					//Collide with the floor
					uint8_t floorAngle;
					if (!status.reverseGravity)
					{
						CheckFloor(topSolidLayer, nullptr, &distance, &floorAngle);
					}
					else
					{
						CheckCeiling(lrbSolidLayer, nullptr, &distance, &floorAngle);
						floorAngle = -(floorAngle + 0x40) - 0x40;
					}
					
					if (distance < 0)
					{
						//Clip out of floor
						if (!status.reverseGravity)
							y.pos += distance;
						else
							y.pos -= distance;
						
						//Inherit floor's angle
						angle = floorAngle;
						
						//Inherit horizontal velocity
						yVel = 0;
						inertia = xVel;
						
						//Land on floor
						LandOnFloor();
					}
				}
			}
			break;
		}
		
		case 0x80: //Moving upwards
		{
			//Check for wall collisions
			int16_t distance;
			distance = CheckLeftWallDist(x.pos, y.pos, lrbSolidLayer, nullptr);
			if (distance < 0)
			{
				//Clip out of the wall and stop our velocity
				x.pos -= distance;
				xVel = 0;
			}
			
			distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, nullptr);
			if (distance < 0)
			{
				//Clip out of the wall and stop our velocity
				x.pos += distance;
				xVel = 0;
			}
			
			//Check for collision with ceilings
			uint8_t ceilingAngle;
			if (!status.reverseGravity)
			{
				CheckCeiling(lrbSolidLayer, nullptr, &distance, &ceilingAngle);
			}
			else
			{
				CheckFloor(topSolidLayer, nullptr, &distance, &ceilingAngle);
				ceilingAngle = -(ceilingAngle + 0x40) - 0x40;
			}
			
			if (distance < 0)
			{
				//Clip out of ceiling
				if (!status.reverseGravity)
					y.pos -= distance;
				else
					y.pos += distance;
				
				//If ceiling is less than 135 degrees, land on it, otherwise be stopped by it
				if (((ceilingAngle + 0x20) & 0x40) == 0)
				{
					//Stop our vertical velocity
					yVel = 0;
				}
				else
				{
					//Land on ceiling
					angle = ceilingAngle;
					LandOnFloor();
					inertia = ceilingAngle >= 0x80 ? -yVel : yVel;
				}
			}
			break;
		}
		
		case 0xC0: //Moving to the right
		{
			//Collide with walls
			int16_t distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, nullptr);

			if (distance < 0)
			{
				//Clip out of the wall
				x.pos += distance;
				
				//Stop our velocity
				xVel = 0;
				inertia = yVel; //This affects walk / run animations to make them usually appear slower
			}
			else
			{
				//Collide with ceilings
				if (!status.reverseGravity)
					CheckCeiling(lrbSolidLayer, nullptr, &distance, nullptr);
				else
					CheckFloor(topSolidLayer, nullptr, &distance, nullptr);
				
				if (distance < 0)
				{
					//Clip out of ceiling (NOTE: There's no "> -14" check here, unlike moving left)
					if (!status.reverseGravity)
						y.pos -= distance;
					else
						y.pos += distance;
					
					//Stop our vertical velocity
					if (yVel < 0)
						yVel = 0;
				}
				else if (status.windTunnel || yVel >= 0)
				{
					//Collide with the floor
					uint8_t floorAngle;
					if (!status.reverseGravity)
					{
						CheckFloor(topSolidLayer, nullptr, &distance, &floorAngle);
					}
					else
					{
						CheckCeiling(lrbSolidLayer, nullptr, &distance, &floorAngle);
						floorAngle = -(floorAngle + 0x40) - 0x40;
					}
					
					if (distance < 0)
					{
						//Clip out of floor
						if (!status.reverseGravity)
							y.pos += distance;
						else
							y.pos -= distance;
						
						//Inherit floor's angle
						angle = floorAngle;
						
						//Inherit horizontal velocity
						yVel = 0;
						inertia = xVel;
						
						//Land on floor
						LandOnFloor();
					}
				}
			}
			break;
		}
	}
}

//Functions for landing on the ground
void PLAYER::LandOnFloor()
{
#ifndef SONIC2_SPINDASH_ANIM_BUG
	if (status.pinballMode || spindashing)
#else
	if (status.pinballMode)
#endif
	{
		//Do not exit ball form if in pinball mode
		LandOnFloor_SetState();
	}
	else
	{
		//Exit ball form
		anim = PLAYERANIMATION_WALK;
		LandOnFloor_ExitBall();
	}
}

void PLAYER::LandOnFloor_ExitBall()
{
	//Keep track of our previous height
	uint8_t oldYRadius = yRadius;
	xRadius = defaultXRadius;
	yRadius = defaultYRadius;
	
	if (status.inBall)
	{
		//Exit ball form
		status.inBall = false;
		anim = PLAYERANIMATION_WALK; //again
		
		//Shift up to ground level
		#ifndef SONIC12_ROLLJUMP_LAND
			uint8_t difference = yRadius - oldYRadius;
		#else
			uint8_t difference = 5;
		#endif
		
		YSHIFT_ON_FLOOR(-difference);
	}

	LandOnFloor_SetState();
}

void PLAYER::LandOnFloor_SetState()
{
	//Exit airborne state
	status.inAir = false;
	status.pushing = false;
	status.rollJumping = false;
	status.jumping = false;
	
	//Clear chain point counter
	chainPointCounter = 0;
	
	//Clear flipping
	flipAngle = 0;
	flipType = 0;
	flipsRemaining = 0;
	
	#ifndef SONIC12_NO_JUMP_ABILITY
		//Jump ability
		if (jumpAbility != 0)
		{
			//Handle jump abilities on landing
			if (characterType == CHARACTERTYPE_SONIC && super == false)
			{
				//Bubble shield bounce
				if (shield == SHIELD_BUBBLE)
				{
					//Get the force of our bounce
					int16_t bounceForce = 0x780;
					if (status.underwater)
						bounceForce = 0x400;
					
					//Bounce us up from the ground
					int16_t sin, cos;
					GetSine(angle - 0x40, &sin, &cos);
					xVel += (cos * bounceForce) >> 8;
					yVel += (sin * bounceForce) >> 8;
					
					//Put us back into the air state
					status.inAir = true;
					status.pushing = false;
					status.jumping = true;
					anim = PLAYERANIMATION_ROLL;
					status.inBall = true;
					
					//Return to the ball hitbox
					xRadius = rollXRadius;
					yRadius = rollYRadius;
					
					YSHIFT_ON_FLOOR(-(defaultYRadius - yRadius));
					
					//Play the sound and play the squish animation for the bubble
					if (shieldObject != nullptr)
						shieldObject->anim = 2;
					PlaySound(SOUNDID_USE_BUBBLE_SHIELD);
				}
			}
			
			//Enable jump ability
			jumpAbility = 0;
		}
	#endif
}

//Record our position in the records
void PLAYER::RecordPos()
{
	//Set the records at the current position
	posRecord[recordPos] = {x.pos, y.pos};
	statRecord[recordPos] = {controlHeld, controlPress, status};
	
	//Increment record position
	recordPos++;
	recordPos %= PLAYER_RECORD_LENGTH;
}

void PLAYER::ResetRecords(int16_t xPos, int16_t yPos)
{
	//Clear our records with the given position
	for (int i = 0; i < PLAYER_RECORD_LENGTH; i++)
	{
		posRecord[i].x = xPos;
		posRecord[i].y = yPos;
		memset(&statRecord[i], 0, sizeof(statRecord[0])); //Stat record is initialized as 0
	}
	
	//Reset our record pos
	recordPos = 0;
}

//Spindash code
const uint16_t spindashSpeed[9] =		{0x800, 0x880, 0x900, 0x980, 0xA00, 0xA80, 0xB00, 0xB80, 0xC00};
const uint16_t spindashSpeedSuper[9] =	{0xB00, 0xB80, 0xC00, 0xC80, 0xD00, 0xD80, 0xE00, 0xE80, 0xF00};
		
bool PLAYER::Spindash()
{
	#if (defined(SONIC1_NO_SPINDASH) || defined(SONICCD_SPINDASH))
		return true;
	#else
		if (spindashing == false)
		{
			//We must be ducking in order to spindash
			if (anim != PLAYERANIMATION_DUCK)
				return true;
			
			//Initiate spindash
			if (controlPress.a || controlPress.b || controlPress.c)
			{
				//Play animation and sound
				anim = PLAYERANIMATION_SPINDASH;
				PlaySound(SOUNDID_SPINDASH_REV);
				
				//Set spindash variables
				spindashing = true;
				spindashCounter = 0;
				
				//Make our spindash dust visible
				if (spindashDust != nullptr)
					spindashDust->anim = 1;
			}
			else
			{
				return true;
			}
		}
		//Release spindash
		else if (!controlHeld.down)
		{
			//Begin rolling
			xRadius = rollXRadius;
			yRadius = rollYRadius;
			anim = PLAYERANIMATION_ROLL;
			
			//Offset our position to line up with the ground
			YSHIFT_ON_FLOOR(5);
			
			//Release spindash
			spindashing = false;
			
			//Set our speed
			if (super)
				inertia = spindashSpeedSuper[spindashCounter >> 8];
			else
				inertia = spindashSpeed[spindashCounter >> 8];
			
			//Lock the camera behind us
			scrollDelay = (0x2000 - (inertia - 0x800) * 2) % (PLAYER_RECORD_LENGTH << 8);
			
			//Revert if facing left
			if (status.xFlip)
				inertia = -inertia;
			
			//Actually go into the roll routine
			status.inBall = true;
			if (spindashDust != nullptr)
				spindashDust->anim = 0;
			PlaySound(SOUNDID_SPINDASH_RELEASE);
			
			#ifdef FIX_SPINDASH_JUMP
				//Convert our inertia into global speeds
				int16_t sin, cos;
				GetSine(angle, &sin, &cos);
				xVel = (cos * inertia) >> 8;
				yVel = (sin * inertia) >> 8;
			#endif
		}
		//Charging spindash
		else
		{
			//Reduce our spindash counter
			if (spindashCounter != 0)
			{
				uint16_t nextCounter = (spindashCounter - spindashCounter / 0x20);
				
				//The original makes sure the spindash counter is 0 if it underflows (which seems to be impossible, to my knowledge)
				if (nextCounter <= spindashCounter)
					spindashCounter = nextCounter;
				else
					spindashCounter = 0;
			}
			
			#ifdef FIX_SPINDASH_ANIM
				//Ensure we're doing the spindash animation
				anim = PLAYERANIMATION_SPINDASH;
			#endif
			
			//Rev our spindash
			if (controlPress.a || controlPress.b || controlPress.c)
			{
				//Restart the spindash animation and play the rev sound
				anim = PLAYERANIMATION_SPINDASH;
				prevAnim = PLAYERANIMATION_WALK;
				PlaySound(SOUNDID_SPINDASH_REV);
				
				//Increase our spindash counter
				spindashCounter += 0x200;
				if (spindashCounter >= 0x800)
					spindashCounter = 0x800;
			}
		}
		
		//Collide with the level (S3K has code to crush you against the foreground layer from the background here)
		LevelBound();
		AnglePos();
		return false;
	#endif
}

//CD Peelout and Spindash
bool PLAYER::CDPeeloutSpindash(bool *moveRet) //return false = branch to updatespeed, true = branch to resetscr
{
#ifdef SONICCD_PEELOUT
	if (!(cdChargeDelay & 0x80))
	{
		if (!((cdChargeDelay & 0x40) || controlHeld.down))
		{
			//Handle how up input gets used
			if (cdChargeDelay &= 0x0F)
			{
				if (controlPress.up)
				{
					cdChargeDelay |= 0x80;
					return false;
				}
			}
			else
			{
				if (controlPress.up)
				{
					cdChargeDelay = 0x01;
					return false;
				}
			}
			
			//Check if we should peelout or already are
			if (controlHeld.up)
			{
				anim = PLAYERANIMATION_LOOKUP;
				
				if (cdSPTimer != 0)
				{
					//Charge peelout
					anim = PLAYERANIMATION_WALK;
					
					int16_t peeloutAccel = 0x64;
					int16_t peeloutCap = top << 1;
					
					if (item.hasSpeedShoes)
						peeloutCap -= (top >> 1);
					
					//Reverse if facing left
					if (status.xFlip)
					{
						peeloutAccel = -peeloutAccel;
						peeloutCap = -peeloutCap;
					}
					
					//Increase peelout speed
					inertia += peeloutAccel;
					
					//Cap our inertia
					if (!status.xFlip)
					{
						if (inertia > peeloutCap)
							inertia = peeloutCap;
					}
					else
					{
						if (inertia < peeloutCap)
							inertia = peeloutCap;
					}
					
					//NOTE: this cuts the movement sub-routine short
					*moveRet = true;
					return true;
				}
				else
				{
					//Check if we should start a peelout
					if (controlPress.a || controlPress.b || controlPress.c)
					{
						cdSPTimer = 1;
						PlaySound(SOUNDID_SPINDASH_REV);
					}
					
					return false;
				}
			}
			else
			{
				//Release peelout
				if (cdSPTimer != 30)
				{
					//Failed peelout
					cdSPTimer = 0;
					inertia = 0;
				}
				else
				{
					//Successful peelout
					cdSPTimer = 0;
					PlaySound(SOUNDID_SPINDASH_RELEASE);
					return true;
				}
			}
		}
	}
	else
#endif
	{
		//Handle looking up
		if (controlHeld.up)
			anim = PLAYERANIMATION_LOOKUP;
	}
	
#ifdef SONICCD_SPINDASH
	if (!(cdChargeDelay & 0x40))
	{
		//Handle how down input gets used
		if (cdChargeDelay &= 0x0F)
		{
			if (controlPress.down)
			{
				cdChargeDelay |= 0x40;
				return false;
			}
		}
		else
		{
			if (controlPress.down)
			{
				cdChargeDelay = 0x01;
				return false;
			}
		}
		
		//Check if we should spindash or already are
		if (!controlHeld.down)
			return true;
		anim = PLAYERANIMATION_DUCK;
		if (cdSPTimer != 0 || !(controlPress.a || controlPress.b || controlPress.c))
			return false;
		
		//Start spindashing
		PlaySound(SOUNDID_SPINDASH_REV);
		cdSPTimer = 1;
		inertia = 0x16;
		if (status.xFlip)
			inertia = -inertia;
		
		ChkRoll();
		return false;
	}
	else
#endif
	{
		//Handle ducking
		if (!controlHeld.down)
			return true;
		anim = PLAYERANIMATION_DUCK;
		return false;
	}
	
	return true;
}

//Jump ability functions
void PLAYER::JumpAbilities()
{
	if (jumpAbility == 0 && (controlPress.a || controlPress.b || controlPress.c))
	{
		#ifndef CONTROL_JA_DONT_CLEAR_ROLLJUMP
			//Clear the roll jump flag, so we regain horizontal control
			status.rollJumping = false;
		#endif
		
		//Perform our ability
		if (super)
		{
			if (0 /*hyper*/)
			{
				//Hyper dash
			}
			else
			{
				//Super cannot use any jump abilities, just clear jump ability
				jumpAbility = 1;
			}
		}
		else if (!item.isInvincible)
		{
			#ifndef SONIC12_NO_SHIELD_ABILITIES
				//Check and handle shield abilities
				if (shield != SHIELD_NULL)
				{
					if (shield == SHIELD_FIRE)
					{
						//Update our shield and ability flag
						if (shieldObject != nullptr)
							shieldObject->anim = 1;
						jumpAbility = 1;
						
						//Dash in our facing direction
						int16_t speed = 0x800;
						if (status.xFlip)
							speed = -speed;
						
						xVel = speed;
						inertia = speed;
						yVel = 0;
						
						//Make the camera lag behind us
						scrollDelay = 0x2000;
						ResetRecords(x.pos, y.pos);
						PlaySound(SOUNDID_USE_FIRE_SHIELD);
					}
					else if (shield == SHIELD_ELECTRIC)
					{
						//Update our shield and ability flag
						if (shieldObject != nullptr)
							shieldObject->anim = 1;
						jumpAbility = 1;
						
						//Electric shield double jump
						yVel = -0x680;
						status.jumping = false;
						PlaySound(SOUNDID_USE_ELECTRIC_SHIELD);
					}
					else if (shield == SHIELD_BUBBLE)
					{
						//Update our shield and ability flag
						if (shieldObject != nullptr)
							shieldObject->anim = 1;
						jumpAbility = 1;
						
						//Shoot down to the ground
						xVel = 0;
						inertia = 0;
						yVel = 0x800;
						PlaySound(SOUNDID_USE_BUBBLE_SHIELD);
					}
					
					return;
				}
			#endif
			#if !(defined(SONIC1_NO_SUPER) || defined(SONIC2_SUPER_AT_PEAK))
				if (SuperTransform())
					return;
			#endif
			#ifndef SONIC12_NO_INSTASHIELD
				//Update our shield and ability flag
				if (shieldObject != nullptr)
						shieldObject->anim = 1;
				jumpAbility = 1;
				PlaySound(SOUNDID_USE_INSTA_SHIELD);
			#endif
		}
	}
}

//Jumping functions
void PLAYER::JumpHeight()
{
	if (status.jumping)
	{
		//Slow us down if ABC is released when jumping
		#if (!defined(SONIC2_SUPER_AT_PEAK) || !defined(SONIC12_NO_INSTASHIELD) || !defined(SONIC12_NO_SHIELD_ABILITIES))
			if (-jumpRelease <= yVel)
				JumpAbilities();
			else if (!controlHeld.a && !controlHeld.b && !controlHeld.c)
				yVel = -jumpRelease;
		#else
			if (-jumpRelease > yVel && !controlHeld.a && !controlHeld.b && !controlHeld.c)
				yVel = -jumpRelease;
		#endif
		
		#if (!defined(SONIC1_NO_SUPER) && defined(SONIC2_SUPER_AT_PEAK))
			if (!(yVel & 0xFF00))
				SuperTransform();
		#endif
	}
	else
	{
		//Cap our upwards velocity
		if (!status.pinballMode && yVel < -0xFC0)
			yVel = -0xFC0;
	}
}

void PLAYER::ChgJumpDir()
{
	//Move left and right
#ifndef SONICCD_ROLLJUMP
	if (!status.rollJumping)
#endif
	{
		int16_t newVelocity = xVel;
		int16_t jumpAcceleration = acceleration << 1;
		
		//Move left if left is held
		if (controlHeld.left)
		{
			//Accelerate left
			status.xFlip = true;
			newVelocity -= jumpAcceleration;
			
			//Don't accelerate past the top speed
			#ifndef SONIC12_AIR_CAP
				if (newVelocity <= -top)
				{
					newVelocity += jumpAcceleration;
					if (newVelocity >= -top)
						newVelocity = -top;
				}
			#else
				if (newVelocity <= -top)
					newVelocity = -top;
			#endif
		}
		
		//Move right if right is held
		if (controlHeld.right)
		{
			//Accelerate right
			status.xFlip = false;
			newVelocity += jumpAcceleration;
			
			//Don't accelerate past the top speed
			#ifndef SONIC12_AIR_CAP
				if (newVelocity >= top)
				{
					newVelocity -= jumpAcceleration;
					if (newVelocity <= top)
						newVelocity = top;
				}
			#else
				if (newVelocity >= top)
					newVelocity = top;
			#endif
		}
		
		//Copy our new velocity to our velocity
		xVel = newVelocity;
	}
	
	//Air drag
	if (yVel >= -0x400 && yVel < 0)
	{
		int16_t drag = xVel >> 5;
		
		if (drag > 0)
		{
			xVel -= drag;
			if (xVel < 0)
				xVel = 0;
		}
		else if (drag < 0)
		{
			xVel -= drag;
			if (xVel >= 0)
				xVel = 0;
		}
	}
}

void PLAYER::JumpAngle()
{
	//Bring our angle down back upwards
	if (angle != 0)
	{
		if (angle >= 0x80)
		{
			angle += 2;
			if (angle < 0x80)
				angle = 0;
		}
		else
		{
			angle -= 2;
			if (angle >= 0x80)
				angle = 0;
		}
	}
	
	//Handle our flipping
	uint8_t nextFlipAngle = flipAngle;
	
	if (nextFlipAngle != 0)
	{
		if (inertia >= 0 || flipType >= 0x80)
		{
			nextFlipAngle += flipSpeed;
			
			typeof(flipsRemaining) lastFlipsRemaining = flipsRemaining;
			if (nextFlipAngle < flipAngle && --flipsRemaining > lastFlipsRemaining) //If flipped
			{
				flipsRemaining = 0;
				nextFlipAngle = 0;
			}
		}
		else
		{
			nextFlipAngle -= flipSpeed;
			
			typeof(flipsRemaining) lastFlipsRemaining = flipsRemaining;
			if (nextFlipAngle > flipAngle && --flipsRemaining > lastFlipsRemaining) //If flipped
			{
				flipsRemaining = 0;
				nextFlipAngle = 0;
			}
		}
		
		flipAngle = nextFlipAngle;
	}
}

bool PLAYER::Jump()
{
	#if defined(SONICCD_PEELOUT) || defined(SONICCD_SPINDASH)
		//Get if we should filter up/down input (peelout / spindash)
		bool upOrDownFilter = false;
		
		#ifdef SONICCD_PEELOUT
			if (controlHeld.up)
				upOrDownFilter = true;
		#endif
		#ifdef SONICCD_SPINDASH
			if (controlHeld.down)
				upOrDownFilter = true;
		#endif
		
		//Check if we can jump (CD spindash)
		if (cdSPTimer != 0 ||
			((upOrDownFilter) && !inertia))
			return true;
	#endif
	
	if (controlPress.a || controlPress.b || controlPress.c)
	{
		//Get the angle of our head
		uint8_t headAngle = angle;
		if (status.reverseGravity)
			headAngle = (~(headAngle + 0x40)) - 0x40;
		headAngle -= 0x80;
		
		//Don't jump if under a low ceiling
		if (CalcRoomOverHead(headAngle) >= 6)
		{
			//Apply the velocity
			int16_t sin, cos;
			GetSine(angle - 0x40, &sin, &cos);
			
			xVel += (cos * jumpForce) >> 8;
			yVel += (sin * jumpForce) >> 8;
			
			//Put us in the jump state
			status.inAir = true;
			status.pushing = false;
			status.jumping = true;
			status.stickToConvex = false;
			
			PlaySound(SOUNDID_JUMP);
			
			//Handle our collision and roll state
			#ifndef FIX_ROLLJUMP_COLLISION
				xRadius = defaultXRadius;
				yRadius = defaultYRadius;
			#else
				xRadius = rollXRadius;
				yRadius = rollYRadius;
			#endif
			
			#ifndef CONTROL_NO_ROLLJUMP_LOCK
			if (!status.inBall)
			#endif
			{
				//Go into ball form
				#ifndef FIX_ROLLJUMP_COLLISION
					xRadius = rollXRadius;
					yRadius = rollYRadius;
				#endif
				
				anim = PLAYERANIMATION_ROLL;
				status.inBall = true;
				
				//Shift us down to the ground
				YSHIFT_ON_FLOOR(-(yRadius - defaultYRadius));
			}
			else
			{
				//Set our roll jump flag (also we use the regular non-roll collision size for some reason)
				status.rollJumping = true;
			}
			return false;
		}
	}
	
	return true;
}

//Slope gravity related functions
void PLAYER::SlopeResist()
{
	if (((angle + 0x60) & 0xFF) < 0xC0)
	{
		//Get our slope gravity
		int16_t sin;
		GetSine(angle, &sin, nullptr);
		sin = (sin * 0x20) >> 8;
		
		#ifndef SONIC12_SLOPE_RESIST
			//Apply our slope gravity (if our inertia is non-zero, always apply, if it is 0, apply if the force is at least 0xD units per frame)
			if (inertia != 0)
			{
				if (inertia < 0)
					inertia += sin;
				else if (sin != 0)
					inertia += sin;
			}
			else
			{
				
				if (abs(sin) >= 0xD)
					inertia += sin;
			}
		#else
			if (inertia > 0)
			{
				if (sin != 0)
					inertia += sin;
			}
			else if (inertia < 0)
			{
				inertia += sin;
			}
		#endif
	}
}

void PLAYER::RollRepel()
{
	if (((angle + 0x60) & 0xFF) < 0xC0)
	{
		//Get our slope gravity
		int16_t sin;
		GetSine(angle, &sin, nullptr);
		sin = (sin * 0x50) >> 8;
		
		//Apply our slope gravity (divide by 4 if opposite to our inertia sign)
		if (inertia >= 0)
		{
			if (sin < 0)
				sin >>= 2;
			inertia += sin;
		}
		else
		{
			if (sin >= 0)
				sin >>= 2;
			inertia += sin;
		}
	}
}

void PLAYER::SlopeRepel()
{
	if (!status.stickToConvex)
	{
		if (moveLock == 0)
		{
			#ifndef SONIC12_SLOPE_REPEL
				//Are we on a steep enough slope and going too slow?
				if (((angle + 0x18) & 0xFF) >= 0x30 && abs(inertia) < 0x280)
				{
					//Lock our controls for 30 frames (half a second)
					moveLock = 30;
					
					//Slide down the slope, or fall off if very steep
					if (((angle + 0x30) & 0xFF) >= 0x60)
						status.inAir = true;
					else if (((angle + 0x30) & 0xFF) >= 0x30)
						inertia += 0x80;
					else
						inertia -= 0x80;
				}
			#else
				//Are we on a steep enough slope and going too slow?
				if (((angle + 0x20) & 0xC0) && abs(inertia) < 0x280)
				{
					inertia = 0;
					status.inAir = true;
					moveLock = 30;
				}
			#endif
		}
		else
		{
			//Decrement moveLock every frame it's non-zero
			moveLock--;
		}
	}
}

//Movement functions
void PLAYER::MoveLeft()
{
	int16_t newInertia = inertia;
	
	if (newInertia <= 0)
	{
		//Flip if not already turned around
		if (!status.xFlip)
		{
			status.xFlip = true;
			status.pushing = false;
			prevAnim = PLAYERANIMATION_RUN;
		}
		
		//Accelerate
		newInertia -= acceleration;
		
		//Don't accelerate past the top speed
		#ifndef SONIC1_GROUND_CAP
			if (newInertia <= -top)
			{
				newInertia += acceleration;
				if (newInertia >= -top)
					newInertia = -top;
			}
		#else
			if (newInertia <= -top)
				newInertia = -top;
		#endif
		
		//Set inertia and do walk animation
		inertia = newInertia;
		anim = PLAYERANIMATION_WALK;
	}
	else
	{
		//Decelerate
		newInertia -= deceleration;
		if (newInertia < 0)
			newInertia = -0x80;
		inertia = newInertia;
		
		//Do skid animation if on a floor and above 0x400 units per frame
		if (((angle + 0x20) & 0xC0) == 0 && inertia >= 0x400)
		{
			PlaySound(SOUNDID_SKID);
			anim = PLAYERANIMATION_SKID;
			status.xFlip = false;
			
			if (skidDust != nullptr)
				skidDust->anim = 1;
		}
	}
}

void PLAYER::MoveRight()
{
	int16_t newInertia = inertia;
	
	if (newInertia >= 0)
	{
		//Flip if not already turned around
		if (status.xFlip)
		{
			status.xFlip = false;
			status.pushing = false;
			prevAnim = PLAYERANIMATION_RUN;
		}
		
		//Accelerate
		newInertia += acceleration;
		
		//Don't accelerate past the top speed
		#ifndef SONIC1_GROUND_CAP
			if (newInertia >= top)
			{
				newInertia -= acceleration;
				if (newInertia <= top)
					newInertia = top;
			}
		#else
			if (newInertia >= top)
				newInertia = top;
		#endif
		
		//Set inertia and do walk animation
		inertia = newInertia;
		anim = PLAYERANIMATION_WALK;
	}
	else
	{
		//Decelerate
		newInertia += deceleration;
		if (newInertia >= 0)
			newInertia = 0x80;
		inertia = newInertia;
		
		//Do skid animation if on a floor and above 0x400 units per frame
		if (((angle + 0x20) & 0xC0) == 0 && inertia <= -0x400)
		{
			PlaySound(SOUNDID_SKID);
			anim = PLAYERANIMATION_SKID;
			status.xFlip = true;
			
			if (skidDust != nullptr)
				skidDust->anim = 1;
		}
	}
}

void PLAYER::Move()
{
	if (!status.isSliding)
	{
		if (!moveLock)
		{
			//Move left and right
			if (cdSPTimer == 0)
			{
				if (controlHeld.left)
					MoveLeft();
				if (controlHeld.right)
					MoveRight();
			}
			
			if (((angle + 0x20) & 0xC0) == 0 && (inertia == 0 || cdSPTimer != 0))
			{
				if (cdSPTimer == 0)
				{
					//Do idle animation
					status.pushing = false;
					anim = PLAYERANIMATION_IDLE;
					
					//Balancing
					if (status.shouldNotFall)
					{
						OBJECT *object = (OBJECT*)interact;
						
						//Balancing on an object
						if (object != nullptr && !object->status.noBalance)
						{
							//Get our area we stand on
							int width = (object->widthPixels * 2) - 4;
							int xDiff = (object->widthPixels + x.pos) - object->x.pos;
							
							if (xDiff < 2)
							{
								if (!super)
								{
									if (status.xFlip)
									{
										if (xDiff < -4)
											anim = PLAYERANIMATION_BALANCE2;	//Balancing on the edge
										else
											anim = PLAYERANIMATION_BALANCE1;	//Far over the edge
									}
									else
									{
										if (xDiff < -4)
										{
											anim = PLAYERANIMATION_BALANCE4;	//Turn around to the "far over the edge" balancing animation
											status.xFlip = true;
										}
										else
											anim = PLAYERANIMATION_BALANCE3;	//Balancing on the edge backwards
									}
								}
								else
								{
									anim = PLAYERANIMATION_BALANCE1;	//Super sonic balance animation
									status.xFlip = true;
								}
							}
							else if (xDiff >= width)
							{
								if (!super)
								{
									if (!status.xFlip)
									{
										if (xDiff >= width + 6)
											anim = PLAYERANIMATION_BALANCE2;	//Balancing on the edge
										else
											anim = PLAYERANIMATION_BALANCE1;	//Far over the edge
									}
									else
									{
										if (xDiff >= width + 6)
										{
											anim = PLAYERANIMATION_BALANCE4;	//Turn around to the "far over the edge" balancing animation
											status.xFlip = false;
										}
										else
											anim = PLAYERANIMATION_BALANCE3;	//Balancing on the edge backwards
									}
								}
								else
								{
									anim = PLAYERANIMATION_BALANCE1;	//Super sonic balance animation
									status.xFlip = false;
								}
							}
						}
					}
					else
					{
						//If Sonic's middle bottom point is 12 pixels away from the floor, start balancing
						if (ChkFloorEdge(topSolidLayer, x.pos, y.pos, nullptr) >= 12)
						{
							if (nextTilt == 3) //If there's no floor to the left of us
							{
								if (!super)
								{
									if (!status.xFlip)
									{
										if (ChkFloorEdge(topSolidLayer, x.pos - 6, y.pos, nullptr) >= 12)
											anim = PLAYERANIMATION_BALANCE2;	//Far over the edge
										else
											anim = PLAYERANIMATION_BALANCE1;	//Balancing on the edge
									}
									else
									{
										//Facing right on ledge to the left of us...
										if (ChkFloorEdge(topSolidLayer, x.pos - 6, y.pos, nullptr) >= 12)
										{
											anim = PLAYERANIMATION_BALANCE4;	//Turn around to the "far over the edge" balancing animation
											status.xFlip = false;
										}
										else
											anim = PLAYERANIMATION_BALANCE3;	//Balancing on the edge backwards
									}
								}
								else
								{
									anim = PLAYERANIMATION_BALANCE1;	//Super sonic balance animation
									status.xFlip = false;
								}
							}
							else if (tilt == 3) //If there's no floor to the right of us
							{
								if (!super)
								{
									if (status.xFlip)
									{
										if (ChkFloorEdge(topSolidLayer, x.pos + 6, y.pos, nullptr) >= 12)
											anim = PLAYERANIMATION_BALANCE2;	//Far over the edge
										else
											anim = PLAYERANIMATION_BALANCE1;	//Balancing on the edge
									}
									else
									{
										//Facing right on ledge to the left of us...
										if (ChkFloorEdge(topSolidLayer, x.pos + 6, y.pos, nullptr) >= 12)
										{
											anim = PLAYERANIMATION_BALANCE4;	//Turn around to the "far over the edge" balancing animation
											status.xFlip = true;
										}
										else
											anim = PLAYERANIMATION_BALANCE3;	//Balancing on the edge backwards
									}
								}
								else
								{
									anim = PLAYERANIMATION_BALANCE1;	//Super sonic balance animation
									status.xFlip = true;
								}
							}
						}
					}
				}
				
				if (anim == PLAYERANIMATION_IDLE || cdSPTimer != 0)
				{
					#if (defined(SONICCD_PEELOUT) || defined(SONICCD_SPINDASH))
						//Handle charge delay
						if (cdChargeDelay & 0x0F)
							cdChargeDelay = (cdChargeDelay + 0x01) & 0xCF;
						
						bool moveRet = false;
						bool checkChargeClear = CDPeeloutSpindash(&moveRet);
						if (moveRet)
							return;
						
						if (checkChargeClear && !(cdChargeDelay & 0x0F))
							cdChargeDelay = 0;
					#else
						//Look up and down
						if (controlHeld.up)
							anim = PLAYERANIMATION_LOOKUP;
						else if (controlHeld.down)
							anim = PLAYERANIMATION_DUCK; //This is done in Roll too
					#endif
				}
				else
				{
					//Reset charge delay
					if (!(cdChargeDelay & 0x0F))
						cdChargeDelay = 0;
				}
			}
		}
		
		//Friction
		uint16_t friction = acceleration;
		if (super)
			friction = 0x000C;
		
		if (!controlHeld.left && !controlHeld.right && inertia != 0)
		{
			if (inertia > 0)
			{
				inertia -= friction;
				if (inertia < 0)
					inertia = 0;
			}
			else
			{
				inertia += friction;
				if (inertia >= 0)
					inertia = 0;
			}
		}
	}
	
	//Convert our inertia into global speeds
	int16_t sin, cos;
	GetSine(angle, &sin, &cos);
	xVel = (cos * inertia) >> 8;
	yVel = (sin * inertia) >> 8;
	
	//Collide with walls
	CheckWallsOnGround();
}

//Rolling functions
void PLAYER::ChkRoll()
{
	if (!status.inBall)
	{
		//Enter ball state
		status.inBall = true;
		xRadius = rollXRadius;
		yRadius = rollYRadius;
		anim = PLAYERANIMATION_ROLL;
		
		//This is supposed to keep us on the ground, but when we're on a ceiling, it does... not that
		YSHIFT_ON_FLOOR(5);
		
		//Play the sound (if not charging a CD spindash)
		if (!cdSPTimer)
			PlaySound(SOUNDID_ROLL);
		
		#ifndef SONICCD_ROLLING
			//Leftover from Sonic 1's S-tubes
			if (inertia == 0)
				inertia = 0x200;
		#else
			//I don't know why it's like this in CD
			if (inertia >= 0 && inertia < 0x200)
				inertia = 0x200;
		#endif
	}
}

void PLAYER::Roll()
{
	if (!status.isSliding && !controlHeld.left && !controlHeld.right && !status.inBall)
	{
		#ifndef SONIC123_ROLL_DUCK
			if (controlHeld.down)
			{
				if (abs(inertia) >= 0x100)
					ChkRoll();
				else if (cdSPTimer == 0)
				{
					#ifndef FIX_DUCK_CONDITION
						anim = PLAYERANIMATION_DUCK;
					#else
						if (((angle + 0x20) & 0xC0) == 0)
							anim = PLAYERANIMATION_DUCK;
						else
							anim = PLAYERANIMATION_WALK;
					#endif
				}
			}
			else if (anim == PLAYERANIMATION_DUCK)
			{
				//Revert to walk animation if was ducking
				anim = PLAYERANIMATION_WALK;
			}
		#else
			if (controlHeld.down && abs(inertia) >= 0x80)
				ChkRoll();
		#endif
	}
}

void PLAYER::RollLeft()
{
	if (inertia <= 0)
	{
		status.xFlip = true;
		anim = PLAYERANIMATION_ROLL;
	}
	else
	{
		inertia -= 0x20;
		if (inertia < 0)
			inertia = -0x80;
	}
}
	
void PLAYER::RollRight()
{
	if (inertia >= 0)
	{
		status.xFlip = false;
		anim = PLAYERANIMATION_ROLL;
	}
	else
	{
		inertia += 0x20;
		if (inertia >= 0)
			inertia = 0x80;
	}
}

void PLAYER::RollSpeed()
{
	if (!status.isSliding)
	{
		//Decelerate if pulling back
		if (!status.pinballMode && !moveLock)
		{
			if (controlHeld.left)
				RollLeft();
			if (controlHeld.right)
				RollRight();
		}
		
		#ifdef SONICCD_SPINDASH
			//Check if we're doing a CD spindash
			if (cdSPTimer != 0)
			{
				//Handle CD spindash
				int16_t spindashAccel = 0x4B;
				int16_t spindashCap = top << 1;
				
				if (item.hasSpeedShoes)
					spindashCap -= (top >> 1);
				
				//Reverse if facing left
				if (status.xFlip)
				{
					spindashAccel = -spindashAccel;
					spindashCap = -spindashCap;
				}
				
				//Increase spindash speed
				inertia += spindashAccel;
				
				//Cap our inertia
				if (!status.xFlip)
				{
					if (inertia > spindashCap)
						inertia = spindashCap;
				}
				else
				{
					if (inertia < spindashCap)
						inertia = spindashCap;
				}
				
				//Check if we're still holding down
				if (!controlHeld.down)
				{
					if (cdSPTimer != 45)
					{
						//Failed to spindash
						cdSPTimer = 0;
						inertia = 0;
						xVel = 0;
						yVel = 0;
						return;
					}
					else
					{
						//Successfully spindashed
						PlaySound(SOUNDID_SPINDASH_RELEASE);
						cdSPTimer = 0;
						
						if (status.xFlip)
							RollLeft();
						else
							RollRight();
					}
				}
				else
					return;
			}
		#else
			//Cancel peelout
			if (cdSPTimer)
			{
				inertia = 0;
				cdSPTimer = 0;
			}
		#endif
		
		//Friction
		if (inertia > 0)
		{
			inertia -= rollDeceleration;
			if (inertia < 0)
				inertia = 0;
		}
		else if (inertia < 0)
		{
			inertia += rollDeceleration;
			if (inertia >= 0)
				inertia = 0;
		}
		
		//Stop if slowed down
	#ifndef SONIC123_ROLL_DUCK
		if (abs(inertia) < 0x80)
	#else
		if (inertia == 0)
	#endif
		{
			if (!status.pinballMode)
			{
				//Exit ball form
				status.inBall = false;
				xRadius = defaultXRadius;
				yRadius = defaultYRadius;
				anim = PLAYERANIMATION_IDLE;
				YSHIFT_ON_FLOOR(-5);
			}
			else
			{
				//Speed us back up if we slow down
				if (status.xFlip)
					inertia = -0x400;
				else
					inertia = 0x400;
			}
		}
	}
	
	//Convert our inertia into global speeds
	int16_t sin, cos;
	GetSine(angle, &sin, &cos);
	xVel = (cos * inertia) >> 8;
	yVel = (sin * inertia) >> 8;
	
	//Cap our global horizontal speed
	if (xVel <= -0x1000)
		xVel = -0x1000;
	if (xVel >= 0x1000)
		xVel = 0x1000;
	
	//Collide with walls
	CheckWallsOnGround();
}

//Hurt and death functions
void PLAYER::DeadCheckRespawn()
{
	//Lock our camera
	int16_t cameraY = gLevel->camera->y;
	cameraLock = true;
	
	//Stop the timer
	gLevel->updateTime = false;
	
	//Check if we're off-screen
	#ifndef SONIC12_DEATH_RESPAWN
		if (status.reverseGravity)
		{
			if (y.pos > cameraY - 0x10)
				return;
		}
		else
		{
			if (y.pos < cameraY + (gRenderSpec.height + 0x20))
				return;
		}
	#else
		if ((unsigned)y.pos < gLevel->bottomBoundaryTarget + 0x20)
			return;
	#endif
	
	//Enter respawn state
	routine = PLAYERROUTINE_RESET_LEVEL;
	restartCountdown = 60;
	
	gLives--;
}

void PLAYER::HurtStop()
{
	//Check if we've fallen off the stage, and die
	if (status.reverseGravity)
	{
		if (y.pos < gLevel->topBoundaryTarget)
		{
			KillCharacter(SOUNDID_HURT);
			return;
		}
	}
	else
	{
		if (y.pos >= gLevel->bottomBoundaryTarget)
		{
			KillCharacter(SOUNDID_HURT);
			return;
		}
	}
	
	//Check if we've touched the ground
	DoLevelCollision();
	
	if (!status.inAir)
	{
		//Clear our speed
		xVel = 0;
		yVel = 0;
		inertia = 0;
		
		//Clear our object control
		objectControl.disableOurMovement = false;
		objectControl.disableAnimation = false;
		objectControl.disableWallCollision = false;
		objectControl.disableObjectInteract = false;
		
		//Reset animation and priority
		anim = PLAYERANIMATION_WALK;
		priority = 2;
		
		//Set routine
		routine = PLAYERROUTINE_CONTROL;
		
		//Restart invulnerability timer (why)
		invulnerabilityTime = 120;
		spindashing = false;
	}
}

bool PLAYER::KillCharacter(SOUNDID soundId)
{
	if (debug == 0)
	{
		//Reset our state
		shield = SHIELD_NULL;
		item.isInvincible = false;
		item.hasSpeedShoes = false;
		item.shieldReflect = false;
		item.immuneFire = false;
		item.immuneElectric = false;
		item.immuneWater = false;
		
		routine = PLAYERROUTINE_DEATH;
		LandOnFloor_ExitBall();
		status.inAir = true;
		
		//Clear shield animation so it doesn't show during death
		shieldObject->anim = 0;
		
		//Set our velocity
		xVel = 0;
		yVel = -0x700;
		inertia = 0;
		
		//If the lead player, freeze level
		if (follow == nullptr)
			gLevel->updateStage = false;
		
		//Do animation and sound
		anim = PLAYERANIMATION_DEATH;
		highPriority = true;
		PlaySound(soundId);
		return true;
	}
	
	return false;
}

bool PLAYER::CheckHurt(void *hit)
{
	OBJECT *object = (OBJECT*)hit;
	
	//Check our shield and invincibility state
	if (shield != SHIELD_NULL || item.isInvincible)
	{
		//If we're immune to the object, don't hurt us
		if ((item.immuneFire && object->hurtType.fire) || (item.immuneElectric && object->hurtType.electric) || (item.immuneWater && object->hurtType.water))
			return true;
	}
	
	//Don't check for reflection if we are invincible, don't have a shield (and not using insta-shield, but... this check is fucked because isInvincible is set during the insta-shield here)
	if ((jumpAbility == 1 || item.shieldReflect) && !item.isInvincible)
	{
		//If we should be reflected, reflect
		if (object->hurtType.reflect)
		{
			//Get the velocity to reflect at
			int16_t xVel, yVel;
			uint8_t angle = GetAtan(x.pos - object->x.pos, y.pos - object->y.pos);
			GetSine(angle, &yVel, &xVel);
			
			object->xVel = (xVel * -0x800) >> 8;
			object->yVel = (yVel * -0x800) >> 8;
			
			//Clear the object's collision
			object->collisionType = COLLISIONTYPE_ENEMY;
			object->touchWidth = 0;
			object->touchHeight = 0;
			return true;
		}
	}
	
	//Check if we can be hurt
	if (item.isInvincible)
		return true;
	else if (invulnerabilityTime <= 0)
		return HurtCharacter(hit);
	
	return false;
}

bool PLAYER::HurtCharacter(void *hit)
{
	OBJECT *object = (OBJECT*)hit;
	
	//If a spike object, use the spike hurt sound
	SOUNDID soundId = SOUNDID_HURT;
	
	//TODO: above
	
	//Get which ring count to use
	unsigned int *rings = &gRings; //TODO: multiplayer stuff
	
	//If we have a shield, lose it, otherwise, lose rings
	if (shield != SHIELD_NULL)
	{
		shield = SHIELD_NULL;
		item.shieldReflect = false;
		item.immuneFire = false;
		item.immuneElectric = false;
		item.immuneWater = false;
	}
	else
	{
		//Check if we should die
		if (*rings == 0)
		{
			//Die, using the sound id we've gotten
			return KillCharacter(soundId);
		}
		else
		{
			//Lose rings
			OBJECT *ringObject = new OBJECT(&gLevel->objectList, &ObjBouncingRing);
			ringObject->x.pos = x.pos;
			ringObject->y.pos = y.pos;
			ringObject->parent = (void*)this;
		}
	}
	
	//Enter hurt routine
	routine = PLAYERROUTINE_HURT;
	LandOnFloor_ExitBall();
	status.inAir = true;
	
	//Get our hurt velocity
	xVel = status.underwater ? 0x100 : 0x200;
	yVel = status.underwater ? -0x200 : -0x400;
	
	if (x.pos < object->x.pos)
		xVel = -xVel;
	
	//Other stuff for getting hurt
	inertia = 0;
	anim = PLAYERANIMATION_HURT;
	invulnerabilityTime = 120;
	
	PlaySound(soundId);
	return true;
}

//Level boundary function
void PLAYER::LevelBoundSide(int32_t bound)
{
	//Set our position to the boundary
	x.pos = bound;
	x.sub = 0;
	
	//Set our speed to 0
	xVel = 0;
	inertia = 0;
}

void PLAYER::LevelBound()
{
	//Get our next position and boundaries
	#ifdef FIX_HORIZONTAL_WRAP
		#define lbType int16_t
	#else
		#define lbType uint16_t
	#endif
	
	lbType nextPos = (xPosLong + (xVel << 8)) >> 16;
	lbType leftBound = gLevel->leftBoundary + 0x10;
	lbType rightBound = gLevel->rightBoundary + 0x40 - 0x18;
	
	//Clip us into the boundaries
	if (nextPos < leftBound)
		LevelBoundSide(leftBound);
	else if (nextPos > rightBound)
		LevelBoundSide(rightBound);
	
	//Die if reached bottom boundary
#ifndef SONIC1_DEATH_BOUNDARY
	if (status.reverseGravity ? (y.pos <= gLevel->topBoundaryTarget) : (y.pos >= gLevel->bottomBoundaryTarget))
#else
	if (status.reverseGravity ? (y.pos <= gLevel->topBoundary) : (y.pos >= gLevel->bottomBoundary))
#endif
	{
		x.pos = nextPos;
		x.sub = 0;
		xVel = 0;
		inertia = 0;
		KillCharacter(SOUNDID_HURT);
	}
}

//Super and hyper code
static const uint8_t sonicPalette[16][4][3] = {
	{{0x24, 0x24, 0xB6}, {0x24, 0x48, 0xDA}, {0x48, 0x48, 0xFF}, {0x6C, 0x6C, 0xFF}},
	{{0x48, 0x48, 0x91}, {0x48, 0x6D, 0xB6}, {0x6D, 0x6D, 0xFF}, {0x91, 0x91, 0xFF}},
	{{0x6D, 0x6D, 0x6D}, {0x6D, 0x91, 0xB6}, {0x91, 0x91, 0xFF}, {0xB6, 0xB6, 0xFF}},
	{{0x91, 0x91, 0x48}, {0x91, 0xB6, 0xB6}, {0xB6, 0xB6, 0xFF}, {0xDF, 0xDF, 0xFF}},
	{{0xB6, 0xB6, 0x48}, {0xB6, 0xDF, 0xB6}, {0xDA, 0xDA, 0xFF}, {0xFF, 0xFF, 0xFF}},
	{{0xDA, 0xDA, 0x48}, {0xDA, 0xFF, 0xB6}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x48}, {0xFF, 0xFF, 0xB6}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x6D}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x91}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x6D}, {0xFF, 0xFF, 0xDA}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x48}, {0xFF, 0xFF, 0xB6}, {0xFF, 0xFF, 0xFF}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x24}, {0xFF, 0xFF, 0x91}, {0xFF, 0xFF, 0xDA}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0x6D}, {0xFF, 0xFF, 0xB6}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0x48}, {0xFF, 0xFF, 0x91}, {0xFF, 0xFF, 0xDA}},
	{{0xFF, 0xFF, 0x00}, {0xFF, 0xFF, 0x6D}, {0xFF, 0xFF, 0xB6}, {0xFF, 0xFF, 0xFF}},
	{{0xFF, 0xFF, 0x24}, {0xFF, 0xFF, 0x91}, {0xFF, 0xFF, 0xDA}, {0xFF, 0xFF, 0xFF}},
};

#define SET_PALETTE_FROM_ENTRY(entry)	ModifyPaletteColour(&texture->loadedPalette->colour[0x2], entry[0][0], entry[0][1], entry[0][2]);	\
										ModifyPaletteColour(&texture->loadedPalette->colour[0x3], entry[1][0], entry[1][1], entry[1][2]);	\
										ModifyPaletteColour(&texture->loadedPalette->colour[0x4], entry[2][0], entry[2][1], entry[2][2]);	\
										ModifyPaletteColour(&texture->loadedPalette->colour[0x5], entry[3][0], entry[3][1], entry[3][2]);

void PLAYER::SuperPaletteCycle()
{
	switch (paletteState)
	{
		case PALETTESTATE_REGULAR:
		{
			SET_PALETTE_FROM_ENTRY(sonicPalette[0]);
			break;
		}
		case PALETTESTATE_FADING_IN:
		{
			//Wait for 4 frames before updating
			if (--paletteTimer >= 0)
				break;
			paletteTimer = 3;
			
			//Increment frame and check for fade-in completion
			int16_t prevFrame = paletteFrame++;
			
			if (paletteFrame >= 6)
			{
				paletteState = PALETTESTATE_SUPER;
				memset(&objectControl, 0, sizeof(objectControl));
			}
			
			SET_PALETTE_FROM_ENTRY(sonicPalette[prevFrame]);
			break;
		}
		case PALETTESTATE_SUPER:
		{
			//Wait for 8 frames before updating
			if (--paletteTimer >= 0)
				break;
			paletteTimer = 8;
			
			//Increment frame and set palette
			int16_t prevFrame = paletteFrame++;
			if (paletteFrame >= 15)
				paletteFrame = 6;
			SET_PALETTE_FROM_ENTRY(sonicPalette[prevFrame]);
			break;
		}
		case PALETTESTATE_FADING_OUT:
		{
			//Wait for 4 frames before updating
			if (--paletteTimer >= 0)
				break;
			paletteTimer = 3;
			
			//Decrement frame and check for fade-out completion
			int16_t prevFrame = paletteFrame--;
			
			if (paletteFrame >= prevFrame)
			{
				paletteState = PALETTESTATE_REGULAR;
				paletteFrame = 0;
			}
			
			SET_PALETTE_FROM_ENTRY(sonicPalette[prevFrame]);
			break;
		}
	}
}

bool PLAYER::SuperTransform()
{
	if (!super && gLevel->updateTime && gRings >= 50) //Super transformation
	{
		//Set our super state
		paletteState = PALETTESTATE_FADING_IN;
		paletteTimer = 15;
		
		superTimer = 60;
		super = true;
		
		objectControl.disableOurMovement = true;
		objectControl.disableObjectInteract = true;
		anim = PLAYERANIMATION_TRANSFORM;
		
		//Super and hyper specific stuff (stars and trail stuff)
		if (0)
		{
			//TODO: Super effects
		}
		else
		{
			//TODO: Hyper effects
		}
		
		//Become invincible and set speed
		invincibilityTime = 0;
		item.isInvincible = true;
		
		if (!item.hasSpeedShoes)
			SetSpeedFromDefinition(status.underwater ? underwaterSuperSD : superSD);
		else
			SetSpeedFromDefinition(status.underwater ? underwaterSuperSpeedShoesSD : superSpeedShoesSD);
		
		//Play super theme (if lead) and transformation sound
		if (follow == nullptr)
			gLevel->ChangeSecondaryMusic(gLevel->superMusic);
		PlaySound(SOUNDID_SUPER_TRANSFORM);
		return true;
	}
	
	return false;
}

void PLAYER::UpdateSuper()
{
	if (super)
	{
		if (gLevel->updateTime)
		{
			//Wait about 60 frames (61) before depleting rings
			if (--superTimer >= 0)
				return;
			superTimer = 60;
			
			//Check if we've run out of rings
			if (gRings != 0)
			{
				if (--gRings > 0)
					return;
			}
		}
		
		//Revert to regular
		paletteState = PALETTESTATE_FADING_OUT;
		paletteFrame = 5;
		super = false;
		prevAnim = PLAYERANIMATION_RUN;
		invincibilityTime = 1;
		
		if (!item.hasSpeedShoes)
			SetSpeedFromDefinition(status.underwater ? underwaterNormalSD : normalSD);
		else
			SetSpeedFromDefinition(status.underwater ? underwaterSpeedShoesSD : speedShoesSD);
	}
}

//Animation update
void PLAYER::FrameCommand(const uint8_t* animation)
{
	switch (animation[1 + animFrame])
	{
		case 0xFF: //Restart animation
			animFrame = 0;
			break;
		case 0xFE: //Go back X amount of frames
			animFrame -= animation[2 + animFrame];
			break;
		case 0xFD: //Switch to X animation
			anim = (PLAYERANIMATION)animation[2 + animFrame];
			return;
		default:
			return;
	}
	
	//Advance frame
	mappingFrame = animation[1 + animFrame];
	animFrame++;
}

void PLAYER::AdvanceFrame(const uint8_t* animation)
{
	//Handle commands
	if (animation[1 + animFrame] >= 0xFC)
	{
		FrameCommand(animation);
		return;
	}
	
	//Advance frame otherwise
	mappingFrame = animation[1 + animFrame];
	animFrame++;
}

const uint8_t flipTypeMapping[] = {0, MAPPINGFRAME_FLIP1 + 12, MAPPINGFRAME_FLIP1 + 24, MAPPINGFRAME_FLIP1 + 24};

void PLAYER::Animate()
{
	//Get our animation list to reference
	const uint8_t **aniList;
	if (super)
		aniList = animationListSuper;
	else
		aniList = animationList;
	
	//Handle animation reset based on prevAnim
	if (anim != prevAnim)
	{
		prevAnim = anim;
		animFrame = 0;
		animFrameDuration = 0;
		status.pushing = false;
	}
	
	//Get the animation to reference
	const uint8_t *animation = aniList[anim];
	
	if (animation[0] < 0x80) //Generic animation
	{
		//Wait for next frame (and copy our flip)
		renderFlags.xFlip = status.xFlip;
		renderFlags.yFlip = false;
		
		if (--animFrameDuration >= 0)
			return;
		animFrameDuration = animation[0];
		
		AdvanceFrame(animation);
		return;
	}
	else
	{
		if (animation[0] == 0xFF) //Is a walking variant
		{
			if (flipType == 0 && flipAngle == 0) //Not flipping
			{
				#ifdef SONIC1_WALK_ANIMATION
					if (--animFrameDuration >= 0)
					{
						if (status.reverseGravity)
							renderFlags.yFlip ^= 1;
						return;
					}
				#endif
				
				//Get the 45 degree increment to rotate our sprites at
				uint8_t rotAngle = angle;
				
				#ifndef SONIC1_SLOPE_ROTATION
					if (rotAngle < 0x80)
						rotAngle--;
				#endif
				
				if (!status.xFlip) //Invert angle if not flipped horizontally
					rotAngle = ~rotAngle;
				
				rotAngle += 0x10;
				
				//Set our horizontal and vertical flip if tilted over 180 degrees
				if (rotAngle >= 0x80)
				{
					renderFlags.xFlip = !status.xFlip;
					renderFlags.yFlip = true;
				}
				else
				{
					renderFlags.xFlip = status.xFlip;
					renderFlags.yFlip = false;
				}
				
				//Handle our animation
				if (!status.pushing) //Not pushing
				{
					//Get our angle increment
					uint8_t angleIncrement = (rotAngle / 0x20) & 0x3; //Halved from the original, so this is the actual increment
					
					//Get our speed factor
					uint16_t speedFactor = abs(inertia);
					if (status.isSliding)
						speedFactor *= 2;
					
					if (!super)
					{
						//Get the animation to use
						if (speedFactor >= 0x600)
						{
							//Run animation (at or above 0x600 speed)
							animation = aniList[PLAYERANIMATION_RUN];
							angleIncrement *= RUN_FRAMES;
						}
						else
						{
							//Walk animation (below 0x600 speed)
							animation = aniList[PLAYERANIMATION_WALK];
							angleIncrement *= WALK_FRAMES;
						}
						
						//Check if our animation is going to loop
						if (animation[1 + animFrame] == 0xFF)
							animFrame = 0;
						
						mappingFrame = animation[1 + animFrame] + angleIncrement;

						#ifdef SONIC1_WALK_ANIMATION
							//Set our frame duration
							speedFactor = -speedFactor + 0x800;
							if (speedFactor >= 0x8000)
								speedFactor = 0;
							animFrameDuration = speedFactor >> 8;
							
							//Increment frame
							animFrame++;
						#else
							//Wait for the next frame
							if (--animFrameDuration < 0)
							{
								//Set our frame duration
								speedFactor = -speedFactor + 0x800;
								if (speedFactor >= 0x8000)
									speedFactor = 0;
								animFrameDuration = speedFactor >> 8;
								
								//Increment frame
								animFrame++;
							}
						#endif
						return;
					}
					else
					{
						//Get the animation to use
						if (speedFactor >= 0x800)
						{
							//Run animation (at or above 0x600 speed)
							animation = aniList[PLAYERANIMATION_RUN];
							angleIncrement *= SUPER_FLY_FRAMES;
						}
						else
						{
							//Walk animation (below 0x600 speed)
							animation = aniList[PLAYERANIMATION_WALK];
							angleIncrement *= WALK_FRAMES;
						}
						
						//Check if our animation is going to loop
						if (animation[1 + animFrame] == 0xFF)
							animFrame = 0;
						
						//Set frame
						mappingFrame = animation[1 + animFrame] + angleIncrement;
						if (animation == aniList[PLAYERANIMATION_WALK] && gLevel->frameCounter & 0x3)
							mappingFrame += WALK_FRAMES * 4;

						#ifdef SONIC1_WALK_ANIMATION
							//Set our frame duration
							speedFactor = -speedFactor + 0x800;
							if (speedFactor >= 0x8000)
								speedFactor = 0;
							animFrameDuration = speedFactor >> 8;
							
							//Increment frame
							animFrame++;
						#else
							//Wait for the next frame
							if (--animFrameDuration < 0)
							{
								//Set our frame duration
								speedFactor = -speedFactor + 0x800;
								if (speedFactor >= 0x8000)
									speedFactor = 0;
								animFrameDuration = speedFactor >> 8;
								
								//Increment frame
								animFrame++;
							}
						#endif
						return;
					}
				}
				else
				{
					//Set animation and flip
					animation = aniList[PLAYERANIMATION_PUSH];
					renderFlags.xFlip = status.xFlip;
					renderFlags.yFlip = false;
					
					#ifndef SONIC1_WALK_ANIMATION
						//Wait for next frame
						if (--animFrameDuration >= 0)
							return;
					#endif
					
					//Set frame duration
					uint16_t speedFactor = -abs(inertia) + 0x800;
					if (speedFactor >= 0x8000)
						speedFactor = 0;
					animFrameDuration = speedFactor / 0x40;
					
					//Advance frame
					AdvanceFrame(animation);
					return;
				}
			}
			else
			{
				//Flipping
				uint8_t type = flipType & 0x7F; //Don't include the direction flip bit
				
				if (type == 0)
				{
					uint8_t thisAngle = flipAngle;
					
					if (!status.xFlip)
					{
						//Clear our render flip
						renderFlags.xFlip = false;
						renderFlags.yFlip = false;
						
						//Set our mapping frame according to angle and the type
						if (flipType >= 0x80)
						{
							renderFlags.yFlip = true;
							thisAngle = ~thisAngle;
							thisAngle += 0x8F;
						}
						else
						{
							thisAngle += 0xB;
						}
						
						mappingFrame = (thisAngle / 0x16) + MAPPINGFRAME_FLIP1;
						animFrameDuration = 0;
					}
					else
					{
						//Clear our render flip
						renderFlags.xFlip = false;
						renderFlags.yFlip = false;
						
						//Set our mapping frame according to angle and the type
						if (flipType >= 0x80)
						{
							renderFlags.xFlip = true;
							thisAngle += 0xB;
						}
						else
						{
							renderFlags.xFlip = true;
							renderFlags.yFlip = true;
							thisAngle = ~thisAngle;
							thisAngle += 0x8F;
						}
						
						mappingFrame = (thisAngle / 0x16) + MAPPINGFRAME_FLIP1;
						animFrameDuration = 0;
					}
				}
				else
				{
					uint8_t mapFrame = type < 4 ? flipTypeMapping[type] : 0; //The original doesn't account for type being 4 or greater
					
					switch (type)
					{
						case 1:
						{
							uint8_t thisAngle = flipAngle;
							
							if (!status.xFlip)
							{
								//Clear our render flip
								renderFlags.xFlip = false;
								renderFlags.yFlip = false;
								
								//Set our mapping frame
								mappingFrame = ((thisAngle - 0x8) / 0x16) + mapFrame;
								animFrameDuration = 0;
							}
							else
							{
								//Clear our render flip
								renderFlags.xFlip = true;
								renderFlags.yFlip = false;
								
								//Set our mapping frame
								mappingFrame = ((thisAngle - 0x8) / 0x16) + mapFrame;
								animFrameDuration = 0;
							}
							break;
						}
						case 2:
						{
							uint8_t thisAngle = flipAngle;
							
							if (!status.xFlip)
							{
								//Clear our render flip
								renderFlags.xFlip = false;
								renderFlags.yFlip = false;
								
								//Set our mapping frame
								mappingFrame = ((thisAngle + 0xB) / 0x16) + mapFrame;
								animFrameDuration = 0;
							}
							else
							{
								//Clear our render flip
								renderFlags.xFlip = true;
								renderFlags.yFlip = true;
								
								//Set our mapping frame
								mappingFrame = (((~thisAngle) + 0x8F) / 0x16) + mapFrame;
								animFrameDuration = 0;
							}
							break;
						}
						case 3:
						{
							uint8_t thisAngle = flipAngle;
							
							if (!status.xFlip)
							{
								//Clear our render flip
								renderFlags.xFlip = false;
								renderFlags.yFlip = true;
								
								//Set our mapping frame
								mappingFrame = (((~thisAngle) + 0x8F) / 0x16) + mapFrame;
								animFrameDuration = 0;
							}
							else
							{
								//Clear our render flip
								renderFlags.xFlip = true;
								renderFlags.yFlip = false;
								
								//Set our mapping frame
								mappingFrame = ((thisAngle + 0xB) / 0x16) + mapFrame;
								animFrameDuration = 0;
							}
							break;
						}
						case 4:
						{
							uint8_t thisAngle = flipAngle;
							
							if (!status.xFlip)
							{
								//Clear our render flip
								renderFlags.xFlip = false;
								renderFlags.yFlip = false;
								
								//Get the angle to use
								if (flipType >= 0x80)
									thisAngle += 0xB;
								else
									thisAngle += 0xB; //nice
								
								//Set our mapping frame
								mappingFrame = (thisAngle / 0x16) + MAPPINGFRAME_FLIP1;
								animFrameDuration = 0;
							}
							else
							{
								//Get the angle to use and set our render flip
								if (flipType >= 0x80)
								{
									renderFlags.xFlip = true;
									renderFlags.yFlip = true;
									thisAngle = (~thisAngle) + 0x8F;
								}
								else
								{
									renderFlags.xFlip = true; //nice
									renderFlags.yFlip = true; //nice
									thisAngle = (~thisAngle) + 0x8F; //nice
								}
								
								//Set our mapping frame
								mappingFrame = (thisAngle / 0x16) + MAPPINGFRAME_FLIP1;
								animFrameDuration = 0;
							}
							break;
						}
						default:
						{
							uint8_t thisAngle = flipAngle;
							
							//Clear our render flip
							renderFlags.xFlip = status.xFlip;
							renderFlags.yFlip = false;
							
							//Set our mapping frame
							mappingFrame = ((thisAngle + 0xB) / 0x16) + MAPPINGFRAME_FLIP1;
							animFrameDuration = 0;
							break;
						}
					}
				}
			}
		}
		else //Rolling animation
		{
			//Copy our flip
			renderFlags.xFlip = status.xFlip;
			renderFlags.yFlip = false;
			
			//Wait for next frame
			if (--animFrameDuration >= 0)
				return;
			
			//Get our speed factor
			uint16_t speedFactor = abs(inertia);
			
			//Set our roll animation and duration
			if (speedFactor < 0x600)
				animation = aniList[PLAYERANIMATION_ROLL];
			else
				animation = aniList[PLAYERANIMATION_ROLL2];
			
			speedFactor = -speedFactor + 0x400;
			if (speedFactor >= 0x8000)
				speedFactor = 0;
			animFrameDuration = speedFactor >> 8;
			
			//Advance frame
			AdvanceFrame(animation);
			return;
		}
	}
}

//CPU control code
void PLAYER::CPUControl()
{
	PLAYER *followPlayer = (PLAYER*)follow;
	STAT_RECORD goal = followPlayer->statRecord[(followPlayer->recordPos - 16) % (unsigned)PLAYER_RECORD_LENGTH];
	
	controlHeld = goal.controlHeld;
	controlPress = goal.controlPress;
}

//Update
void PLAYER::ControlRoutine()
{
	//Standing / walking on ground
	if (status.inBall == false && status.inAir == false)
	{
		//Handle transitioning from Sonic's idle animations to movement
		if (!(controlPress.a || controlPress.b || controlPress.c))
		{
			//If we're still getting up / blinking, don't update this frame
			if (anim == PLAYERANIMATION_BLINK || anim == PLAYERANIMATION_GETUP)
				return;
			
			//Check if we're trying to move out of an idle animation
			if (anim == PLAYERANIMATION_IDLE && animFrame >= 30)
			{
				if (!(controlHeld.left || controlHeld.up || controlHeld.right || controlHeld.down || controlHeld.a || controlHeld.b || controlHeld.c))
					return;
				
				if (animFrame >= 172)
					anim = PLAYERANIMATION_GETUP;
				else
					anim = PLAYERANIMATION_BLINK;
				return;
			}
		}
		
		if (Spindash() && Jump())
		{
			//Handle slope gravity and our movement
			SlopeResist();
			Move();
			Roll();
			
			//Keep us in level bounds
			LevelBound();
			
			//Move according to our velocity
			xPosLong += xVel << 8;
			if (status.reverseGravity)
				yPosLong -= yVel << 8;
			else
				yPosLong += yVel << 8;
			
			//Handle collision and falling off of slopes
			AnglePos();
			SlopeRepel();
		}
	}
	else if (status.inBall == true && status.inAir == false)
	{
		if (status.pinballMode || Jump())
		{
			//Handle slope gravity and our movement
			RollRepel();
			RollSpeed();
			
			//Keep us in level bounds
			LevelBound();
			
			if (cdSPTimer == 0)
			{
				//Move according to our velocity
				xPosLong += xVel << 8;
				if (status.reverseGravity)
					yPosLong -= yVel << 8;
				else
					yPosLong += yVel << 8;
			}
			
			//Handle collision and falling off of slopes
			AnglePos();
			SlopeRepel();
		}
	}
	//In mid-air
	else if (status.inAir == true)
	{
		//Handle our movement
		JumpHeight();
		ChgJumpDir();
		
		//Keep us in level bounds
		LevelBound();
		
		//Move according to our velocity
		xPosLong += xVel << 8;
		if (status.reverseGravity)
			yPosLong -= yVel << 8;
		else
			yPosLong += yVel << 8;
		
		//Gravity (0x38 above water, 0x10 below water)
		yVel += 0x38;
		if (status.underwater)
			yVel -= 0x28;
		
		//Handle our angle receding when we run / jump off of a ledge
		JumpAngle();
		
		//Handle collision
		DoLevelCollision();
	}
}

void PLAYER::Update()
{
	//Clear drawing flag
	isDrawing = false;
	
	//Update (check if we're using debug, first)
	switch (debug & 0xFF)
	{
		case 0:
		{
			#if (defined(SONICCD_PEELOUT) || defined(SONICCD_SPINDASH))
				//Update peelout / spindash
				uint8_t nextSPTimer = cdSPTimer;
				
				if (nextSPTimer != 0)
				{
					//Increment and cap appropriately
					nextSPTimer++;
					
					if (status.inBall)
					{
						//Spindash cap
						if (nextSPTimer >= 45)
							nextSPTimer = 45;
					}
					else
					{
						//Peelout cap
						if (nextSPTimer >= 30)
							nextSPTimer = 30;
					}
					
					//Copy our next value
					cdSPTimer = nextSPTimer;
				}
			#endif
			
			//Run main player code
			switch (routine)
			{
				case PLAYERROUTINE_CONTROL:
					//Handle our debug buttons
					if (gDebugEnabled && controller == 0)
					{
						//Toggle our reverse gravity
						if (gController[controller].press.a)
							status.reverseGravity ^= 1;
						
						//Enable debug mode
						if (gController[controller].press.b)
						{
							//Unlock controls
							controlLock = false;
							
							//Enter the debug mode
							if (gController[controller].held.c)
								debug = 2; //Mapping tester
							else
								debug = 1; //Object placement
							return;
						}
					}
					
					//Copy the given controller's inputs if not locked
					if (follow == nullptr)
					{
						//If the lead, just directly copy our controller inputs
						if (!controlLock)
						{
							controlHeld = gController[controller].held;
							controlPress = gController[controller].press;
						}
					}
					else
					{
						//Copy our controller inputs
						if (!controlLock)
						{
							controlHeld = gController[controller].held;
							controlPress = gController[controller].press;
						}
						
						//Use our CPU to update our input
						CPUControl();
					}
					
					if (objectControl.disableOurMovement)
					{
						//Enable our jump abilities
						jumpAbility = 0;
					}
					else
					{
						//The original uses the two bits for a jump table, but we can't do that because it'd be horrible
						ControlRoutine();
					}
					
					//Draw, record position, and handle super and water
					Display();
					UpdateSuper();
					RecordPos();
					//bsr.w	Sonic_Water
					
					//Copy our angles to tilt
					nextTilt = primaryAngle;
					tilt = secondaryAngle;
					
					//Animation
					if (status.windTunnel && anim != PLAYERANIMATION_WALK)
						prevAnim = anim;
					
					if (!objectControl.disableAnimation)
					{
						//Run animation code
						Animate();
						
						//Flip vertically if gravity is reversed
						if (status.reverseGravity)
							renderFlags.yFlip ^= 1;
					}
					
					//Interact with objects
					if (!objectControl.disableObjectInteract)
						TouchResponse();
					break;
					
				case PLAYERROUTINE_HURT:
					//Handle our debug buttons
					if (gDebugEnabled && controller == 0)
					{
						if (gController[controller].press.b)
						{
							controlLock = false;
							debug = 1;
							return;
						}
					}
					
					//Move according to our velocity
					xPosLong += xVel << 8;
					if (status.reverseGravity)
						yPosLong -= yVel << 8;
					else
						yPosLong += yVel << 8;
					
					//Gravity (0x30 above water, 0x10 below water)
					yVel += 0x30;
					if (status.underwater)
						yVel -= 0x20;
					
					//Handle other movements
					HurtStop();
					LevelBound();
					RecordPos();
					
					//Draw
					Draw();
					
					//Animate
					Animate();
					if (status.reverseGravity)
						renderFlags.yFlip ^= 1;
					break;
					
				case PLAYERROUTINE_DEATH:
					//Handle our debug buttons
					if (gDebugEnabled && controller == 0)
					{
						if (gController[controller].press.b)
						{
							controlLock = false;
							debug = 1;
							return;
						}
					}
					
					//Check if we should respawn soon
					DeadCheckRespawn();
					
					//Move and fall
					xPosLong += xVel << 8;
					if (status.reverseGravity)
						yPosLong -= yVel << 8;
					else
						yPosLong += yVel << 8;
					yVel += 0x38;
					
					//Record pos and draw
					RecordPos();
					Draw();
					
					//Animate
					Animate();
					if (status.reverseGravity)
						renderFlags.yFlip ^= 1;
					break;
					
				case PLAYERROUTINE_RESET_LEVEL:
					//After a second, fade the level out to restart
					if (--restartCountdown == 0)
						gLevel->SetFade(false, false);
					break;
			}
			break;
		}
		case 1: //Object placement mode
		{
			DebugMode();
			Draw();
			break;
		}	
		case 2: //Mapping test mode
		{
			//Exit if B is pressed
			if (gController[controller].press.b)
				debug = 0;
			
			//Cycle through our mappings
			mappingFrame++;
			mappingFrame %= mappings->size;
			
			//Draw
			Draw();
			break;
		}
	}
	
	//Restart if start + a
	if (gController[controller].press.start && gController[controller].held.a)
		gLevel->SetFade(false, false);
	if (gController[controller].press.start && gController[controller].held.b)
	{
		gLevel->SetFade(false, false);
		gGameLoadLevel++;
		gGameLoadLevel %= LEVELID_MAX;
	}
	if (gController[controller].press.start && gController[controller].held.c)
	{
		if (shield == SHIELD_NULL)
			GiveShield(SOUNDID_GET_BLUE_SHIELD, SHIELD_BLUE);
		else if (shield == SHIELD_BLUE)
			GiveShield(SOUNDID_GET_FIRE_SHIELD, SHIELD_FIRE);
		else if (shield == SHIELD_FIRE)
			GiveShield(SOUNDID_GET_ELECTRIC_SHIELD, SHIELD_ELECTRIC);
		else if (shield == SHIELD_ELECTRIC)
			GiveShield(SOUNDID_GET_BUBBLE_SHIELD, SHIELD_BUBBLE);
		else if (shield == SHIELD_BUBBLE)
			GiveShield(SOUNDID_NULL, SHIELD_NULL);
	}
}

//Draw our player
void PLAYER::Display()
{
	//Handle invulnerability (decrement the invulnerability time and don't draw us every 4 frames)
	if (invulnerabilityTime == 0 || (--invulnerabilityTime & 0x4) != 0)
		Draw();
	
	//Handle invincibility (every 8 frames)
	if (item.isInvincible && invincibilityTime != 0 && (gLevel->frameCounter & 0x7) == 0 && --invincibilityTime == 0)
	{
		//Resume music
		if (follow == nullptr && (gLevel->secondaryMusic == gLevel->invincibilityMusic || gLevel->secondaryMusic == gLevel->superMusic))
		{
			if (super)
				gLevel->ChangeSecondaryMusic(gLevel->superMusic);
			else
				gLevel->StopSecondaryMusic();
		}
		
		//Lose invincibility
		item.isInvincible = false;
	}
	
	//Handle speed shoes (every 8 frames)
	if (item.hasSpeedShoes && speedShoesTime != 0 && (gLevel->frameCounter & 0x7) == 0 && --speedShoesTime == 0)
	{
		//Resume music
		if (follow == nullptr && gLevel->secondaryMusic == gLevel->speedShoesMusic)
		{
			if (super)
				gLevel->ChangeSecondaryMusic(gLevel->superMusic);
			else
				gLevel->StopSecondaryMusic();
		}
		
		//Lose speed shoes
		item.hasSpeedShoes = false;
		
		//Reset our speed
		if (!super)
			SetSpeedFromDefinition(status.underwater ? underwaterNormalSD : normalSD);
		else
			SetSpeedFromDefinition(status.underwater ? underwaterSuperSD : superSD);
	}
}

void PLAYER::Draw()
{
	//Set drawing flag
	isDrawing = true;
}

void PLAYER::DrawToScreen()
{
	if (isDrawing)
	{
		//Don't draw if we don't have textures or mappings
		if (texture != nullptr && mappings != nullptr)
		{
			//Draw our sprite
			RECT *mapRect = &mappings->rect[mappingFrame];
			POINT *mapOrig = &mappings->origin[mappingFrame];
			
			int origX = mapOrig->x;
			int origY = mapOrig->y;
			if (renderFlags.xFlip)
				origX = mapRect->w - origX;
			if (renderFlags.yFlip)
				origY = mapRect->h - origY;
			
			int alignX = renderFlags.alignPlane ? gLevel->camera->x : 0;
			int alignY = renderFlags.alignPlane ? gLevel->camera->y : 0;
			
			//Check if on-screen
			renderFlags.onScreen = false;
			
			//Horizontal check, uses widthPixels
			if (x.pos - alignX < -widthPixels || x.pos - alignX > gRenderSpec.width + widthPixels)
				return;
			
			//Vertical check, uses 32 pixels for height or heightPixels
			int16_t heightCheck = renderFlags.onScreenUseHeightPixels ? heightPixels : 32;
			if (y.pos - alignY < -heightCheck || y.pos - alignY > gRenderSpec.height + heightCheck)
				return;
			
			//We're on-screen, now set flag and draw
			renderFlags.onScreen = true;
			gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, mapRect, gLevel->GetObjectLayer(highPriority, priority), x.pos - origX - alignX, y.pos - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
			
			//Draw trail when using speed shoes or hyper
			if (item.hasSpeedShoes || hyper)
			{
				//If an even frame, use the position from 3 frames ago, odd frame, 5 frames ago
				int trailSeek = (gLevel->frameCounter & 0x1) ? 5 : 3;
				
				//Shorten if running out of speed shoes time
				if (hyper == false && item.hasSpeedShoes && speedShoesTime <= 1)
					trailSeek /= 2;
				
				//Draw at the position from the frame above
				int x = posRecord[(recordPos - trailSeek) % (unsigned)PLAYER_RECORD_LENGTH].x, y = posRecord[(recordPos - trailSeek) % (unsigned)PLAYER_RECORD_LENGTH].y;
				gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, mapRect, gLevel->GetObjectLayer(highPriority, priority), x - origX - alignX, y - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
			}
		}
	}
}

//Debug mode
void PLAYER::RestoreStateDebug()
{
	//Reset animation and subpixel position
	anim = PLAYERANIMATION_WALK;
	x.sub = 0;
	y.sub = 0;
	
	//Clear other state stuff
	memset(&objectControl, 0, sizeof(objectControl));
	spindashing = false;
	
	//Clear speeds and inertia
	xVel = 0;
	yVel = 0;
	inertia = 0;
	
	//Clear our status and return to the control routine
	memset(&status, 0, sizeof(status));
	routine = PLAYERROUTINE_CONTROL;
}

void PLAYER::DebugControl()
{
	//Move in the direction held
	CONTROLMASK selectedControl = gController[controller].held;
	
	if (gController[controller].held.up || gController[controller].held.down || gController[controller].held.left || gController[controller].held.right)
	{
		//Handle our acceleration and speed
		if (--debugAccel == 0)
		{
			debugAccel = 1;
			if (++debugSpeed == 0)
				debugSpeed = 0xFF;
		}
		else
			selectedControl = gController[controller].press;
		
		//Move according to our speed and direction
		int32_t calcSpeed = (debugSpeed + 1) << 12;
		
		if (selectedControl.up)
		{
			yPosLong -= calcSpeed;
			if (yPosLong < gLevel->topBoundaryTarget << 16)
				yPosLong = gLevel->topBoundaryTarget << 16;
		}
		else if (selectedControl.down)
		{
			yPosLong += calcSpeed;
			if (yPosLong > gLevel->bottomBoundaryTarget << 16)
				yPosLong = gLevel->bottomBoundaryTarget << 16;
		}
		
		if (selectedControl.left)
		{
			xPosLong -= calcSpeed;
			if (xPosLong < 0) //Doesn't check left boundary, just 0
				xPosLong = 0;
		}
		else if (selectedControl.right)
		{
			xPosLong += calcSpeed;
			//There's no boundary check here at all!
		}
	}
	else
	{
		//Reset our speed and timer
		debugAccel = 12;
		debugSpeed = 15;
	}
	
	//Handle pressed buttons
	if (gController[controller].press.b)
	{
		//Exit debug mode
		debug = 0;
		gLevel->updateStage = true;
		gLevel->updateTime = true;
		RestoreStateDebug();
		xRadius = defaultXRadius;
		yRadius = defaultYRadius;
	}
}

void PLAYER::DebugMode()
{
	switch (debug >> 8)
	{
		case 0:
			//Unlock camera
			cameraLock = false;
			
			//Initialize debug and routine
			debug += 0x100;
			debugAccel = 12;
			debugSpeed = 1;
		//Fallthrough
		case 1:
			DebugControl();
			break;
	}
}

//Ring attraction check
#define RING_ATTRACT_RADIUS 64

void PLAYER::RingAttractCheck(OBJECT *object)
{
	//Check object
	if (object->function == ObjRing && object->routine)
	{
		//If we're within range, change the object to the attraction type
		int xDiff = object->x.pos - x.pos + RING_ATTRACT_RADIUS;
		int yDiff = object->y.pos - y.pos + RING_ATTRACT_RADIUS;
		
		if (xDiff >= 0 && xDiff <= RING_ATTRACT_RADIUS * 2 && yDiff >= 0 && yDiff <= RING_ATTRACT_RADIUS * 2)
		{
			//Turn this ring into an attracted ring
			object->function = ObjAttractRing;
			object->parent = (void*)this;
		}
	}
	
	//Iterate and check children
	for (OBJECT *obj = object->children; obj != nullptr; obj = obj->next)
		RingAttractCheck(obj);
}

//Object interaction functions
bool PLAYER::TouchResponseObject(OBJECT *object, int16_t playerLeft, int16_t playerTop, int16_t playerWidth, int16_t playerHeight)
{
	//Check object
	if (object->collisionType != COLLISIONTYPE_NULL)
	{
		//Check if our hitboxes are colliding
		int16_t horizontalCheck = playerLeft - (object->x.pos - object->touchWidth);
		int16_t verticalCheck = playerTop - (object->y.pos - object->touchHeight);
		
		if (horizontalCheck >= -playerWidth && horizontalCheck <= object->touchWidth * 2 && verticalCheck >= -playerHeight && verticalCheck <= object->touchHeight * 2)
		{
			//If so, handle interaction
			switch (object->collisionType)
			{
				case COLLISIONTYPE_ENEMY:
				case COLLISIONTYPE_BOSS:
					//Check if we're gonna hurt the enemy, or ourselves
					if (item.isInvincible || anim == PLAYERANIMATION_SPINDASH || anim == PLAYERANIMATION_ROLL)
						return object->Hurt(this);
					
					//Check character specific states
					switch (characterType)
					{
						case CHARACTERTYPE_TAILS:
							//If not flying or underwater, hurt us
							if (jumpAbility == 0 || status.underwater)
								return CheckHurt(object);
							
							//If the object is just about above us, hurt them
							if (((GetAtan(x.pos - object->x.pos, y.pos - object->y.pos) + 0x20) & 0xFF) < 0x40)
								return object->Hurt(this);
							
							//Otherwise, hurt us
							return CheckHurt(object);
						case CHARACTERTYPE_KNUCKLES:
							//If gliding or sliding after gliding, hurt the object
							if (jumpAbility == 1 || jumpAbility == 3)
								return object->Hurt(this);
							
							//Otherwise, hurt us
							return CheckHurt(object);
						default:
							//No special abilities, hurt us
							return CheckHurt(object);
					}
					break;
				case COLLISIONTYPE_SPECIAL:
					//Yeah, no
					return true;
				case COLLISIONTYPE_HURT:
					//Hurt us and never don't do that
					return CheckHurt(object);
				case COLLISIONTYPE_OTHER:
					//Generic collision, set to the second routine if we weren't just hit
					if (invulnerabilityTime < 90)
						object->routine = 2;
					return true;
				case COLLISIONTYPE_MONITOR:
					//If moving upwards, make the monitor bounce upwards
					if (yVel < 0)
					{
						
						if ((y.pos - 0x10) >= object->y.pos)
						{
							//Reverse our y-velocity and bump the monitor upwards (make it fall, too!)
							yVel = -yVel;
							object->yVel = -0x180;
							object->routineSecondary = 4; //why is this 4, the code only checks if it's non-zero?
						}
					}
					//Check if we're the lead, and only break the monitor if so and if in the rolling animation
					else if (follow == nullptr && anim == PLAYERANIMATION_ROLL)
					{
						//Reverse our y-velocity and destroy the monitor
						yVel = -yVel;
						object->routine = 2;
						object->parent = (void*)this;
					}
					return true;
			}
		}
	}
	
	//Iterate through children, we haven't hit the parent
	for (OBJECT *obj = object->children; obj != nullptr; obj = obj->next)
	{
		if (TouchResponseObject(obj, playerLeft, playerTop, playerWidth, playerHeight))
			return true;
	}
	
	return false;
}

void PLAYER::TouchResponse()
{
	//Check for ring attraction
	if (shield == SHIELD_ELECTRIC)
		for (OBJECT *obj = gLevel->objectList; obj != nullptr; obj = obj->next)
			RingAttractCheck(obj);
	
	//Get our collision hitbox
	bool wasInvincible = item.isInvincible; //Remember if we were invincible, since this gets temporarily overwritten by the insta-shield hitbox
	int16_t playerLeft, playerTop, playerWidth, playerHeight;
	
	if ((shield == SHIELD_NULL && item.isInvincible == false && jumpAbility == 1)
		|| (status.shouldNotFall && interact != nullptr && interact->function == ObjMinecart && abs(interact->xVel) >= 0x200))
	{
		//Use insta-shield extended hitbox
		playerWidth = 24; //radius
		playerLeft = x.pos - playerWidth;
		playerWidth *= 2; //diameter
		
		playerHeight = 24; //radius
		playerTop = y.pos - playerHeight;
		playerHeight *= 2; //diameter
		
		//Make us invincible (don't worry, this gets undone by the time TouchResponse is ending)
		item.isInvincible = true;
	}
	else
	{
		//Use player's hitbox
		playerWidth = 8; //radius
		playerLeft = x.pos - playerWidth;
		playerWidth *= 2; //diameter
		
		playerHeight = yRadius - 3; //radius
		playerTop = y.pos - playerHeight;
		playerHeight *= 2; //diameter
		
		//Code to handle the ducking hitbox from Sonic 1 and 2 (TODO: it's supposed to check the mapping frame)
		if (anim == PLAYERANIMATION_DUCK)
		{
			playerTop += 12;
			playerHeight = 10;
		}
	}
	
	//Iterate through every object
	for (OBJECT *obj = gLevel->objectList; obj != nullptr; obj = obj->next)
	{
		//Check for collision with this object
		if (TouchResponseObject(obj, playerLeft, playerTop, playerWidth, playerHeight))
			break;
	}
	
	//Restore our original invincibility to before the insta-shield code
	item.isInvincible = wasInvincible;
}

void PLAYER::AttachToObject(OBJECT *object, bool *standingBit)
{
	//If already standing on an object, clear that object's standing bit
	if (status.shouldNotFall && interact != nullptr)
		*((bool*)interact + (size_t)standingBit) = false; //Clear the object we're standing on's standing bit
	
	//Set to stand on this object
	interact = object;
	angle = 0;
	yVel = 0;
	inertia = xVel;
	
	//Land on object
	status.shouldNotFall = true;
	*((bool*)object + (size_t)standingBit) = true;
	
	if (status.inAir)
	{
		status.inAir = false;
		LandOnFloor_ExitBall();
	}
}

void PLAYER::MoveOnPlatform(OBJECT *platform, int16_t height, int16_t lastXPos)
{
	int top;
	if (status.reverseGravity)
		top = platform->y.pos + height;
	else
		top = platform->y.pos - height;
	
	//Check if we're in an intangible state
	if (objectControl.disableObjectInteract || routine == PLAYERROUTINE_DEATH || debug != 0)
		return;
	
	//Move with the platform
	x.pos += (platform->x.pos - lastXPos);
	
	if (status.reverseGravity)
		y.pos = top + yRadius;
	else
		y.pos = top - yRadius;
}

//Item functions
void PLAYER::GiveSpeedShoes()
{
	//Give speed shoes
	item.hasSpeedShoes = true;
	speedShoesTime = 150;
	
	if (!super)
		SetSpeedFromDefinition(status.underwater ? underwaterSpeedShoesSD : speedShoesSD);
	else
		SetSpeedFromDefinition(status.underwater ? underwaterSuperSpeedShoesSD : superSpeedShoesSD);
	
	//Play speed shoes music (only if lead)
	if (follow == nullptr)
		gLevel->ChangeSecondaryMusic(gLevel->speedShoesMusic);
}

void PLAYER::GiveShield(SOUNDID sound, SHIELD type)
{
	//Give us the shield and play sound
	shield = type;
	item.shieldReflect =	type == SHIELD_NULL ? false : (type != SHIELD_BLUE);
	item.immuneFire =		type == SHIELD_NULL ? false : (type == SHIELD_FIRE);
	item.immuneElectric =	type == SHIELD_NULL ? false : (type == SHIELD_ELECTRIC);
	item.immuneWater =		type == SHIELD_NULL ? false : (type == SHIELD_BUBBLE);
	
	if (sound != SOUNDID_NULL)
		PlaySound(sound);
}
