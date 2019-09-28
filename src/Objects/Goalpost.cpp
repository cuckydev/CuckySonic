#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"
#include "../Objects.h"

static const uint8_t animationEggman[] =	{0x0F,0x00,0xFF};
static const uint8_t animationSpin1[] =		{0x01,0x00,0x01,0x02,0x03,0xFF};
static const uint8_t animationSpin2[] =		{0x01,0x04,0x01,0x02,0x03,0xFF};
static const uint8_t animationSonic[] =		{0x0F,0x04,0xFF};

static const uint8_t *animationList[] = {
	animationEggman,
	animationSpin1,
	animationSpin2,
	animationSonic,
};

static const int8_t goalpostSparklePos[8][2] = {
	{-24, -10},
	{  8,   8},
	{-16,   0},
	{ 24,  -8},
	{  0,  -8},
	{ 16,   0},
	{-24,   8},
	{ 24,  16},
};

void ObjGoalpost(OBJECT *object)
{
	enum SCRATCH
	{
		//U8
		SCRATCHU8_SPARKLE = 0,
		//S16
		SCRATCHS16_SPIN_TIME = 0,
		SCRATCHS16_SPARKLE_TIME = 1,
	};
	
	switch (object->routine)
	{
		case 0:
		{
			//Advance routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Goalpost.bmp");
			object->mappings = gLevel->GetObjectMappings("data/Object/Goalpost.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 0x18;
			object->priority = 4;
		}
//Fallthrough
		case 1: //Check for contact
		{
			//If near the end of the level, lock screen
			int16_t boundary = gLevel->rightBoundaryTarget - gRenderSpec.width - 0x100;
			if (gLevel->camera->x >= boundary)
				gLevel->leftBoundaryTarget = boundary;
			
			//If the main player is near us, start spinning
			if (gLevel->playerList->x.pos >= object->x.pos && gLevel->playerList->x.pos < (object->x.pos + 0x20))
			{
				//Lock the camera, timer, and increment routine
				gLevel->leftBoundaryTarget = gLevel->rightBoundaryTarget - gRenderSpec.width;
				gLevel->updateTime = false;
				
				//Play sound and increment routine
				PlaySound(SOUNDID_GOALPOST_SPIN);
				object->routine++;
			}
			break;
		}
		case 2: //Touched and spinning
		{
			//Handle our spin timer
			if (--object->scratchS16[SCRATCHS16_SPIN_TIME] < 0)
			{
				object->scratchS16[SCRATCHS16_SPIN_TIME] = 60;
				
				//Increment spin cycle, and if reached end...
				if (++object->anim >= 3)
					object->routine++;
			}
			
			//Handle our sparkling
			if (--object->scratchS16[SCRATCHS16_SPARKLE_TIME] < 0)
			{
				object->scratchS16[SCRATCHS16_SPARKLE_TIME] = 11;
				
				//Increment our sparkle index
				object->scratchU8[SCRATCHU8_SPARKLE]++;
				object->scratchU8[SCRATCHU8_SPARKLE] %= 8;
				
				//Create a sparkle object
				OBJECT *sparkle = new OBJECT(&gLevel->objectList, &ObjRing);
				sparkle->routine = 3;
				sparkle->x.pos = object->x.pos + goalpostSparklePos[object->scratchU8[SCRATCHU8_SPARKLE]][0];
				sparkle->y.pos = object->y.pos + goalpostSparklePos[object->scratchU8[SCRATCHU8_SPARKLE]][1];
				
				//Initialize sparkle's rendering stuff
				sparkle->texture = gLevel->GetObjectTexture("data/Object/Ring.bmp");
				sparkle->mappings = gLevel->GetObjectMappings("data/Object/Ring.map");
				sparkle->renderFlags.alignPlane = true;
				sparkle->widthPixels = 8;
			}
			break;
		}
		case 3: //Make players run to the right of the screen
		{
			for (PLAYER *player = gLevel->playerList; player != nullptr; player = player->next)
			{
				//Skip if debug is enabled
				if (player->debug)
					continue;
				
				//Lock controls
				if (player->status.inAir == false)
				{
					player->controlLock = true;
					player->controlHeld = {false, false, false, false, true, false, false, false};
					player->controlPress = {false, false, false, false, true, false, false, false};
				}
				
				//If the main player, and near the right of the screen, increment routine
				if (player == gLevel->playerList && player->x.pos >= gLevel->rightBoundaryTarget)
					object->routine++;
			}
			break;
		}
		case 4: //End of level
		{
			//TEMP: Load next level
			gLevel->SetFade(false, false);
			gGameLoadLevel++;
			gGameLoadLevel %= LEVELID_MAX;
			break;
		}
	}
	
	//Animate and draw sprite
	object->Animate(animationList);
	object->Draw();
}
