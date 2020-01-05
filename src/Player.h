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
#define PLAYER_RECORD_LENGTH	0x40
#define INVINCIBILITYSTARS		4

#define NO_FLOOR_ANGLE	3	//Floor angle values' "no floor" flag (must be an odd number)

#define PEELOUT_CHARGE	30
#define SPINDASH_CHARGE	45
#define DROPDASH_CHARGE	20

//Routines
enum PLAYER_ROUTINE
{
	PLAYERROUTINE_CONTROL,
	PLAYERROUTINE_HURT,
	PLAYERROUTINE_DEATH,
	PLAYERROUTINE_RESET_LEVEL,
};

enum PLAYERCPU_ROUTINE
{
	CPUROUTINE_INIT,
	CPUROUTINE_SPAWNING,
	CPUROUTINE_FLYING,
	CPUROUTINE_NORMAL,
	CPUROUTINE_PANIC,
};

//Other enumerations
enum BARRIER
{
	BARRIER_NULL,
	BARRIER_BLUE,
	BARRIER_FLAME,
	BARRIER_LIGHTNING,
	BARRIER_AQUA,
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
	PLAYERANIMATION_DASH,
	PLAYERANIMATION_ROLL,
	PLAYERANIMATION_ROLL2,
	PLAYERANIMATION_DROPDASH,
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
	PALETTESTATE_REGULAR =		 0,
	PALETTESTATE_FADING_IN =	 1,
	PALETTESTATE_FADING_OUT =	 2,
	PALETTESTATE_SUPER =		-1,
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
		const char *fail = nullptr;
		
		//Rendering stuff
		struct
		{
			bool xFlip = false;
			bool yFlip = false;
			bool alignPlane = false;
			bool isOnscreen = false;
		} renderFlags;
		
		bool isDrawing = false;
		
		//Our texture and mappings
		TEXTURE *texture = nullptr;
		MAPPINGS *mappings = nullptr;
		
		//Position
		FPDEF(x, int16_t, pos, uint8_t, sub, int32_t)
		FPDEF(y, int16_t, pos, uint8_t, sub, int32_t)
		
		//Current routine
		PLAYER_ROUTINE routine = PLAYERROUTINE_CONTROL;
		
		//Death state
		unsigned int restartCountdown = 0;	//Timer for the level restarting after death
		bool inGameover = false;			//If got a game-over
		
		//Collision box size
		uint8_t xRadius;
		uint8_t yRadius;
		
		uint8_t defaultXRadius;
		uint8_t defaultYRadius;
		uint8_t rollXRadius;
		uint8_t rollYRadius;
		
		//Sprite properties
		bool highPriority = false;						//Drawn above the foreground
		uint8_t priority = 2;							//Priority of sprite when drawing
		uint8_t widthPixels = 24, heightPixels = 24;	//Width and height of sprite in pixels
		
		//Animation and mapping
		uint16_t mappingFrame = 0;
		
		uint16_t animFrame = 0;
		PLAYERANIMATION anim = (PLAYERANIMATION)0;
		PLAYERANIMATION prevAnim = (PLAYERANIMATION)0;
		int16_t animFrameDuration = 0;
		
		//Our status
		struct PLAYER_STATUS
		{
			bool xFlip = false;				//Set if facing left
			bool inAir = false;				//In the air
			bool inBall = false;			//In ball-form
			bool shouldNotFall = false;		//Typically set while standing on an object (the "Slope Glitch")
			bool rollJumping = false;		//If set, we don't have control in mid-air when we jump from a roll
			bool pushing = false;			//Pushing against a wall
			bool underwater = false;		//In water
			bool jumping = false;			//Set if we're jumping
			bool isSliding = false;			//If set, the player will slide about at their previous speed (Oil Ocean's oil slides)
			bool stickToConvex = false;		//If set, the collision will never detect us as running off of a ledge, or when we would normally detach from a ramp when going too fast (the wheel glitch from Carnival Night)
			bool reverseGravity = false;	//If set, gravity is reversed (No, really?)
			bool windTunnel = false;		//Inside a wind tunnel (Labyrinth Zone, Hydrocity)
		} status;
		
		//Movement speeds and state
		int16_t xVel = 0;		//Global X-velocity
		int16_t yVel = 0;		//Global Y-velocity
		int16_t inertia = 0;	//Horizontal velocity (on ground)
		
		uint8_t moveLock = 0;	//Player cannot move if this is non-zero (decrements every frame)
		
		//Current item status
		struct
		{
			//Status
			bool isInvincible = false;	//Do we have invincibility
			bool hasSpeedShoes = false;	//Do we have speed shoes
			
			//Barrier effects
			bool barrierReflect = false;	//Reflects projectiles
			bool immuneFlame = false;		//Flame immunity
			bool immuneLightning = false;	//Thunder immunity
			bool immuneAqua = false;		//Water immunity
		} item;
		
		BARRIER barrier = BARRIER_NULL;	//Our current barrier
		uint8_t jumpAbility = 0;		//Our current jump ability state (0 = can use, >= 1 = used)
		uint8_t abilityProperty = 0;	//Tails's flying time, Knuckles' gliding speed(?)
		
		uint16_t invulnerabilityTime = 0;
		uint16_t invincibilityTime = 0;
		uint16_t speedShoesTime = 0;
		
		//Current angles
		uint8_t angle = 0; //Current movement / visual angle
		
		uint8_t floorAngle1 = 0;
		uint8_t floorAngle2 = 0;
		uint8_t lastFloorAngle1 = 0;
		uint8_t lastFloorAngle2 = 0;
		
		//Object control
		struct
		{
			bool disableOurMovement = false;		//Disables our movement functions
			bool disableAnimation = false;			//Disables animation
			bool disableWallCollision = false;		//Disables collision with walls
			bool disableObjectInteract = false;		//Disables generic interaction with objects (we'll still otherwise collide with objects that have separate collision, such as bubbles)
		} objectControl;
		
		OBJECT *interact = nullptr;	//Object we're touching
		
		//Chain point counter
		uint16_t chainPointCounter = 0;
		
		//Spindash state and also force roll
		union
		{
			struct
			{
				bool spindashing : 1;	//Set if we're spindashing
				bool forceRoll : 1;		//Set if we're forced to roll (S-tubes)
			} spindashForceRoll;
			bool forceRollOrSpindash = false;
		};
		
		uint16_t spindashCounter = 0;	//Our counter for spindashing
		
		//CD Spindash/Peelout
		uint8_t cdSPTimer = 0;		//Actual timer for if we're fully charged or not
		uint8_t cdChargeDelay = 0;	//Makes sure we can't spindash / peelout if we double tap up or down
		
		//Flipping (hit a spring that causes the player to spin about, or running off of that curved ramp in Angel Island Zone)
		uint8_t flipType = 0;
		uint8_t flipAngle = 0;
		uint8_t flipsRemaining = 0;
		uint8_t flipSpeed = 0;
		
		//Air remaining in seconds before drowning
		uint8_t airRemaining = 0;
		
		//Our collision layers
		COLLISIONLAYER topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
		COLLISIONLAYER lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
		
		//Speeds
		uint16_t top = 0;
		uint16_t acceleration = 0;
		uint16_t deceleration = 0;
		uint16_t rollDeceleration = 0;
		uint16_t jumpForce = 0, jumpRelease = 0;
		
		SPEEDDEFINITION normalSD;
		SPEEDDEFINITION speedShoesSD;
		SPEEDDEFINITION superSD;
		SPEEDDEFINITION superSpeedShoesSD;
		SPEEDDEFINITION underwaterNormalSD;
		SPEEDDEFINITION underwaterSpeedShoesSD;
		SPEEDDEFINITION underwaterSuperSD;
		SPEEDDEFINITION underwaterSuperSpeedShoesSD;
		
		//Super state
		bool super = false, hyper = false;
		int16_t superTimer = 0;
		
		//Palette state
		PALETTESTATE paletteState = PALETTESTATE_REGULAR;
		int16_t paletteTimer = 0;
		uint16_t paletteFrame = 0;
		
		//Debug state
		uint16_t debug = 0;
		int debugAccel = 0;
		uint8_t debugSpeed = 0;
		int debugObject = 0;
		
		//Character type id (ex. Sonic, Tails, Knuckles, or anyone else)
		CHARACTERTYPE characterType;
		
		//Camera scrolling
		unsigned int scrollDelay = 0;
		bool cameraLock = false;
		
		//Records
		struct PLAYER_RECORD
		{
			int16_t x = 0, y = 0;
			CONTROLMASK controlHeld;
			CONTROLMASK controlPress;
			PLAYER_STATUS status;
		} record[PLAYER_RECORD_LENGTH];
		size_t recordPos = 0;
		
		//Objects that belong to us
		OBJECT *spindashDust = nullptr;
		OBJECT *skidDust = nullptr;
		OBJECT *barrierObject = nullptr;
		OBJECT *invincibilityStarObject[INVINCIBILITYSTARS] = {nullptr};
		
		//CPU state
		PLAYERCPU_ROUTINE cpuRoutine = CPUROUTINE_INIT;
		int cpuOverride = 0;
		unsigned int cpuTimer = 0;
		bool cpuJumping = false;
		
		//Input specification (controller and player to follow)
		size_t controller;
		PLAYER *follow;
		
		//Current input state
		CONTROLMASK controlHeld;
		CONTROLMASK controlPress;
		bool controlLock = false;
		
	public:
		PLAYER(std::string specPath, PLAYER *myFollow, size_t myController);
		~PLAYER();
		
		void SetSpeedFromDefinition(SPEEDDEFINITION definition);
		
		uint8_t GetCloserFloor_General(uint8_t angleSide, int16_t *distance, int16_t *distance2);
		void CheckCollisionDown_2Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		void CheckCollisionUp_2Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		void CheckCollisionLeft_2Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		void CheckCollisionRight_2Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, int16_t *distance, int16_t *distance2, uint8_t *outAngle);
		
		int16_t CheckCollisionDown_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		int16_t CheckCollisionUp_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		int16_t CheckCollisionLeft_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);
		int16_t CheckCollisionRight_1Point(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle);

		int16_t GetDistancePerpendicular(uint8_t inAngle);
		int16_t GetWallDistance(uint8_t inAngle);
		
		int16_t GetCloserFloor_Ground(int16_t distance, int16_t distance2);
		void GroundFloorCollision();
		void GroundWallCollision();
		
		void AirCollision();
		
		void LandOnFloor();
		void LandOnFloor_ExitBall();
		void LandOnFloor_SetState();
		
		void RecordPos();
		void ResetRecords(int16_t xPos, int16_t yPos);
		
		bool Spindash();
		bool CDPeeloutSpindash();
		void CheckDropdashRelease();
		
		void JumpAbilities();
		void JumpHeight();
		void AirMovement();
		void UpdateAngleInAir();
		bool Jump();
		
		void RegularSlopeGravity();
		void RollingSlopeGravity();
		void WallDetach();
		
		void GroundMoveLeft();
		void GroundMoveRight();
		void GroundMovement();
		
		void PerformRoll();
		void Roll();
		void RollMoveLeft();
		void RollMoveRight();
		void RollMovement();
		
		void DeadCheckOffscreen();
		void HurtCheckGround();
		
		bool Kill(SOUNDID soundId);
		bool Hurt(OBJECT *hit);
		bool HurtFromObject(OBJECT *hit);
		
		void LevelBoundSide(int32_t bound);
		void LevelBoundaries();
		
		void SuperPaletteCycle();
		bool SuperTransform();
		void UpdateSuper();
		
		void FrameCommand(const uint8_t* animation);
		void AdvanceFrame(const uint8_t* animation);
		void FlipAngle();
		void Animate();
		
		void CPUControl();
		
		void ControlRoutine();
		void Update();
		
		void Draw();
		void Draw_UpdateStatus();
		
		void DrawToScreen();
		
		void RestoreStateDebug();
		void DebugControl();
		void DebugMode();
		
		void RingAttractCheck(OBJECT *object);
		
		bool ObjectTouch(OBJECT *object, int16_t playerLeft, int16_t playerTop, int16_t playerWidth, int16_t playerHeight);
		void CheckObjectTouch();
		
		void GiveSpeedShoes();
		void GiveInvincibility();
		void GiveBarrier(SOUNDID sound, BARRIER type);
};
