#pragma once
#include <stdint.h>

#include "LinkedList.h"
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
#define CHECK_LINKEDLIST_OBJECTDELETE(linkedList)	for (LL_NODE<OBJECT*> *node = linkedList.head; node != nullptr;)	\
													{	\
														node = linkedList.head;	\
														for (; node != nullptr; node = node->next)	\
														{	\
															if (node->nodeEntry->deleteFlag)	\
															{	\
																delete node->nodeEntry;	\
																linkedList.eraseNode(node);	\
																break;	\
															}	\
														}	\
													}

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
	bool isOnscreen : 1;
};

struct OBJECT_STATUS
{
	//Properties to load from
	bool xFlip : 1;				//Set if facing left
	bool yFlip : 1;				//In the air
	bool releaseDestroyed : 1;	//Don't reload if destroyed (enemies)
	bool noBalance : 1;			//Set to make sure players don't balance on us
	bool objectSpecific : 1;	//Used for anything any specific object wants
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
		OBJECT_DRAWINSTANCE();
		~OBJECT_DRAWINSTANCE();
		void Draw();
};

//Object class
class OBJECT
{
	public:
		//Failure
		const char *fail;
		
		//Object sub-type
		unsigned int subtype;
		
		//Rendering stuff
		OBJECT_RENDERFLAGS renderFlags;
		LINKEDLIST<OBJECT_DRAWINSTANCE*> drawInstances;
		
		//Our texture and mappings
		TEXTURE *texture;
		MAPPINGS *mappings;
		
		//Position
		POSDEF(x)
		POSDEF(y)
		
		//Speeds
		int16_t xVel;		//Global X-velocity
		int16_t yVel;		//Global Y-velocity
		int16_t inertia;	//Generic horizontal velocity
		
		//Collision properties
		int16_t xRadius;
		int16_t yRadius;
		
		COLLISIONTYPE collisionType;
		int16_t touchWidth;
		int16_t touchHeight;
		
		struct
		{
			bool reflect : 1;	//Projectile that gets reflected
			bool fire : 1;		//Is fire
			bool electric : 1;	//Is electric
			bool water : 1;		//Is water
		} hurtType;
		
		//Sprite properties
		bool highPriority;					//Drawn above the foreground
		unsigned int priority;					//Priority of sprite when drawing
		int16_t widthPixels, heightPixels;	//Width and height of sprite in pixels (used for on screen checking and balancing)
		
		//Animation and mapping
		unsigned int mappingFrame;
		
		unsigned int animFrame;
		unsigned int anim;
		unsigned int prevAnim;
		signed int animFrameDuration;
		
		//Our status
		OBJECT_STATUS status;
		
		//Player contact status
		struct
		{
			bool standing : 1;
			bool pushing : 1;
			bool objectSpecific : 1;
		} playerContact[OBJECT_PLAYER_REFERENCES];
		
		//Routine
		uint8_t routine;			//Routine
		uint8_t routineSecondary;	//Routine Secondary
		
		uint8_t angle;	//Angle
		union //Parent
		{
			void *parent;
			OBJECT *parentObject;
			PLAYER *parentPlayer;
		};
		
		//Children linked list
		LINKEDLIST<OBJECT*> children;
		
		//Scratch memory
		 uint8_t  *scratchU8;
		  int8_t  *scratchS8;
		uint16_t *scratchU16;
		 int16_t *scratchS16;
		uint32_t *scratchU32;
		 int32_t *scratchS32;
		
		//Our object-specific function
		OBJECTFUNCTION function;
		OBJECTFUNCTION prevFunction;
		
		//Delete flag
		bool deleteFlag : 1;
		
	public:
		//Constructor and destructor
		OBJECT(OBJECTFUNCTION object);
		~OBJECT();
		
		//Scratch allocation functions
		void  ScratchAllocU8(size_t max);
		void  ScratchAllocS8(size_t max);
		void ScratchAllocU16(size_t max);
		void ScratchAllocS16(size_t max);
		void ScratchAllocU32(size_t max);
		void ScratchAllocS32(size_t max);
		
		//Generic object functions
		void Move();
		void MoveAndFall();
		
		void Animate(const uint8_t **animationList);
		void Animate_S1(const uint8_t **animationList);
		
		int16_t CheckFloorEdge(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		
		void DrawInstance(OBJECT_RENDERFLAGS iRenderFlags, TEXTURE *iTexture, MAPPINGS *iMappings, bool iHighPriority, uint8_t iPriority, uint16_t iMappingFrame, int16_t iXPos, int16_t iYPos);
		
		void UnloadOffscreen(int16_t xPos);
		
		//Object interaction functions
		bool Hurt(PLAYER *player);
		void ClearSolidContact();
		
		//Player solid contact functions
		void PlatformObject(int16_t width, int16_t height, int16_t lastXPos, bool setAirOnExit, const int8_t *slope);
		bool LandOnPlatform(PLAYER *player, size_t i, int16_t width1, int16_t width2, int16_t height, int16_t lastXPos, const int8_t *slope);
		void ExitPlatform(PLAYER *player, size_t i, bool setAirOnExit);
		
		OBJECT_SOLIDTOUCH SolidObject(int16_t width, int16_t height_air, int16_t height_standing, int16_t lastXPos, bool setAirOnExit, const int8_t *slope, bool doubleSlope);
		void SolidObjectCont(OBJECT_SOLIDTOUCH *solidTouch, PLAYER *player, size_t i, int16_t width, int16_t height, int16_t lastXPos, const int8_t *slope, bool doubleSlope);
		void SolidObjectClearPush(PLAYER *player, size_t i);
		
		//Main update and draw functions
		bool Update();
		void Draw();
};
