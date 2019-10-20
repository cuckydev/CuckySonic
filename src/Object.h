#pragma once
#include <stdint.h>
#include <deque>

#include "Render.h"
#include "Mappings.h"
#include "LevelCollision.h"
#include "CommonMacros.h"

//Declare the object and player classes
class OBJECT;
class PLAYER;

//Object function type
typedef void (*OBJECTFUNCTION)(OBJECT*);

//Constants
#define OBJECT_PLAYER_REFERENCES 0x100

//Common macros
#define CHECK_LINKEDLIST_OBJECTDELETE(linkedList)	for (signed int i = (signed int)linkedList.size() - 1; i >= 0; i--)	\
													{	\
														if (linkedList[i]->deleteFlag)	\
														{	\
															delete linkedList[i];	\
															linkedList.erase(linkedList.begin() + i);	\
														}	\
													}	\
													linkedList.shrink_to_fit();

//Enumerations
enum COLLISIONTYPE
{
	COLLISIONTYPE_NULL,
	COLLISIONTYPE_ENEMY,
	COLLISIONTYPE_BOSS,
	COLLISIONTYPE_OTHER,
	COLLISIONTYPE_HURT,
	COLLISIONTYPE_SPECIAL,
	COLLISIONTYPE_MONITOR,
};

//Object structures
struct OBJECT_RENDERFLAGS
{
	bool xFlip : 1;
	bool yFlip : 1;
	bool alignPlane : 1;
};

struct OBJECT_SOLIDTOUCH
{
	bool side[OBJECT_PLAYER_REFERENCES];
	bool bottom[OBJECT_PLAYER_REFERENCES];
	bool top[OBJECT_PLAYER_REFERENCES];
};

//Object drawing instance class
class OBJECT_DRAWINSTANCE
{
	public:
		//Render properties
		OBJECT_RENDERFLAGS renderFlags;
		TEXTURE *texture;
		MAPPINGS *mappings;
		bool highPriority;
		uint8_t priority;
		uint16_t mappingFrame;
		int16_t xPos, yPos;
		
	public:
		OBJECT_DRAWINSTANCE(std::deque<OBJECT_DRAWINSTANCE*> *linkedList);
		~OBJECT_DRAWINSTANCE();
		void Draw();
};

//Object class
class OBJECT
{
	public:
		//Fail
		const char *fail;
		
		//Object sub-type
		int subtype;
		
		//Rendering stuff
		OBJECT_RENDERFLAGS renderFlags;
		
		//Drawing instances
		std::deque<OBJECT_DRAWINSTANCE*> drawInstances;
		
		//Our texture and mappings
		TEXTURE *texture;
		MAPPINGS *mappings;
		
		//Position
		POSDEF(x)
		POSDEF(y)
		
		//Speeds
		int16_t xVel;		//Global X-velocity
		int16_t yVel;		//Global Y-velocity
		int16_t inertia;	//Horizontal velocity (on ground)
		
		//Collision box size
		uint8_t xRadius;
		uint8_t yRadius;
		
		COLLISIONTYPE collisionType;
		uint8_t touchWidth;
		uint8_t touchHeight;
		
		struct
		{
			bool reflect : 1;	//Projectile that gets reflected
			bool fire : 1;		//Is fire
			bool electric : 1;	//Is electric
			bool water : 1;		//Is water
		} hurtType;
		
		//Sprite properties
		bool highPriority;					//Drawn above the foreground
		uint8_t priority;					//Priority of sprite when drawing
		uint8_t widthPixels, heightPixels;	//Width and height of sprite in pixels (used for on screen checking and balancing)
		
		//Animation and mapping
		uint16_t mappingFrame;
		
		uint16_t animFrame;
		int anim;
		int prevAnim;
		int16_t animFrameDuration;
		
		//Our status
		struct
		{
			bool xFlip : 1;				//Set if facing left
			bool yFlip : 1;				//In the air
			bool noBalance : 1;			//Set to make sure players don't balance on us
		} status;
		
		//Player contact status
		struct
		{
			bool standing;
			bool pushing;
			bool extraBit;
		} playerContact[OBJECT_PLAYER_REFERENCES];
		
		uint8_t routine;			//Routine
		uint8_t routineSecondary;	//Routine Secondary
		uint8_t respawnIndex;		//Index of this object in the respawn table, this is to not respawn unloaded enemies / monitors
		
		uint8_t angle;	//Angle
		void *parent;	//Our parent object or player
		
		//Children linked list
		std::deque<OBJECT*> children;
		
		//Scratch memory
		 uint8_t  *scratchU8;
		  int8_t  *scratchS8;
		uint16_t *scratchU16;
		 int16_t *scratchS16;
		uint32_t *scratchU32;
		 int32_t *scratchS32;
		
		//Object delete flag
		bool deleteFlag;
		
		//Our object-specific function
		OBJECTFUNCTION function;
		OBJECTFUNCTION prevFunction;
		
	public:
		OBJECT(std::deque<OBJECT*> *linkedList, OBJECTFUNCTION object);
		~OBJECT();
		
		void  ScratchAllocU8(int max);
		void  ScratchAllocS8(int max);
		void ScratchAllocU16(int max);
		void ScratchAllocS16(int max);
		void ScratchAllocU32(int max);
		void ScratchAllocS32(int max);
		
		void Move();
		void MoveAndFall();
		
		bool Hurt(PLAYER *player);
		
		void Animate(const uint8_t **animationList);
		int16_t CheckFloorEdge(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		
		void PlatformObject(int16_t width, int16_t height, int16_t lastXPos);
		bool LandOnPlatform(PLAYER *player, int i, int16_t width1, int16_t width2, int16_t height, int16_t lastXPos);
		void ExitPlatform(PLAYER *player, int i);
		
		OBJECT_SOLIDTOUCH SolidObject(int16_t width, int16_t height_air, int16_t height_standing, int16_t lastXPos);
		void SolidObjectCont(OBJECT_SOLIDTOUCH *solidTouch, PLAYER *player, int i, int16_t width, int16_t height, int16_t lastXPos);
		void SolidObjectClearPush(PLAYER *player, int i);
		
		void ClearSolidContact();
		
		bool Update();
		void DrawInstance(OBJECT_RENDERFLAGS iRenderFlags, TEXTURE *iTexture, MAPPINGS *iMappings, bool iHighPriority, uint8_t iPriority, uint16_t iMappingFrame, int16_t iXPos, int16_t iYPos);
		void Draw();
};
