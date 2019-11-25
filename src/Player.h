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
		POSDEF(x)
		POSDEF(y)
		
		//Speeds
		int16_t xVel = 0;		//Global X-velocity
		int16_t yVel = 0;		//Global Y-velocity
		int16_t inertia = 0;	//Horizontal velocity (on ground)
		
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
			bool pinballMode = false;		//If set, the player is forced to be rolling constantly (S-tubes, Casino Night things)
			bool stickToConvex = false;		//If set, the collision will never detect us as running off of a ledge, or when we would normally detach from a ramp when going too fast (the wheel glitch from Carnival Night)
			bool reverseGravity = false;	//If set, gravity is reversed (No, really?)
			bool windTunnel = false;		//Inside a wind tunnel (Labyrinth Zone, Hydrocity)
		} status;
		
		//Items we have
		struct
		{
			bool isInvincible = false;	//Do we have invincibility
			bool hasSpeedShoes = false;	//Do we have speed shoes
			
			//Shield effects
			bool shieldReflect = false;		//Reflects projectile
			bool immuneFire = false;		//Immune to fire
			bool immuneElectric = false;	//Immune to electric
			bool immuneWater = false;		//Immune to water
		} item;
		
		SHIELD shield = SHIELD_NULL;	//Our shield type
		uint8_t jumpAbility = 0;		//Our jump ability (can we use shield ability, insta-shield, dropdash, etc.)
		uint8_t abilityProperty = 0;	//Tails's flying time, Knuckles' gliding speed(?)
		
		uint16_t invulnerabilityTime = 0;
		uint16_t invincibilityTime = 0;
		uint16_t speedShoesTime = 0;
		
		PLAYER_ROUTINE routine = PLAYERROUTINE_CONTROL;	//Routine
		
		uint8_t angle = 0;		//Angle
		uint8_t moveLock = 0;	//Player cannot move if this is non-zero (decrements every frame)
		
		//These two are used for collision, usually represent the player's two ground point angles
		uint8_t primaryAngle = 0;
		uint8_t secondaryAngle = 0;
		
		//These are the above, basically, but from the last frame
		uint8_t tilt = 0;		//set to Secondary Angle
		uint8_t nextTilt = 0;	//set to Primary Angle
		
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
		
		//Spindash
		bool spindashing = false;		//Set if we're spindashing
		uint16_t spindashCounter = 0;	//Our counter for spindashing
		
		//CD Spindash/Peelout
		uint8_t cdSPTimer = 0;
		uint8_t cdChargeDelay = 0;
		
		//Flipping (hit a spring that causes the player to spin about, or running off of that curved ramp in Angel Island Zone)
		uint8_t flipType = 0;
		uint8_t flipAngle = 0;
		uint8_t flipsRemaining = 0;
		uint8_t flipSpeed = 0;
		
		//Air left
		uint8_t airRemaining = 0;
		
		//Our collision layers
		COLLISIONLAYER topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
		COLLISIONLAYER lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
		
		uint16_t restartCountdown = 0;	//Timer for the level restarting after death
		bool inGameover = false;			//If got a game-over
		
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
		
		PALETTESTATE paletteState = PALETTESTATE_REGULAR;
		int16_t paletteTimer = 0;
		uint16_t paletteFrame = 0;
		
		//In debug flag
		uint16_t debug = 0;
		int debugAccel = 0;
		uint8_t debugSpeed = 0;
		int debugObject = 0;
		
		//Character type id (ex. Sonic, Tails, Knuckles, or anything else)
		CHARACTERTYPE characterType;
		
		//Camera scrolling
		unsigned int scrollDelay = 0;
		bool cameraLock = false;
		
		//Position and status records
		struct PLAYER_RECORD
		{
			int16_t x = 0, y = 0;
			CONTROLMASK controlHeld;
			CONTROLMASK controlPress;
			PLAYER_STATUS status;
		} record[PLAYER_RECORD_LENGTH];
		
		int recordPos = 0;
		
		//Objects that belong to us
		OBJECT *spindashDust = nullptr;
		OBJECT *skidDust = nullptr;
		OBJECT *shieldObject = nullptr;
		OBJECT *invincibilityStarObject[INVINCIBILITYSTARS] = {nullptr};
		
		//CPU and other control things
		PLAYERCPU_ROUTINE cpuRoutine = CPUROUTINE_INIT;
		int cpuOverride = 0;
		unsigned int cpuTimer = 0;
		bool cpuJumping = false;
		
		size_t controller;
		PLAYER *follow;
		
		//Current input
		CONTROLMASK controlHeld;
		CONTROLMASK controlPress;
		bool controlLock = false;
		
	public:
		PLAYER(const char *specPath, PLAYER *myFollow, size_t myController);
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
		bool CheckHurt(OBJECT *hit);
		bool HurtCharacter(OBJECT *hit);
		
		void LevelBoundSide(int32_t bound);
		void LevelBound();
		
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
		
		void Display();
		
		void Draw();
		void DrawToScreen();
		
		void RestoreStateDebug();
		void DebugControl();
		void DebugMode();
		
		void RingAttractCheck(OBJECT *object);
		
		bool TouchResponseObject(OBJECT *object, int16_t playerLeft, int16_t playerTop, int16_t playerWidth, int16_t playerHeight);
		void TouchResponse();
		
		void AttachToObject(OBJECT *object, size_t i);
		void MoveOnPlatform(OBJECT *platform, int16_t width, int16_t height, int16_t lastXPos, const int8_t *slope, bool doubleSlope);
		
		void GiveSpeedShoes();
		void GiveInvincibility();
		void GiveShield(SOUNDID sound, SHIELD type);
};
