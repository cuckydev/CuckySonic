#include "SDL_RWops.h"

#include "Player.h"
#include "MathUtil.h"
#include "Audio.h"
#include "Path.h"
#include "Log.h"
#include "Error.h"
#include "Level.h"
#include "Game.h"
#include "Object.h"
#include "Objects.h"
#include "GameConstants.h"

//Bug-fixes
//#define FIX_SPINDASH_ANIM     //Usually when spindashing, if you manage to exit the spindash animation, you'll stay out of it
//#define FIX_SPINDASH_JUMP     //When you jump the frame after you spindash, you'll jump straight upwards
//#define FIX_HORIZONTAL_WRAP   //In the originals, for some reason, the LevelBound uses unsigned checks, meaning if you go off to the left, you'll be sent to the right boundary
//#define FIX_DUCK_CONDITION    //In Sonic and Knuckles, the conditions for ducking are so loose, you can duck (and spindash) in unexpected situations.
//#define FIX_ROLL_YSHIFT       //In the originals, when you roll, you're always shifted up / down globally, this can cause weird behaviour such as falling off of ceilings

//Bug-fix macros
#define YSHIFT_ON_FLOOR(shift)	\
	int16_t sin2, cos2;	\
	GetSine(angle - 0x40, &sin2, &cos2);	\
	xPosLong -= cos2 * shift * 0x100;	\
	yPosLong -= sin2 * shift * (status.reverseGravity ? -0x100 : 0x100);	\

//Game differences (Uncomment all for an experience like that of Sonic 1, and comment all for the experience of Sonic 3 and Knuckles)
//#define SONIC1_SLOPE_ANGLE      //In Sonic 2+, the floor's angle will be replaced with the player's cardinal floor angle if there's a 45+ degree difference
//#define SONIC1_WALK_ANIMATION   //For some reason, in Sonic 2+, the animation code was messed up, making the first frame of the walk animation last only one frame
//#define SONIC1_SLOPE_ROTATION   //In Sonic 2+, a few lines were added to the animation code to make the floor rotation more consistent
//#define SONIC12_SLOPE_RESIST    //In Sonic 3, they made it so you're always affected by slope gravity unless you're on a shallow floor
//#define SONIC12_SLOPE_REPEL     //In Sonic 3, the code to make it so you fall off of walls and ceilings when going too slow was completely redone
//#define SONIC1_GROUND_CAP       //In Sonic 1, your speed on the ground is capped to your top speed when above it, even if you're already above it
//#define SONIC12_AIR_CAP         //In Sonic 1 and 2, your speed in the air is capped to your top speed when above it, even if you're already above it
//#define SONIC123_ROLL_DUCK      //In Sonic and Knuckles, they added a greater margin of speed for ducking and rolling, so you can duck while moving
//#define SONIC12_ROLLJUMP_LAND   //In Sonic 3, they fixed the roll jump landing bug, where you'd land 5 pixels above the ground after jumping from a roll
//#define SONIC1_NO_SPINDASH      //The spindash, it needs no introduction
//#define SONIC12_NO_JUMP_ABILITY //Jump abilities from Sonic 3, such as the insta-shield and elemental shields
//#define SONIC1_DEATH_BOUNDARY   //In Sonic 2, the death boundary code was fixed so that it doesn't use the camera's boundary but the level boundary, so that you don't die while the camera boundary is scrolling
//#define SONIC12_DEATH_RESPAWN   //In Sonic 3, it was changed so that death respawns you once you go off-screen, not when you leave the level boundaries, since this was a very buggy check

//Animation data
#define MAPPINGFRAME_FLIP1 0x5E

static const uint8_t animationWalk[] =			{0xFF,0x08,0x09,0x0A,0x0B,0x06,0x07,0xFF};
static const uint8_t animationRun[] =			{0xFF,0x1E,0x1F,0x20,0x21,0xFF,0xFF,0xFF};
static const uint8_t animationRoll[] =			{0xFE,0x2E,0x2F,0x30,0x31,0x32,0xFF,0xFF};
static const uint8_t animationRoll2[] =			{0xFE,0x2E,0x2F,0x32,0x30,0x31,0x32,0xFF};
static const uint8_t animationPush[] =			{0xFD,0x45,0x46,0x47,0x48,0xFF,0xFF,0xFF};
static const uint8_t animationIdle[] =			{0x17,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x03,0x02,0x02,0x02,0x03,0x04,0xFE,0x02};
static const uint8_t animationBalance[] =		{0x1F,0x3A,0x3B,0xFF};
static const uint8_t animationLookUp[] =		{0x3F,0x05,0xFF};
static const uint8_t animationDuck[] =			{0x3F,0x39,0xFF};
static const uint8_t animationSpindash[] =		{0x00,0x58,0x59,0x58,0x5A,0x58,0x5B,0x58,0x5C,0x58,0x5D,0xFF};
static const uint8_t animationWarp1[] =			{0x3F,0x33,0xFF};
static const uint8_t animationWarp2[] =			{0x3F,0x34,0xFF};
static const uint8_t animationWarp3[] =			{0x3F,0x35,0xFF};
static const uint8_t animationWarp4[] =			{0x3F,0x36,0xFF};
static const uint8_t animationSkid[] =			{0x07,0x37,0x38,0xFF};
static const uint8_t animationFloat1[] =		{0x07,0x3C,0x3F,0xFF};
static const uint8_t animationFloat2[] =		{0x07,0x3C,0x3D,0x53,0x3E,0x54,0xFF};
static const uint8_t animationSpring[] =		{0x2F,0x40,0xFD,0x00};
static const uint8_t animationHang[] =			{0x04,0x41,0x42,0xFF};
static const uint8_t animationLeap1[] =			{0x0F,0x43,0x43,0x43,0xFE,0x01};
static const uint8_t animationLeap2[] =			{0x0F,0x43,0x44,0xFE,0x01};
static const uint8_t animationSurf[] =			{0x3F,0x49,0xFF};
static const uint8_t animationGetAir[] =		{0x0B,0x56,0x56,0x0A,0x0B,0xFD,0x00};
static const uint8_t animationBurnt[] =			{0x20,0x4B,0xFF};
static const uint8_t animationDrown[] =			{0x20,0x4C,0xFF};
static const uint8_t animationDeath[] =			{0x03,0x4D,0xFF};
static const uint8_t animationShrink[] =		{0x03,0x4E,0x4F,0x50,0x51,0x52,0x00,0xFE,0x01};
static const uint8_t animationHurt[] =			{0x03,0x55,0xFF};
static const uint8_t animationWaterSlide[] =	{0x07,0x55,0x57,0xFF};
static const uint8_t animationNull[] =			{0x77,0x00,0xFD,0x00};
static const uint8_t animationFloat3[] =		{0x03,0x3C,0x3D,0x53,0x3E,0x54,0xFF};
static const uint8_t animationFloat4[] =		{0x03,0x3C,0xFD,0x00};

static const uint8_t* animationList[] = {
	animationWalk,
	animationRun,
	animationRoll,
	animationRoll2,
	animationPush,
	animationIdle,
	animationBalance,
	animationLookUp,
	animationDuck,
	animationSpindash,
	animationWarp1,
	animationWarp2,
	animationWarp3,
	animationWarp4,
	animationSkid,
	animationFloat1,
	animationFloat2,
	animationSpring,
	animationHang,
	animationLeap1,
	animationLeap2,
	animationSurf,
	animationGetAir,
	animationBurnt,
	animationDrown,
	animationDeath,
	animationShrink,
	animationHurt,
	animationWaterSlide,
	animationNull,
	animationFloat3,
	animationFloat4,
};

static const uint8_t* animationListSuper[] = {
	animationWalk,
	animationRun,
	animationRoll,
	animationRoll2,
	animationPush,
	animationIdle,
	animationBalance,
	animationLookUp,
	animationDuck,
	animationSpindash,
	animationWarp1,
	animationWarp2,
	animationWarp3,
	animationWarp4,
	animationSkid,
	animationFloat1,
	animationFloat2,
	animationSpring,
	animationHang,
	animationLeap1,
	animationLeap2,
	animationSurf,
	animationGetAir,
	animationBurnt,
	animationDrown,
	animationDeath,
	animationShrink,
	animationHurt,
	animationWaterSlide,
	animationNull,
	animationFloat3,
	animationFloat4,
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
	switch (object->routine)
	{
		case 0:
			//Initialize render properties
			object->texture = gLevel->GetObjectTexture("data/Object/SpindashDust.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/SpindashDust.map");
			
			object->priority = 1;
			object->widthPixels = 16;
			object->renderFlags.alignPlane = true;
			
			//Increment routine
			object->routine = 1;
//Fallthrough
		case 1:
			//If we're active
			if (object->anim == 1)
			{
				//Are we to be deleted?
				if (((PLAYER*)object->parent)->routine != PLAYERROUTINE_CONTROL || ((PLAYER*)object->parent)->spindashing == false)
				{
					object->anim = 0;
					break;
				}
				
				//Copy the player's position
				object->status.xFlip = ((PLAYER*)object->parent)->status.xFlip;
				object->status.yFlip = ((PLAYER*)object->parent)->status.reverseGravity;
				object->highPriority = ((PLAYER*)object->parent)->highPriority;
				
				object->x.pos = ((PLAYER*)object->parent)->x.pos;
				object->y.pos = ((PLAYER*)object->parent)->y.pos;
				
				//Offset if our height is non-default
				int heightDifference = 0x13 - ((PLAYER*)object->parent)->defaultYRadius;
				if (heightDifference)
				{
					if (((PLAYER*)object->parent)->status.reverseGravity)
						object->y.pos += heightDifference;
					else
						object->y.pos -= heightDifference;
				}
			}
			break;
	}
	
	//Draw and animate
	object->Animate(animationListSpindashDust);
	object->Draw();
}

//Player class
PLAYER::PLAYER(PLAYER **linkedList, const char *specPath, PLAYER *myFollow, int myController)
{
	LOG(("Creating a player with spec %s and controlled by controller %d...\n", specPath, myController));
	memset(this, 0, sizeof(PLAYER));
	
	//Equivalent of routine 0
	routine = PLAYERROUTINE_CONTROL;
	
	//Load art and mappings
	GET_APPEND_PATH(texPath, specPath, ".bmp");
	GET_APPEND_PATH(mapPath, specPath, ".map");
	
	texture = gLevel->GetObjectTexture(texPath);
	if (texture->fail != NULL)
	{
		Error(fail = texture->fail);
		return;
	}
	
	mappings = gLevel->GetObjectMappings(mapPath);
	if (mappings->fail != NULL)
	{
		Error(fail = mappings->fail);
		return;
	}
	
	//Read properties from the specifications
	GET_APPEND_PATH(plrSpecPathNG, specPath, ".psp");
	GET_GLOBAL_PATH(plrSpecPath, plrSpecPathNG);
	
	SDL_RWops *playerSpec = SDL_RWFromFile(plrSpecPath, "rb");
	
	if (playerSpec == NULL)
	{
		Error(fail = SDL_GetError());
		return;
	}
	
	xRadius = SDL_ReadU8(playerSpec);
	yRadius = SDL_ReadU8(playerSpec);
	
	defaultXRadius = xRadius;
	defaultYRadius = yRadius;
	rollXRadius = SDL_ReadU8(playerSpec);
	rollYRadius = SDL_ReadU8(playerSpec);
	
	characterType = (CHARACTERTYPE)SDL_ReadU8(playerSpec);
	
	//Close file
	SDL_RWclose(playerSpec);
	
	//Set render properties
	priority = 2;
	widthPixels = 0x18;
	
	//Render flags
	renderFlags.alignPlane = true;
	
	//Initialize our speeds
	top = 0x0600;
	acceleration = 0x000C;
	deceleration = 0x0080;
	
	//Collision
	topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
	lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
	
	//Flipping stuff
	flipsRemaining = 0;
	flipSpeed = 4;
	
	super = false;
	
	//Set our following person
	follow = (void*)myFollow;
	
	//Set the controller to use
	controller = myController;
	
	//Load our objects
	spindashDust = new OBJECT(&gLevel->objectList, ObjSpindashDust);
	((OBJECT*)spindashDust)->parent = this;
	
	//Initialize our record arrays
	for (int i = 0; i < PLAYER_RECORD_LENGTH; i++)
	{
		posRecord[i].x = x.pos - 0x20;	//Why the hell is the position offset?
		posRecord[i].y = x.pos - 0x04;
		memset(&statRecord[i], 0, sizeof(statRecord[0])); //Stat record is initialized as 0
	}
	
	recordPos = 0;
	
	//Attach to linked list (if applicable)
	if (linkedList != NULL)
	{
		list = linkedList;
		
		//If linked list is unset, set us as the first 
		if (*linkedList == NULL)
		{
			*linkedList = this;
			return;
		}
		
		//Attach us to the linked list
		for (PLAYER *player = *linkedList;; player = player->next)
		{
			if (player->next == NULL)
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
	if (list != NULL)
	{
		for (PLAYER **player = list; *player != NULL; player = &(*player)->next)
		{
			if (*player == this)
			{
				*player = next;
				break;
			}
		}
	}
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

	if (distance != NULL)
		*distance = retDistance;

	if (distance2 != NULL)
		*distance2 = retDistance2;

	if (outAngle != NULL)
		*outAngle = retAngle;
}

void PLAYER::CheckCeiling(COLLISIONLAYER layer, int16_t *distance, int16_t *distance2, uint8_t *outAngle)
{
	int16_t retDistance = FindFloor(x.pos + xRadius, y.pos - yRadius, layer, true, &primaryAngle);
	int16_t retDistance2 = FindFloor(x.pos - xRadius, y.pos - yRadius, layer, true, &secondaryAngle);

	uint8_t retAngle = AngleIn(0x80, &retDistance, &retDistance2);

	if (distance != NULL)
		*distance = retDistance;

	if (distance2 != NULL)
		*distance2 = retDistance2;
	
	if (outAngle != NULL)
		*outAngle = retAngle;
}

//Check floor distance
int16_t PLAYER::ChkFloorEdge(COLLISIONLAYER layer, int16_t xPos, int16_t yPos, uint8_t *outAngle)
{
	//Clear primary angle
	primaryAngle = 0;
	
	//Get floor distance and angle
	int16_t distance = FindFloor(xPos, yPos + (status.reverseGravity ? -yRadius : yRadius), layer, status.reverseGravity, &primaryAngle);
	if (outAngle != NULL)
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
	if (outAngle != NULL)
		*outAngle = AngleSide(0x00);
	return distance;
}

int16_t PLAYER::CheckCeilingDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindFloor(xPos, yPos - 10, layer, true, &primaryAngle);
	if (outAngle != NULL)
		*outAngle = AngleSide(0x80);
	return distance;
}

int16_t PLAYER::CheckLeftWallDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindWall(xPos - 10, yPos, layer, true, &primaryAngle);
	if (outAngle != NULL)
		*outAngle = AngleSide(0x40);
	return distance;
}

int16_t PLAYER::CheckRightWallDist(int16_t xPos, int16_t yPos, COLLISIONLAYER layer, uint8_t *outAngle)
{
	int16_t distance = FindWall(xPos + 10, yPos, layer, false, &primaryAngle);
	if (outAngle != NULL)
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
			CheckFloor(topSolidLayer, NULL, &distance, NULL);
			break;
		case 0x40:
			distance = CheckLeftCeilingDist(lrbSolidLayer, NULL);
			break;
		case 0x80:
			CheckCeiling(lrbSolidLayer, NULL, &distance, NULL);
			break;
		case 0xC0:
			distance = CheckRightCeilingDist(lrbSolidLayer, NULL);
			break;
	}

	return distance;
}

//Calculate room in front of us
int16_t PLAYER::CalcRoomInFront(uint8_t moveAngle)
{
	int16_t xPos = (xPosLong + (xVel * 0x100)) / 0x10000;
	int16_t yPos = (yPosLong + (yVel * (status.reverseGravity ? -0x100 : 0x100))) / 0x10000;

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
		return CheckFloorDist(xPos, yPos, lrbSolidLayer, NULL);
	}
	else if (offAngle == 0x80)
	{
		return CheckCeilingDist(xPos, yPos, lrbSolidLayer, NULL);
	}
	else
	{
		//If at a low angle, offset the position down 8 pixels
		if ((angle & 0x38) == 0)
			yPos += 8;
		
		if (offAngle == 0x40)
			return CheckLeftWallDist(xPos, yPos, lrbSolidLayer, NULL);
		else
			return CheckRightWallDist(xPos, yPos, lrbSolidLayer, NULL);
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
					uint8_t clipLength = abs(xVel / 0x100) + 4;
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
					uint8_t clipLength = abs(yVel / 0x100) + 4;
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
					uint8_t clipLength = abs(xVel / 0x100) + 4;
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
					uint8_t clipLength = abs(yVel / 0x100) + 4;
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
	
	if (((angle & 0x3F) == 0 || ((angle + 0x40) & 0xFF) < 0x80) && inertia != 0)
	{
		uint8_t faceAngle = angle + (inertia < 0 ? 0x40 : -0x40);
		int16_t distance = CalcRoomInFront(faceAngle);

		if (distance < 0)
		{
			distance *= 0x100;

			switch ((faceAngle + 0x20) & 0xC0)
			{
				case 0:
				{
					yVel += distance;
					break;
				}
				case 0x40:
				{
					xVel -= distance;
					status.pushing = true;
					inertia = 0;
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
					status.pushing = true;
					inertia = 0;
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
			distance = CheckLeftWallDist(x.pos, y.pos, lrbSolidLayer, NULL);
			if (distance < 0)
			{
				//Clip out and stop our velocity
				x.pos -= distance;
				xVel = 0;
			}
			
			distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, NULL);
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
			const int8_t clipLength = -((yVel / 0x100) + 8);
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
				ResetOnFloor();
			}
			break;
		}
		
		case 0x40: //Moving to the left
		{
			//Collide with walls to the left of us
			int16_t distance = CheckLeftWallDist(x.pos, y.pos, lrbSolidLayer, NULL);

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
					CheckCeiling(lrbSolidLayer, NULL, &distance, NULL);
				else
					CheckFloor(topSolidLayer, NULL, &distance, NULL);
				
				if (distance < 0)
				{
					if (distance > -14)
					{
						//Clip out of ceiling
						if (!status.reverseGravity)
							y.pos -= distance;
						else
							y.pos += distance;
						
						//Stop our vertical velocity
						if (yVel < 0)
							yVel = 0;
					}
					else
					{
						//Collide with walls to the right?
						int16_t distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, NULL);
						
						if (distance < 0)
						{
							x.pos += distance;
							xVel = 0;
						}
					}
				}
				else if (status.windTunnel || yVel >= 0)
				{
					//Collide with the floor
					uint8_t floorAngle;
					if (!status.reverseGravity)
					{
						CheckFloor(topSolidLayer, NULL, &distance, &floorAngle);
					}
					else
					{
						CheckCeiling(lrbSolidLayer, NULL, &distance, &floorAngle);
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
						ResetOnFloor();
					}
				}
			}
			break;
		}
		
		case 0x80: //Moving upwards
		{
			//Check for wall collisions
			int16_t distance;
			distance = CheckLeftWallDist(x.pos, y.pos, lrbSolidLayer, NULL);
			if (distance < 0)
			{
				//Clip out of the wall and stop our velocity
				x.pos -= distance;
				xVel = 0;
			}
			
			distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, NULL);
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
				CheckCeiling(lrbSolidLayer, NULL, &distance, &ceilingAngle);
			}
			else
			{
				CheckFloor(topSolidLayer, NULL, &distance, &ceilingAngle);
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
					ResetOnFloor();
					inertia = ceilingAngle >= 0x80 ? -yVel : yVel;
				}
			}
			break;
		}
		
		case 0xC0: //Moving to the right
		{
			//Collide with walls
			int16_t distance = CheckRightWallDist(x.pos, y.pos, lrbSolidLayer, NULL);

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
					CheckCeiling(lrbSolidLayer, NULL, &distance, NULL);
				else
					CheckFloor(topSolidLayer, NULL, &distance, NULL);
				
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
						CheckFloor(topSolidLayer, NULL, &distance, &floorAngle);
					}
					else
					{
						CheckCeiling(lrbSolidLayer, NULL, &distance, &floorAngle);
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
						ResetOnFloor();
					}
				}
			}
			break;
		}
	}
}

//Functions for landing on the ground
void PLAYER::ResetOnFloor()
{
	if (status.pinballMode)
	{
		//Do not exit ball form if in pinball mode
		ResetOnFloor3();
	}
	else
	{
		//Exit ball form
		#ifdef FIX_SPINDASH_ANIM
			if (!spindashing)
				anim = PLAYERANIMATION_WALK;
			else
				anim = PLAYERANIMATION_SPINDASH;
		#else
			anim = PLAYERANIMATION_WALK;
		#endif
		
		ResetOnFloor2();
	}
}

void PLAYER::ResetOnFloor2()
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
		
		#ifndef FIX_ROLL_YSHIFT
			if (status.reverseGravity)
				y.pos += difference;
			else
				y.pos -= difference;
		#else
			YSHIFT_ON_FLOOR(-difference);
		#endif
	}

	ResetOnFloor3();
}

void PLAYER::ResetOnFloor3()
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
			if ((characterType == CHARACTERTYPE_SONIC) && super == false)
			{
				//Bubble shield bounce
				if (item.hasShield && shield == SHIELD_BUBBLE)
				{
					//Get the force of our bounce
					int16_t bounceForce = 0x780;
					if (status.underwater)
						bounceForce = 0x400;
					
					//Bounce us up from the ground
					int16_t sin, cos;
					GetSine(angle - 0x40, &sin, &cos);
					xVel += (cos * bounceForce) / 0x100;
					yVel += (sin * bounceForce) / 0x100;
					
					//Put us back into the air state
					status.inAir = true;
					status.pushing = false;
					status.jumping = true;
					anim = PLAYERANIMATION_ROLL;
					status.inBall = true;
					
					//Return to the ball hitbox
					xRadius = rollXRadius;
					yRadius = rollYRadius;
					
					#ifndef FIX_ROLL_YSHIFT
						if (status.reverseGravity)
							y.pos += (defaultYRadius - yRadius);
						else
							y.pos -= (defaultYRadius - yRadius);
					#else
						YSHIFT_ON_FLOOR(-(defaultYRadius - yRadius));
					#endif
					
					//Play the sound
					//PlaySound(SOUNDID_BUBBLE_BOUNCE);
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

//Spindash code
const uint16_t spindashSpeed[9] =		{0x800, 0x880, 0x900, 0x980, 0xA00, 0xA80, 0xB00, 0xB80, 0xC00};
const uint16_t spindashSpeedSuper[9] =	{0xB00, 0xB80, 0xC00, 0xC80, 0xD00, 0xD80, 0xE00, 0xE80, 0xF00};
		
bool PLAYER::Spindash()
{
	#ifdef SONIC1_NO_SPINDASH
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
				((OBJECT*)spindashDust)->anim = 1;
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
			#ifndef FIX_ROLL_YSHIFT
				if (status.reverseGravity)
					y.pos -= 5;
				else
					y.pos += 5;
			#else
				YSHIFT_ON_FLOOR(5);
			#endif
			
			//Release spindash
			spindashing = false;
			
			//Set our speed
			if (super)
				inertia = spindashSpeedSuper[spindashCounter / 0x100];
			else
				inertia = spindashSpeed[spindashCounter / 0x100];
			
			//Lock the camera behind us
			scrollDelay = (0x2000 - (inertia - 0x800) * 2) % (PLAYER_RECORD_LENGTH << 8);
			
			//Revert if facing left
			if (status.xFlip)
				inertia = -inertia;
			
			//Actually go into the roll routine
			status.inBall = true;
			((OBJECT*)spindashDust)->anim = 0;
			PlaySound(SOUNDID_SPINDASH_RELEASE);
			
			#ifdef FIX_SPINDASH_JUMP
				//Convert our inertia into global speeds
				int16_t sin, cos;
				GetSine(angle, &sin, &cos);
				xVel = (cos * inertia) / 0x100;
				yVel = (sin * inertia) / 0x100;
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

//Jump ability functions
void PLAYER::JumpAbilities()
{
	if (jumpAbility == 0 && (controlPress.a || controlPress.b || controlPress.c))
	{
		//Perform our ability
		if (item.hasShield && shield != SHIELD_BLUE) //If can use an ability
		{
			status.rollJumping = false; //Clear the rolljump flag
		}
		
		//Disable our ability flag
		jumpAbility = 1;
	}
}

//Jumping functions
void PLAYER::JumpHeight()
{
	if (status.jumping)
	{
		//Slow us down if ABC is released when jumping
		int16_t minVelocity = status.underwater ? -0x200 : -0x400;
		
		#ifndef SONIC12_NO_JUMP_ABILITY
			if (minVelocity <= yVel)
				JumpAbilities();
			else if (!controlHeld.a && !controlHeld.b && !controlHeld.c)
				yVel = minVelocity;
		#else
			if (minVelocity > yVel && !controlHeld.a && !controlHeld.b && !controlHeld.c)
				yVel = minVelocity;
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
	if (!status.rollJumping)
	{
		int16_t newVelocity = xVel;
		
		//Move left if left is held
		if (controlHeld.left)
		{
			//Accelerate left
			status.xFlip = true;
			newVelocity -= acceleration * 2;
			
			//Don't accelerate past the top speed
			#ifndef SONIC12_AIR_CAP
				if (newVelocity <= -top)
				{
					newVelocity += acceleration * 2;
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
			newVelocity += acceleration * 2;
			
			//Don't accelerate past the top speed
			#ifndef SONIC12_AIR_CAP
				if (newVelocity >= top)
				{
					newVelocity -= acceleration * 2;
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
			//Get the velocity of our jump
			int16_t jumpVelocity;
			
			if (characterType != CHARACTERTYPE_KNUCKLES)
			{
				//Normal jumping
				jumpVelocity = super ? 0x800 : 0x680;
				
				//Lower our jump in water
				if (status.underwater)
					jumpVelocity = 0x380;
			}
			else
			{
				//Knuckles' low jumping
				jumpVelocity = status.underwater ? 0x300 : 0x600;
			}
			
			//Apply the velocity
			int16_t sin, cos;
			GetSine(angle - 0x40, &sin, &cos);
			
			xVel += (cos * jumpVelocity) / 0x100;
			yVel += (sin * jumpVelocity) / 0x100;
			
			//Put us in the jump state
			status.inAir = true;
			status.pushing = false;
			status.jumping = true;
			status.stickToConvex = false;
			
			PlaySound(SOUNDID_JUMP);
			
			//Handle our collision and roll state
			xRadius = defaultXRadius;
			yRadius = defaultYRadius;
			
			if (!status.inBall)
			{
				//Go into ball form
				xRadius = rollXRadius;
				yRadius = rollYRadius;
				anim = PLAYERANIMATION_ROLL;
				status.inBall = true;
				
				//Shift us down to the ground
				#ifndef FIX_ROLL_YSHIFT
					if (status.reverseGravity)
						y.pos += (yRadius - defaultYRadius);
					else
						y.pos -= (yRadius - defaultYRadius);
				#else
					YSHIFT_ON_FLOOR(-(yRadius - defaultYRadius));
				#endif
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
		GetSine(angle, &sin, NULL);
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
		GetSine(angle, &sin, NULL);
		sin = (sin * 0x50) / 0x100;
		
		//Apply our slope gravity (divide by 4 if opposite to our inertia sign)
		if (inertia >= 0)
		{
			if (sin < 0)
				sin /= 4;
			inertia += sin;
		}
		else
		{
			if (sin >= 0)
				sin /= 4;
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
			
			//TODO: Create dust here
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
			
			//TODO: Create dust here
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
			if (controlHeld.left)
				MoveLeft();
			if (controlHeld.right)
				MoveRight();
			
			if (((angle + 0x20) & 0xC0) == 0 && inertia == 0)
			{
				//Do idle animation
				status.pushing = false;
				anim = PLAYERANIMATION_IDLE;
				
				//Balancing
				if (status.shouldNotFall)
				{
					OBJECT *object = (OBJECT*)interact;
					
					//Balancing on an object
					if (object != NULL && !object->status.noBalance)
					{
						//Get our area we stand on
						int width = (object->widthPixels * 2) - 4;
						int xDiff = (object->widthPixels + x.pos) - object->x.pos;
						
						if (xDiff < 4)
						{
							status.xFlip = true;
							anim = PLAYERANIMATION_BALANCE;
						}
						else if (xDiff >= width)
						{
							status.xFlip = false;
							anim = PLAYERANIMATION_BALANCE;
						}
					}
				}
				else
				{
					//If Sonic's middle bottom point is 12 pixels away from the floor, start balancing
					if (ChkFloorEdge(topSolidLayer, x.pos, y.pos, NULL) >= 12)
					{
						if (nextTilt == 3) //If there's no floor to the left of us
						{
							status.xFlip = false;
							anim = PLAYERANIMATION_BALANCE;
						}
						else if (tilt == 3) //If there's no floor to the right of us
						{
							status.xFlip = true;
							anim = PLAYERANIMATION_BALANCE;
						}
					}
				}
				
				//Look up and down
				if (anim == PLAYERANIMATION_IDLE)
				{
					if (controlHeld.up)
						anim = PLAYERANIMATION_LOOKUP;
					else if (controlHeld.down)
						anim = PLAYERANIMATION_DUCK; //This is done in Roll too
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
	xVel = (cos * inertia) / 0x100;
	yVel = (sin * inertia) / 0x100;
	
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
		#ifndef FIX_ROLL_YSHIFT
			if (status.reverseGravity)
				y.pos -= 5;
			else
				y.pos += 5;
		#else
			YSHIFT_ON_FLOOR(5);
		#endif
		
		//Play the sound
		PlaySound(SOUNDID_ROLL);
		
		//Code that doesn't trigger (leftover from Sonic 1's S-tubes)
		if (inertia == 0)
			inertia = 0x200;
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
				else
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
	//Get our friction (super has separate friction) and deceleration when pulling back
	uint16_t friction = acceleration >> 1;
	if (super)
		friction = 6;
	
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
		
		//Friction
		if (inertia > 0)
		{
			inertia -= friction;
			if (inertia < 0)
				inertia = 0;
		}
		else if (inertia < 0)
		{
			inertia += friction;
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
				
				#ifndef FIX_ROLL_YSHIFT
					if (status.reverseGravity)
						y.pos += 5;
					else
						y.pos -= 5;
				#else
					YSHIFT_ON_FLOOR(-5);
				#endif
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
	xVel = (cos * inertia) / 0x100;
	yVel = (sin * inertia) / 0x100;
	
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
	
	//Cancel spindash
	spindashing = false;
	
	//Check if we're off-screen
	#ifndef SONIC12_DEATH_RESPAWN
		if (status.reverseGravity)
		{
			if (y.pos > cameraY - 0x10)
				return;
		}
		else
		{
			if (y.pos < cameraY + (SCREEN_HEIGHT + 0x20))
				return;
		}
	#else
		if ((unsigned)y.pos < gLevel->bottomBoundaryTarget + 0x20)
			return;
	#endif
	
	//Enter respawn state
	routine = PLAYERROUTINE_RESET_LEVEL;
	restartCountdown = 60;
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
		item.hasShield = false;
		item.isInvincible = false;
		item.hasSpeedShoes = false;
		item.shieldReflect = false;
		item.immuneFire = false;
		item.immuneElectric = false;
		item.immuneWater = false;
		
		routine = PLAYERROUTINE_DEATH;
		ResetOnFloor2();
		status.inAir = true;
		
		//Set our velocity
		xVel = 0;
		yVel = -0x700;
		inertia = 0;
		
		//If the lead player, freeze level
		if (follow == NULL)
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
	if (item.hasShield || item.isInvincible || item.immuneFire || item.immuneElectric || item.immuneWater)
	{
		//If we're immune to the object, don't hurt us
		if ((item.immuneFire && object->hurtType.fire) || (item.immuneElectric && object->hurtType.electric) || (item.immuneWater && object->hurtType.water))
			return true;
		
		//If invincible, don't hurt us
		if (!item.hasShield)
			return true;
	}
	
	//If has a power-up or using the character's special move thing (flying, gliding)
	if (jumpAbility == 1 || item.hasShield || item.isInvincible)
	{
		//If we should be reflected, reflect
		if (object->hurtType.reflect)
		{
			//Get the velocity to reflect at
			int16_t xVel, yVel;
			uint8_t angle = GetAtan(x.pos - object->x.pos, y.pos - object->y.pos);
			GetSine(angle, &xVel, &yVel);
			
			object->xVel = (xVel * 0x800) >> 8;
			object->yVel = (yVel * 0x800) >> 8;
			
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
	if (item.hasShield || item.shieldReflect || item.immuneFire || item.immuneElectric || item.immuneWater)
	{
		item.hasShield = false;
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
		}
	}
	
	//Enter hurt routine
	routine = PLAYERROUTINE_HURT;
	ResetOnFloor2();
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
		#define lbType int32_t
	#else
		#define lbType uint16_t
	#endif
	
	lbType nextPos = (xPosLong + (xVel * 0x100)) / 0x10000;
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
						return;
				#endif
				
				//Get the 45 degree increment to rotate our sprites at
				uint8_t rotAngle = angle;
				
				#ifndef SONIC1_SLOPE_ROTATION
					if (rotAngle > 0 && rotAngle < 0x80)
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
				
				//Run our animation
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
							animation = animationList[PLAYERANIMATION_RUN];
							angleIncrement *= 4;
						}
						else
						{
							//Walk animation (below 0x600 speed)
							animation = animationList[PLAYERANIMATION_WALK];
							angleIncrement *= 6;
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
							animFrameDuration = speedFactor / 0x100;
							
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
								animFrameDuration = speedFactor / 0x100;
								
								//Increment frame
								animFrame++;
							}
						#endif
						return;
					}
					else
					{
						//Super walk and running (no super frames atm)
						return;
					}
				}
				else
				{
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
					
					//Set animation and flip
					animation = aniList[PLAYERANIMATION_PUSH];
					renderFlags.xFlip = status.xFlip;
					renderFlags.yFlip = false;
					
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
			animFrameDuration = speedFactor / 0x100;
			
			//Advance frame
			AdvanceFrame(animation);
			return;
		}
	}
}

//CPU
void PLAYER::CPUFilterAction(CONTROLMASK *nextHeld, CONTROLMASK *nextPress, int16_t leadX, int16_t leadY, bool incP1)
{
	if (incP1)
	{
		//Handle jumping
		if (cpuJumping)
		{
			nextHeld->a = true;
			nextHeld->b = true;
			nextHeld->c = true;
			nextPress->a = true;
			nextPress->b = true;
			nextPress->c = true;
			
			if (!status.inAir)
				cpuJumping = false;
			else
				return;
		}
		
		//If there's a great X-difference attached to a stupid timer thing
		unsigned int timer = (cpuTimer & 0xFF);
		if (timer != 0 && abs(leadX - x.pos) >= 0x40)
			return;
		
		//Y difference check
		int16_t yDiff = leadY - y.pos;
		if (yDiff >= 0 || yDiff > -0x20)
			return;
	}
	
	//Check timer and if we're ducking
	unsigned int timer = cpuTimer & 0x3F;
	if (timer != 0 || anim == PLAYERANIMATION_DUCK)
		return;
	
	//Jump
	nextHeld->a = true;
	nextHeld->b = true;
	nextHeld->c = true;
	nextPress->a = true;
	nextPress->b = true;
	nextPress->c = true;
	cpuJumping = true;
}

void PLAYER::CPUControl()
{
	//Give the player control if any buttons are pressed
	if (controlHeld.a || controlHeld.b || controlHeld.c || controlHeld.start || controlHeld.left || controlHeld.right || controlHeld.down || controlHeld.up)
		cpuOverride = 600; //10 seconds before control is restored to the CPU
	
	switch (cpuRoutine)
	{
		case CPUROUTINE_INIT:
			//Set our intended routine
			cpuRoutine = CPUROUTINE_NORMAL;
			
			//Clear object control
			memset(&objectControl, 0, sizeof(objectControl));
			
			//Reset speed and animation
			anim = PLAYERANIMATION_WALK;
			xVel = 0;
			yVel = 0;
			inertia = 0;
			
			//Clear status
			memset(&status, 0, sizeof(STATUS));
			break;
		case CPUROUTINE_NORMAL:
			//If the lead is dead, fly down to them
			PLAYER *lead;
			for (lead = (PLAYER*)follow; lead->follow != NULL; lead = (PLAYER*)lead->follow);
			
			if (lead->routine == PLAYERROUTINE_DEATH)
			{
				//Fly down to the player's corpse
				cpuRoutine = CPUROUTINE_FLYING;
				spindashing = false;
				spindashCounter = 0;
				
				objectControl.disableOurMovement = true;
				objectControl.disableObjectInteract = true;
				
				//Clear status
				memset(&status, 0, sizeof(STATUS));
				status.inAir = true;
				return;
			}
			
			//If not under human or object control
			if (cpuOverride == 0 && objectControl.disableOurMovement == false)
			{
				//Panic if locked from a slope and stopped
				if (moveLock > 0 && inertia == 0)
					cpuRoutine = CPUROUTINE_PANIC;
				
				//Copy our leads controls from approximately 16 frames ago
				int index = (unsigned)(((PLAYER*)follow)->recordPos - 0x11) % PLAYER_RECORD_LENGTH;
				
				int leadX = ((PLAYER*)follow)->posRecord[index].x;
				int leadY = ((PLAYER*)follow)->posRecord[index].y;
				CONTROLMASK leadHeld = ((PLAYER*)follow)->statRecord[index].controlHeld;
				CONTROLMASK leadPress = ((PLAYER*)follow)->statRecord[index].controlPress;
				STATUS leadStatus = ((PLAYER*)follow)->statRecord[index].status;
				
				//Move towards the lead
				if (!status.pushing)
				{
					CONTROLMASK nextHeld = leadHeld;
					CONTROLMASK nextPress = leadPress;
					
					//If the lead isn't pushing
					if (!leadStatus.pushing)
					{
						int16_t xDiff = leadX - x.pos;
							
						//If not lined up with the lead, move towards them
						if (xDiff < 0)
						{
							//Move left towards the lead
							if (xDiff <= -0x10)
							{
								nextHeld.left = true;
								nextPress.left = true;
								nextHeld.right = false;
								nextPress.right = false;
							}
							
							//Move left 1 pixel if moving and not facing left
							if (inertia != 0 && status.xFlip == false)
								x.pos--;
						}
						else if (xDiff > 0)
						{
							//Move right towards the lead
							if (xDiff >= 0x10)
							{
								nextHeld.left = false;
								nextPress.left = false;
								nextHeld.right = true;
								nextPress.right = true;
							}
							
							//Move left 1 pixel if moving and not facing left
							if (inertia != 0 && status.xFlip == true)
								x.pos++;
						}
						else
						{
							//Standing still
							status.xFlip = leadStatus.xFlip;
						}
						
						//Get our held buttons and actions
						CPUFilterAction(&nextHeld, &nextPress, leadX, leadY, true);
						
						//Copy held and press
						controlHeld = nextHeld;
						controlPress = nextPress;
						break;
					}
					
					//Get our held buttons and actions
					CPUFilterAction(&nextHeld, &nextPress, leadX, leadY, false);
					
					//Copy held and press
					controlHeld = nextHeld;
					controlPress = nextPress;
					break;
				}
			}
			else
			{
				if (cpuOverride > 0)
					cpuOverride--;
			}
			break;
		case CPUROUTINE_PANIC:
			if (cpuOverride == 0 && moveLock == 0)
			{
				if (spindashing == false)
				{
					//Prepare to spindash
					if (inertia == 0)
					{
						//Face towards our lead
						status.xFlip = (((PLAYER*)follow)->x.pos < x.pos);
						
						//Force our input to down
						controlHeld = {false, false, false, false, false, false, false, false};
						controlPress = {false, false, false, false, false, false, false, false};
						controlHeld.down = true;
						controlPress.down = true;
						
						//If taking too long, quit
						if ((cpuTimer & 0x7F) == 0)
						{
							controlHeld = {false, false, false, false, false, false, false, false};
							controlPress = {false, false, false, false, false, false, false, false};
							cpuRoutine = CPUROUTINE_NORMAL;
							return;
						}
						
						//Initiate our spindash if ducking
						if (anim == PLAYERANIMATION_DUCK)
						{
							controlHeld = {false, false, false, false, false, false, false, false};
							controlHeld.down = true;
							controlHeld.a = true;
							controlHeld.b = true;
							controlHeld.c = true;
							controlPress = {false, false, false, false, false, false, false, false};
							controlPress.down = true;
							controlPress.a = true;
							controlPress.b = true;
							controlPress.c = true;
						}
					}
				}
				else
				{
					//Force our input to down
					controlHeld = {false, false, false, false, false, false, false, false};
					controlPress = {false, false, false, false, false, false, false, false};
					controlHeld.down = true;
					controlPress.down = true;
					
					//Release spindash at some point
					if ((cpuTimer & 0x7F) == 0)
					{
						controlHeld = {false, false, false, false, false, false, false, false};
						controlPress = {false, false, false, false, false, false, false, false};
						cpuRoutine = CPUROUTINE_NORMAL;
					}
					
					//Every 20 frames rev our spindash
					else if ((cpuTimer & 0x1F) == 0)
					{
						controlHeld.a = true;
						controlHeld.b = true;
						controlHeld.c = true;
						controlPress.a = true;
						controlPress.b = true;
						controlPress.c = true;
					}
				}
			}
			break;
		default:
			break;
	}
	
	//Increment CPU timer
	cpuTimer++;
}

//Update
void PLAYER::Update()
{
	//Clear drawing flag
	isDrawing = false;
	
	//Update (check if we're using debug, first)
	switch (debug & 0xFF)
	{
		case 0:
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
					if (follow == NULL)
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
						//Standing / walking on ground
						if (status.inBall == false && status.inAir == false)
						{
							if (Spindash() && Jump())
							{
								//Handle slope gravity and our movement
								SlopeResist();
								Move();
								Roll();
								
								//Keep us in level bounds
								LevelBound();
								
								//Move according to our velocity
								xPosLong += xVel * 0x100;
								if (status.reverseGravity)
									yPosLong -= yVel * 0x100;
								else
									yPosLong += yVel * 0x100;
								
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
								
								//Move according to our velocity
								xPosLong += xVel * 0x100;
								if (status.reverseGravity)
									yPosLong -= yVel * 0x100;
								else
									yPosLong += yVel * 0x100;
								
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
							xPosLong += xVel * 0x100;
							if (status.reverseGravity)
								yPosLong -= yVel * 0x100;
							else
								yPosLong += yVel * 0x100;
							
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
					
					//Draw, record position, and handle super and water
					Display();
					//bsr.w	SonicKnux_SuperHyper
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
					xPosLong += xVel * 0x100;
					if (status.reverseGravity)
						yPosLong -= yVel * 0x100;
					else
						yPosLong += yVel * 0x100;
					
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
					xPosLong += xVel * 0x100;
					if (status.reverseGravity)
						yPosLong -= yVel * 0x100;
					else
						yPosLong += yVel * 0x100;
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
		
		case 1: //Object placement mode
			DebugMode();
			Draw();
			break;
			
		case 2: //Mapping test mode
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
	
	//Restart if start + a
	if (gController[controller].press.start && gController[controller].held.a)
		gLevel->SetFade(false, false);
}

//Draw our player
void PLAYER::Display()
{
	//Handle invulnerability (decrement the invulnerability time and don't draw us every 4 frames)
	if (invulnerabilityTime == 0 || (--invulnerabilityTime & 0x4) != 0)
		Draw();
	
	//Handle invincibility (every 8 frames)
	if (item.isInvincible && invincibilityTime != 0 && (gLevel->frameCounter & 0x7) == 0 && -invincibilityTime == 0)
	{
		//Resume music
		//if (gCurrentMusic == MUSICID_INVINCIBILITY)
			PlayMusic(gLevel->musicId);
		
		//Lose invincibility
		item.isInvincible = false;
	}
	
	//Handle speed shoes (every 8 frames)
	if (item.hasSpeedShoes && speedShoesTime != 0 && (gLevel->frameCounter & 0x7) == 0 && --speedShoesTime == 0)
	{
		//Resume music
		if (gCurrentMusic == MUSICID_SPEED_SHOES)
			PlayMusic(gLevel->musicId);
		
		//Lose speed shoes
		item.hasSpeedShoes = false;
		
		//Reset our speed
		if (super)
		{
			top = 0xA00;
			acceleration = 0x30;
			deceleration = 0x100;
		}
		else
		{
			top = 0x600;
			acceleration = 0xC;
			deceleration = 0x80;
		}
	}
}

void PLAYER::Draw()
{
	//Set drawing flag
	isDrawing = true;
}

void PLAYER::DrawToScreen()
{
	//Draw us
	if (isDrawing)
	{
		//Don't draw if we don't have textures or mappings
		if (texture != NULL && mappings != NULL)
		{
			//Draw our sprite
			SDL_Rect *mapRect = &mappings->rect[mappingFrame];
			SDL_Point *mapOrig = &mappings->origin[mappingFrame];
			
			int origX = mapOrig->x;
			int origY = mapOrig->y;
			if (renderFlags.xFlip)
				origX = mapRect->w - origX;
			if (renderFlags.yFlip)
				origY = mapRect->h - origY;
			
			int alignX = renderFlags.alignPlane ? gLevel->camera->x : 0;
			int alignY = renderFlags.alignPlane ? gLevel->camera->y : 0;
			texture->Draw((highPriority ? LEVEL_RENDERLAYER_OBJECT_HIGH_0 : LEVEL_RENDERLAYER_OBJECT_0) + ((OBJECT_LAYERS - 1) - priority), texture->loadedPalette, mapRect, x.pos - origX - alignX, y.pos - origY - alignY, renderFlags.xFlip, renderFlags.yFlip);
		}
	}
}

//Debug mode
void PLAYER::RestoreStateDebug()
{
	anim = PLAYERANIMATION_WALK;
	x.sub = 0;
	y.sub = 0;
	memset(&objectControl, 0, sizeof(objectControl));
	spindashing = false;
	xVel = 0;
	yVel = 0;
	inertia = 0;
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
		int32_t calcSpeed = (debugSpeed + 1) * 0x1000;
		
		if (selectedControl.up)
		{
			yPosLong -= calcSpeed;
			if (yPosLong < gLevel->topBoundaryTarget * 0x10000)
				yPosLong = gLevel->topBoundaryTarget * 0x10000;
		}
		else if (selectedControl.down)
		{
			yPosLong += calcSpeed;
			if (yPosLong > gLevel->bottomBoundaryTarget * 0x10000)
				yPosLong = gLevel->bottomBoundaryTarget * 0x10000;
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

//Object interaction functions
bool PLAYER::TouchResponseObject(void *objPointer)
{
	//Iterate through children first
	OBJECT *object = (OBJECT*)objPointer;
	for (OBJECT *obj = object->children; obj != NULL; obj = obj->next)
	{
		if (TouchResponseObject((void*)obj))
			return true;
	}
	
	int16_t playerLeft, playerTop, playerWidth, playerHeight;
	
	/* https://www.youtube.com/watch?v=32Hp1LW08Yc
		bsr.w	ShieldTouchResponse
		tst.b	character_id(a0)			; Is the player Sonic?
		bne.s	Touch_NoInstaShield			; If not, branch
		move.b	status_secondary(a0),d0
		andi.b	#$73,d0					; Does the player have any shields or is invincible?
		bne.s	Touch_NoInstaShield			; If so, branch
		; By this point, we're focussing purely on the Insta-Shield
		cmpi.b	#1,double_jump_flag(a0)			; Is the Insta-Shield currently in its 'attacking' mode?
		bne.s	Touch_NoInstaShield			; If not, branch
		move.b	status_secondary(a0),d0			; Get status_secondary...
		move.w	d0,-(sp)				; ...and save it
		bset	#Status_Invincible,status_secondary(a0)	; Make the player invincible
		move.w	x_pos(a0),d2				; Get player's x_pos
		move.w	y_pos(a0),d3				; Get player's y_pos
		subi.w	#$18,d2					; Subtract width of Insta-Shield
		subi.w	#$18,d3					; Subtract height of Insta-Shield
		move.w	#$30,d4					; Player's width
		move.w	#$30,d5					; Player's height
		bsr.s	Touch_Process
		move.w	(sp)+,d0				; Get the backed-up status_secondary
		btst	#Status_Invincible,d0			; Was the player already invincible (wait, what? An earlier check ensures that this can't happen)
		bne.s	.alreadyinvincible			; If so, branch
		bclr	#Status_Invincible,status_secondary(a0)	; Make the player vulnerable again

	.alreadyinvincible:
		moveq	#0,d0
		rts
	*/
	
	//Get player hitbox
	playerWidth = 8; //radius
	playerLeft = x.pos - playerWidth;
	playerWidth *= 2; //diameter
	
	playerHeight = yRadius - 3; //radius
	playerTop = y.pos - playerHeight;
	playerHeight *= 2; //diameter
	
	/*
	//Code to handle the ducking hitbox from Sonic 1 and 2
	if (anim == PLAYERANIMATION_DUCK)
	{
		playerTop += 12;
		playerHeight = 10;
	}
	*/
	
	//Check object
	if ((object->collisionType != COLLISIONTYPE_ENEMY && object->collisionType != COLLISIONTYPE_BOSS) || (object->touchWidth | object->touchHeight) != 0)
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
								return CheckHurt(objPointer);
							
							//If the object is above or to the side of us, hurt them
							if (((GetAtan(x.pos - object->x.pos, y.pos - object->y.pos) + 0x20) & 0xFF) < 0x40)
								return object->Hurt(this);
							
							//Hurt us
							return CheckHurt(objPointer);
						case CHARACTERTYPE_KNUCKLES:
							//If gliding or sliding after gliding, hurt the object
							if (jumpAbility == 1 || jumpAbility == 3)
								return object->Hurt(this);
							
							//Otherwise, hurt us
							return CheckHurt(objPointer);
						default:
							return CheckHurt(objPointer);
					}
					break;
				case COLLISIONTYPE_SPECIAL:
					//Yeah, no
					return true;
				case COLLISIONTYPE_HURT:
					return CheckHurt(objPointer);
				case COLLISIONTYPE_OTHER:
					if (invulnerabilityTime < 90)
						object->routine = 2;
					return true;
				case COLLISIONTYPE_MONITOR:
					return true;
			}
		}
	}
	
	//Object and player haven't made contact
	return false;
}

void PLAYER::TouchResponse()
{
	//Iterate through every object
	for (OBJECT *obj = gLevel->objectList; obj != NULL; obj = obj->next)
	{
		if (TouchResponseObject((void*)obj))
			return;
	}
}

void PLAYER::AttachToObject(void *object, bool *standingBit)
{
	//If already standing on an object, clear that object's standing bit
	if (status.shouldNotFall)
		*((bool*)interact + (size_t)standingBit) = false; //Clear the object we're standing on's standing bit
	
	//Set to stand on this object
	interact = object;
	angle = 0;
	yVel = 0;
	inertia = xVel;
	
	//Land on object if in mid-air
	if (status.inAir)
		ResetOnFloor2();
	
	status.shouldNotFall = true;
	status.inAir = false;
	*((bool*)object + (size_t)standingBit) = true;
}

void PLAYER::MoveOnPlatform(void *platform, int16_t height, int16_t lastXPos)
{
	OBJECT *platformObject = (OBJECT*)platform;
	int top = platformObject->y.pos - height;
	
	//Check if we're in an intangible state
	if (objectControl.disableObjectInteract || routine == PLAYERROUTINE_DEATH || debug != 0)
		return;
	
	//Move with the platform
	x.pos += (platformObject->x.pos - lastXPos);
	y.pos = top - yRadius;
}
