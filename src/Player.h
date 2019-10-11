#pragma once
#include <stdint.h>

#include "Render.h"
#include "Mappings.h"
#include "Input.h"
#include "Audio.h"
#include "LevelCollision.h"
#include "CommonMacros.h"

//Declare the object class
class OBJECT;

//Constants
#define PLAYER_RECORD_LENGTH 0x40

//Routines
enum PLAYERROUTINE
{
	PLAYERROUTINE_CONTROL,
	PLAYERROUTINE_HURT,
	PLAYERROUTINE_DEATH,
	PLAYERROUTINE_RESET_LEVEL,
};

enum CPUROUTINE
{
	CPUROUTINE_INIT,
	CPUROUTINE_SPAWNING,
	CPUROUTINE_FLYING,
	CPUROUTINE_NORMAL,
	CPUROUTINE_PANIC,
};

//Other enumerations
enum SHIELD
{
	SHIELD_NULL,
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
	PLAYERANIMATION_BALANCE1,
	PLAYERANIMATION_LOOKUP,
	PLAYERANIMATION_DUCK,
	PLAYERANIMATION_SPINDASH,
	PLAYERANIMATION_BLINK,
	PLAYERANIMATION_GETUP,
	PLAYERANIMATION_BALANCE2,
	PLAYERANIMATION_SKID,
	PLAYERANIMATION_FLOAT1,
	PLAYERANIMATION_FLOAT2,
	PLAYERANIMATION_SPRING,
	PLAYERANIMATION_HANG1,
	PLAYERANIMATION_DASH2,
	PLAYERANIMATION_DASH3,
	PLAYERANIMATION_HANG2,
	PLAYERANIMATION_BUBBLE,
	PLAYERANIMATION_BURNT,
	PLAYERANIMATION_DROWN,
	PLAYERANIMATION_DEATH,
	PLAYERANIMATION_HURT,
	PLAYERANIMATION_SLIDE,
	PLAYERANIMATION_NULL,
	PLAYERANIMATION_BALANCE3,
	PLAYERANIMATION_BALANCE4,
	PLAYERANIMATION_LYING,
	PLAYERANIMATION_LIEDOWN,
	PLAYERANIMATION_TRANSFORM,
};

//Super palette state
enum PALETTESTATE
{
	PALETTESTATE_IDLE =			 0,
	PALETTESTATE_FADING_IN =	 1,
	PALETTESTATE_FADING_OUT =	 2,
	PALETTESTATE_DONE =			-1,
};

//Speed definition
struct SPEEDDEFINITION
{
	uint16_t top, acceleration, deceleration, rollDeceleration, jumpForce, jumpRelease;
};

//Player class
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
			bool onScreen : 1;
		} renderFlags;
		
		bool isDrawing;
		
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
			bool shouldNotFall : 1;		//Typically set while standing on an object (the "Slope Glitch")
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
		
		uint16_t invulnerabilityTime;
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
		} objectControl;
		
		OBJECT *interact;	//Object we're touching
		
		//Chain point counter
		uint16_t chainPointCounter;
		
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
		bool inGameover;			//If got a game-over
		
		//Speeds
		uint16_t top;
		uint16_t acceleration;
		uint16_t deceleration;
		uint16_t rollDeceleration;
		uint16_t jumpForce, jumpRelease;
		
		SPEEDDEFINITION normalSD;
		SPEEDDEFINITION speedShoesSD;
		SPEEDDEFINITION superSD;
		SPEEDDEFINITION superSpeedShoesSD;
		SPEEDDEFINITION underwaterNormalSD;
		SPEEDDEFINITION underwaterSpeedShoesSD;
		SPEEDDEFINITION underwaterSuperSD;
		SPEEDDEFINITION underwaterSuperSpeedShoesSD;
		
		//Super state
		bool super, hyper;
		int16_t superTimer;
		
		PALETTESTATE paletteState;
		int16_t paletteTimer;
		uint16_t paletteFrame;
		
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
		
		//CPU
		CPUROUTINE cpuRoutine;
		int cpuOverride;
		unsigned int cpuTimer;
		bool cpuJumping;
		
		//Controls
		CONTROLMASK controlHeld;
		CONTROLMASK controlPress;
		bool controlLock;
		
		//Camera scrolling
		unsigned int scrollDelay;
		bool cameraLock;
		
		//Position and status records
		struct POS_RECORD
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
		
		//Objects that belong to us
		OBJECT *spindashDust;
		OBJECT *skidDust;
		OBJECT *shieldObject;
		
		//For linked list
		PLAYER **list;
		PLAYER *next;
		
	public:
		PLAYER(PLAYER **linkedList, const char *specPath, PLAYER *myFollow, int myController);
		~PLAYER();
		
		void SetSpeedFromDefinition(SPEEDDEFINITION definition);
		
		uint8_t AngleIn(uint8_t angleSide, int16_t *distance, int16_t *distance2);
		void CheckFloor(COLLISIONLAYER layer, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		void CheckCeiling(COLLISIONLAYER layer, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		
		int16_t ChkFloorEdge(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		
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
		void ResetRecords(int16_t xPos, int16_t yPos);
		
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
		void HurtStop();
		
		bool KillCharacter(SOUNDID soundId);
		bool CheckHurt(void *hit);
		bool HurtCharacter(void *hit);
		
		void LevelBoundSide(int32_t bound);
		void LevelBound();
		
		void SuperPaletteCycle();
		void UpdateSuper();
		
		void FrameCommand(const uint8_t* animation);
		void AdvanceFrame(const uint8_t* animation);
		void FlipAngle();
		void Animate();
		
		void CPUControl();
		
		void ControlRoutine();
		void Update();
		
		void Display();
		
		void Draw();
		void DrawToScreen();
		
		void RestoreStateDebug();
		void DebugControl();
		void DebugMode();
		
		void RingAttractCheck(OBJECT *object);
		
		bool TouchResponseObject(OBJECT *object, int16_t playerLeft, int16_t playerTop, int16_t playerWidth, int16_t playerHeight);
		void TouchResponse();
		
		void AttachToObject(OBJECT *object, bool *standingBit);
		void MoveOnPlatform(OBJECT *platform, int16_t height, int16_t lastXPos);
		
		void GiveSpeedShoes();
		void GiveShield(SOUNDID sound, SHIELD type);
};
