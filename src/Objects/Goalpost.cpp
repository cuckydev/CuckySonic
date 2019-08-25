#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"

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

void ObjGoalpost(OBJECT *object)
{
	enum SCRATCH
	{
		SCRATCH_SPIN_TIME = 0, // 2 bytes
		SCRATCH_SPARKLE_TIME = 1, // 2 bytes
	};
	
	switch (object->routine)
	{
		case 0:
			//Advance routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Goalpost.bmp");
			object->mappings = new MAPPINGS("data/Object/Goalpost.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 0x18;
			object->priority = 4;
//Fallthrough
		case 1: //Check for contact
			//If the main player is near us, start spinning
			if (gLevel->playerList->x.pos >= object->x.pos && gLevel->playerList->x.pos < (object->x.pos + 0x20))
			{
				//Lock the camera
				gLevel->leftBoundary2 = gLevel->rightBoundary2 - SCREEN_WIDTH;
				
				//Increment routine
				object->routine++;
			}
			break;
		case 2: //Touched and spinning
			//Handle our spin timer
			if (--object->scratchS8[SCRATCH_SPIN_TIME] < 0)
			{
				object->scratchS8[SCRATCH_SPIN_TIME] = 60;
				
				//Increment spin cycle, and if reached end...
				if (++object->anim >= 3)
					object->routine++;
			}
			break;
		case 3: //Make players run to the right of the screen
			for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
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
				if (player == gLevel->playerList && player->x.pos >= gLevel->rightBoundary2)
					object->routine++;
			}
			break;
		case 4: //End of level
			//TEMP: Load next level
			gLevel->SetFade(false, false);
			gGameLoadLevel++;
			gGameLoadLevel %= 2;
			break;
	}
	
	//Animate sprite
	object->Animate(animationList);
}
