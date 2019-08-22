#pragma once
#include <stdint.h>
#include "SDL_endian.h"
#include "Render.h"
#include "Mappings.h"
#include "Input.h"
#include "LevelCollision.h"

#define PLAYER_RECORD_LENGTH 0x40

enum PLAYERROUTINE
{
	PLAYERROUTINE_CONTROL,
	PLAYERROUTINE_HURT,
	PLAYERROUTINE_DEATH,
	PLAYERROUTINE_RESET_LEVEL,
};

enum SHIELD
{
	SHIELD_BLUE,
	SHIELD_FIRE,
	SHIELD_ELECTRIC,
	SHIELD_BUBBLE,
};

enum CHARACTERTYPE
{
	CHARACTERTYPE_SONIC,
	CHARACTERTYPE_TAILS,
	CHARACTERTYPE_KNUCKLES,
};

enum PLAYERANIMATION
{
	PLAYERANIMATION_WALK,
	PLAYERANIMATION_RUN,
	PLAYERANIMATION_ROLL,
	PLAYERANIMATION_ROLL2,
	PLAYERANIMATION_PUSH,
	PLAYERANIMATION_IDLE,
	PLAYERANIMATION_BALANCE,
	PLAYERANIMATION_LOOKUP,
	PLAYERANIMATION_DUCK,
	PLAYERANIMATION_SPINDASH,
	PLAYERANIMATION_WARP1,
	PLAYERANIMATION_WARP2,
	PLAYERANIMATION_WARP3,
	PLAYERANIMATION_WARP4,
	PLAYERANIMATION_SKID,
	PLAYERANIMATION_FLOAT1,
	PLAYERANIMATION_FLOAT2,
	PLAYERANIMATION_SPRING,
	PLAYERANIMATION_HANG,
	PLAYERANIMATION_LEAP1,
	PLAYERANIMATION_LEAP2,
	PLAYERANIMATION_SURF,
	PLAYERANIMATION_GETAIR,
	PLAYERANIMATION_BURNT,
	PLAYERANIMATION_DROWN,
	PLAYERANIMATION_DEATH,
	PLAYERANIMATION_SHRINK,
	PLAYERANIMATION_HURT,
	PLAYERANIMATION_WATERSLIDE,
	PLAYERANIMATION_NULL,
	PLAYERANIMATION_FLOAT3,
	PLAYERANIMATION_FLOAT4,
};

class PLAYER
{
	public:
		//Fail
		const char *fail;
		
		//Rendering stuff
		struct
		{
			bool xFlip : 1;
			bool yFlip : 1;
			bool alignPlane : 1;
			bool alignBackground : 1; //Basically, align to background if above is set
			bool assumePixelHeight : 1;
			bool bit5 : 1;
			bool bit6 : 1;
			bool onScreen : 1;
		} renderFlags;
		
		bool doRender;
		
		//Our texture and mappings
		TEXTURE *texture;
		MAPPINGS *mappings;
		
		#if SDL_BYTEORDER == SDL_BIGENDIAN
			//X-position (big endian)
			union
			{
				struct
				{
					int16_t xPos;
					uint16_t xSub;
				};
				int32_t xPosLong;
			};
			
			//Y-position (big endian)
			union
			{
				struct
				{
					int16_t yPos;
					uint16_t ySub;
				};
				int32_t yPosLong;
			};
		#else
			//X-position (little endian)
			union
			{
				struct
				{
					uint16_t sub;
					int16_t pos;
				} x;
				int32_t xPosLong;
			};
			
			//Y-position (little endian)
			union
			{
				struct
				{
					uint16_t sub;
					int16_t pos;
				} y;
				int32_t yPosLong;
			};
		#endif
		
		//Speeds
		int16_t xVel;		//Global X-velocity
		int16_t yVel;		//Global Y-velocity
		int16_t inertia;	//Horizontal velocity (on ground)
		
		//Collision box size
		uint8_t xRadius;
		uint8_t yRadius;
		
		uint8_t defaultXRadius;
		uint8_t defaultYRadius;
		uint8_t rollXRadius;
		uint8_t rollYRadius;
		
		//Sprite properties
		bool highPriority;		//Drawn above the foreground
		uint8_t priority;		//Priority of sprite when drawing
		uint8_t widthPixels;	//Width of sprite in pixels
		
		//Animation and mapping
		uint16_t mappingFrame;
		
		uint16_t animFrame;
		PLAYERANIMATION anim;
		PLAYERANIMATION prevAnim;
		int16_t animFrameDuration;
		
		//Our status
		struct STATUS
		{
			bool xFlip : 1;				//Set if facing left
			bool inAir : 1;				//In the air
			bool inBall : 1;			//In ball-form
			bool shouldNotFall : 1;		//Typically set while standing on an object
			bool rollJumping : 1;		//If set, we don't have control in mid-air when we jump from a roll
			bool pushing : 1;			//Pushing against a wall
			bool underwater : 1;		//In water
			bool jumping : 1;			//Set if we're jumping
			bool isSliding : 1;			//If set, the player will slide about at their previous speed (Oil Ocean's oil slides)
			bool pinballMode : 1;		//If set, the player is forced to be rolling constantly (S-tubes, Casino Night things)
			bool stickToConvex : 1;		//If set, the collision will never detect us as running off of a ledge, or when we would normally detach from a ramp when going too fast (the wheel glitch from Carnival Night)
			bool reverseGravity : 1;	//If set, gravity is reversed (No, really?)
			bool windTunnel : 1;		//Inside a wind tunnel (Labyrinth Zone, Hydrocity)
		} status;
		
		//Items we have
		struct
		{
			bool hasShield : 1;		//Do we have a shield
			bool isInvincible : 1;	//Do we have invincibility
			bool hasSpeedShoes : 1;	//Do we have speed shoes
			
			//Shield effects
			bool shieldReflect : 1;		//Reflects projectile
			bool immuneFire : 1;		//Immune to fire
			bool immuneElectric : 1;	//Immune to electric
			bool immuneWater : 1;		//Immune to water
		} item;
		
		SHIELD shield;				//Our shield type
		uint8_t jumpAbility;		//Our jump ability (can we use shield ability, insta-shield, dropdash, etc.)
		
		uint16_t invincibilityTime;
		uint16_t speedShoesTime;
		
		uint8_t routine;	//Routine
		
		uint8_t angle;		//Angle
		uint8_t moveLock;	//Player cannot move if this is non-zero (decrements every frame)
		
		//These two are used for collision, usually represent the player's two ground point angles
		uint8_t primaryAngle;
		uint8_t secondaryAngle;
		
		//These are the above, basically, but from the last frame
		uint8_t tilt;		//set to Secondary Angle
		uint8_t nextTilt;	//set to Primary Angle
		
		//Object control
		struct
		{
			bool disableOurMovement : 1;		//Disables our movement functions
			bool disableAnimation : 1;			//Disables animation
			bool disableWallCollision : 1;		//Disables collision with walls
			bool disableObjectInteract : 1;		//Disables generic interaction with objects (we'll still otherwise collide with objects that have separate detection, such as bubbles, springs, and other solid objects, though)
			bool disableObjectInteract2 : 1;	//Disables generic interaction with objects (idk what this does specifically yet)
		} objectControl;
		
		void *interact;	//Object we're touching
		
		//Spindash
		bool spindashing;			//Set if we're spindashing
		uint16_t spindashCounter;	//Our counter for spindashing
		
		//Flipping (hit a spring that causes the player to spin about, or running off of that curved ramp in Angel Island Zone)
		uint8_t flipType;
		uint8_t flipAngle;
		uint8_t flipsRemaining;
		uint8_t flipSpeed;
		
		//Air left
		uint8_t airRemaining;
		
		//Our collision layers
		COLLISIONLAYER topSolidLayer;
		COLLISIONLAYER lrbSolidLayer;
		
		uint16_t restartCountdown;	//Timer for the level restarting after death
		
		//Speeds
		uint16_t top;
		uint16_t acceleration;
		uint16_t deceleration;
		
		//Super flag
		bool super;
		
		//In debug flag
		uint16_t debug;
		int debugAccel;
		uint8_t debugSpeed;
		int debugObject;
		
		//Character type id (ex. Sonic, Tails, Knuckles, or anything else)
		CHARACTERTYPE characterType;
		
		//Player to follow (sidekick)
		void *follow; //We can't use the PLAYER type here since we're still defining it
		int controller;
		
		//Controls
		CONTROLMASK controlHeld;
		CONTROLMASK controlPress;
		bool controlLock;
		
		//Camera scrolling
		unsigned int scrollDelay;
		bool cameraLock;
		
		//Position and status records
		struct
		{
			int16_t x, y;
		} posRecord[PLAYER_RECORD_LENGTH];

		struct STAT_RECORD
		{
			CONTROLMASK controlHeld;
			CONTROLMASK controlPress;
			STATUS status;
		} statRecord[PLAYER_RECORD_LENGTH];
		
		int recordPos;
		
	public:
		PLAYER(const char *specPath, PLAYER *myFollow, int myController);
		~PLAYER();
		
		uint8_t AngleIn(uint8_t angleSide, int16_t *distance, int16_t *distance2);
		void CheckFloor(COLLISIONLAYER layer, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		void CheckCeiling(COLLISIONLAYER layer, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		
		uint8_t AngleSide(uint8_t angleSide);
		int16_t CheckFloorDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle);
		int16_t CheckCeilingDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle);
		int16_t CheckLeftWallDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle);
		int16_t CheckRightWallDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle);
		
		int16_t CheckLeftCeilingDist(COLLISIONLAYER layer, uint8_t *outAngle);
		int16_t CheckRightCeilingDist(COLLISIONLAYER layer, uint8_t *outAngle);

		int16_t CalcRoomOverHead(uint8_t upAngle);
		int16_t CalcRoomInFront(uint8_t moveAngle);
		
		int16_t Angle(int16_t distance, int16_t distance2);
		void AnglePos();
		void CheckWallsOnGround();
		
		void DoLevelCollision();
		
		void ResetOnFloor();
		void ResetOnFloor2();
		void ResetOnFloor3();
		
		void RecordPos();
		
		bool Spindash();
		
		void JumpAbilities();
		void JumpHeight();
		void ChgJumpDir();
		void JumpAngle();
		bool Jump();
		
		void SlopeResist();
		void RollRepel();
		void SlopeRepel();
		
		void MoveLeft();
		void MoveRight();
		void Move();
		
		void ChkRoll();
		void Roll();
		void RollLeft();
		void RollRight();
		void RollSpeed();
		
		void DeadCheckRespawn();
		
		void KillCharacter();
		
		void LevelBoundSide(uint16_t bound);
		void LevelBound();
		
		void FrameCommand(const uint8_t* animation);
		void AdvanceFrame(const uint8_t* animation);
		void FlipAngle();
		void Animate();
		
		void Update();
		void Draw();
		
		void RestoreStateDebug();
		void DebugControl();
		void DebugMode();
		
		void RideObject(void *ride, bool *standingBit);
};
