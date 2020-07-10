#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

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
															if (node->node_entry->deleteFlag)	\
															{	\
																delete node->node_entry;	\
																linkedList.erase_node(node);	\
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

enum ANIMATION_COMMAND
{
	ANICOMMAND_RESTART = 0xFF,
	ANICOMMAND_GO_BACK_FRAMES = 0xFE,
	ANICOMMAND_SET_ANIMATION = 0xFD,
	ANICOMMAND_ADVANCE_ROUTINE = 0xFC,
	ANICOMMAND_CLEAR_ROUTINESECONDARY = 0xFB,
	ANICOMMAND_ADVANCE_ROUTINESECONDARY = 0xFA,
};

//Object structures
struct OBJECT_RENDERFLAGS
{
	bool xFlip = false;
	bool yFlip = false;
	bool alignPlane = false;
	bool isOnscreen = false;
	bool staticMapping = false;
};

struct OBJECT_STATUS
{
	bool xFlip = false;				//Set if facing left
	bool yFlip = false;				//In the air
	bool releaseDestroyed = false;	//Don't reload if destroyed (enemies)
	bool noBalance = false;			//Set to make sure players don't balance on us
	bool objectSpecific = false;	//Used for anything any specific object wants
};

struct OBJECT_SOLIDTOUCH
{
	bool side[OBJECT_PLAYER_REFERENCES] = {0};
	bool bottom[OBJECT_PLAYER_REFERENCES] = {0};
	bool top[OBJECT_PLAYER_REFERENCES] = {0};
};

struct OBJECT_SMASHMAP
{
	RECT rect;
	int16_t xVel, yVel;
};

struct OBJECT_FRAGMENTMAP
{
	RECT rect;
	uint8_t delay;
};

struct OBJECT_MAPPING
{
	RECT rect;
	POINT origin;
	MAPPINGS *mappings = nullptr;
};

//Object drawing instance class
struct OBJECT_DRAWINSTANCE
{
	OBJECT_RENDERFLAGS renderFlags;
	TEXTURE *texture;
	OBJECT_MAPPING mapping;
	bool highPriority;
	uint8_t priority;
	uint16_t mappingFrame;
	int16_t xPos, yPos;
};

//Object class
class OBJECT
{
	public:
		//Failure
		const char *fail = nullptr;
		
		//Object sub-type
		unsigned int subtype = 0;
		
		//Rendering stuff
		OBJECT_RENDERFLAGS renderFlags;
		LINKEDLIST<OBJECT_DRAWINSTANCE*> drawInstances;
		
		//Our texture and mappings
		TEXTURE *texture = nullptr;
		OBJECT_MAPPING mapping;
		
		//Position
		FPDEF(x, int16_t, pos, uint8_t, sub, int32_t)
		FPDEF(y, int16_t, pos, uint8_t, sub, int32_t)
		
		//Speeds
		int16_t xVel = 0;		//Global X-velocity
		int16_t yVel = 0;		//Global Y-velocity
		int16_t inertia = 0;	//Generic horizontal velocity
		
		//Collision properties
		int16_t xRadius = 0;
		int16_t yRadius = 0;
		
		COLLISIONTYPE collisionType = COLLISIONTYPE_NULL;
		int16_t touchWidth = 0;
		int16_t touchHeight = 0;
		
		struct
		{
			bool reflect = false;	//Projectile that gets reflected
			bool flame = false;		//Flame
			bool lightning = false;	//Lightning
			bool aqua = false;		//Aqua
		} hurtType;
		
		//Sprite properties
		bool highPriority = false;					//Drawn above the foreground
		unsigned int priority = 0;					//Priority of sprite when drawing
		int16_t widthPixels = 0, heightPixels = 0;	//Width and height of sprite in pixels (used for on screen checking and balancing)
		
		//Animation and mapping
		unsigned int mappingFrame = 0;
		
		unsigned int animFrame = 0;
		unsigned int anim = 0;
		unsigned int prevAnim = 0;
		signed int animFrameDuration = 0;
		
		//Our status
		OBJECT_STATUS status;
		
		//Player contact status
		struct
		{
			bool standing = false;
			bool pushing = false;
			bool objectSpecific = false;
		} playerContact[OBJECT_PLAYER_REFERENCES];
		
		//Routine
		uint8_t routine = 0;			//Routine
		uint8_t routineSecondary = 0;	//Routine Secondary
		
		uint8_t angle = 0;	//Angle
		union //Parent
		{
			void *parent = nullptr;
			OBJECT *parentObject;
			PLAYER *parentPlayer;
		};
		
		//Children linked list
		LINKEDLIST<OBJECT*> children;
		
		//Scratch memory
		void *scratch = nullptr; //No specific type - whatever an object specifies
		
		//Our object-specific function
		OBJECTFUNCTION function = nullptr;
		OBJECTFUNCTION prevFunction = nullptr;
		
		//Delete flag
		bool deleteFlag = false;
		
	public:
		//Constructor and destructor
		OBJECT(OBJECTFUNCTION object);
		~OBJECT();
		
		//Scratch allocation function
		template <typename T> inline T *Scratch()
		{
			//Allocate scratch if null, then return it or the already allocated scratch
			if (scratch == nullptr)
			{
				scratch = malloc(sizeof(T)); //Allocate memory (have to use malloc because destructor doesn't know scratch type)
				*((T*)scratch) = {}; //Explicitly initialize because we used malloc
			}
			return (T*)scratch;
		}
		
		//Generic object functions
		void Move();
		void MoveAndFall();
		
		void Animate(const uint8_t **animationList);
		void Animate_S1(const uint8_t **animationList);
		
		int16_t CheckCollisionDown_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		int16_t CheckCollisionUp_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		int16_t CheckCollisionLeft_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		int16_t CheckCollisionRight_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		
		void DrawInstance(OBJECT_RENDERFLAGS iRenderFlags, TEXTURE *iTexture, OBJECT_MAPPING iMapping, bool iHighPriority, uint8_t iPriority, uint16_t iMappingFrame, int16_t iXPos, int16_t iYPos);
		
		void UnloadOffscreen(int16_t xPos);
		
		void Smash(size_t num, const OBJECT_SMASHMAP *smashmap, OBJECTFUNCTION fragmentFunction);
		void Fragment(size_t num, const OBJECT_FRAGMENTMAP *fragmap, OBJECTFUNCTION fragmentFunction);
		
		//Object interaction functions
		bool Hurt(PLAYER *player);
		void ClearSolidContact();
		
		//Player solid contact functions
		void AttachPlayer(PLAYER *player, size_t i);
		void MovePlayer(PLAYER *player, int16_t width, int16_t height, int16_t lastXPos, const int8_t *slope, bool doubleSlope);
		
		void SolidObjectTop(int16_t width, int16_t height, int16_t lastXPos, bool setAirOnExit, const int8_t *slope);
		bool LandOnTopSolid(PLAYER *player, size_t i, int16_t width1, int16_t width2, int16_t height, int16_t lastXPos, const int8_t *slope);
		void ReleasePlayer(PLAYER *player, size_t i, bool setAirOnExit);
		
		OBJECT_SOLIDTOUCH SolidObjectFull(int16_t width, int16_t height_air, int16_t height_standing, int16_t lastXPos, bool setAirOnExit, const int8_t *slope, bool doubleSlope);
		void SolidObjectFull_Cont(OBJECT_SOLIDTOUCH *solidTouch, PLAYER *player, size_t i, int16_t width, int16_t height, int16_t lastXPos, const int8_t *slope, bool doubleSlope);
		void SolidObjectFull_ClearPush(PLAYER *player, size_t i);
		
		void CollideStandingPlayersWithLevel();
		
		//Main update and draw functions
		bool Update();
		void Draw();
		void RenderDrawInstance(OBJECT_DRAWINSTANCE *drawInstance);
};
