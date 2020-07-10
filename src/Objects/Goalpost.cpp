#include "../Game.h"
#include "../Objects.h"

static const uint8_t animationEggman[] =	{0x0F,0x00,ANICOMMAND_RESTART};
static const uint8_t animationSpin1[] =		{0x01,0x00,0x01,0x02,0x03,ANICOMMAND_RESTART};
static const uint8_t animationSpin2[] =		{0x01,0x04,0x01,0x02,0x03,ANICOMMAND_RESTART};
static const uint8_t animationSonic[] =		{0x0F,0x04,ANICOMMAND_RESTART};

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
	//Define and allocate our scratch
	struct SCRATCH
	{
		uint8_t sparkle = 0;
		int16_t spinTime = 0;
		int16_t sparkleTime = 0;
	};
	
	SCRATCH *scratch = object->Scratch<SCRATCH>();
	
	switch (object->routine)
	{
		case 0:
		{
			//Advance routine
			object->routine++;
			
			//Load graphics
			object->texture = gLevel->GetObjectTexture("data/Object/Generic.bmp");
			object->mapping.mappings = gLevel->GetObjectMappings("data/Object/Goalpost.map");
			
			//Initialize other properties
			object->renderFlags.alignPlane = true;
			object->widthPixels = 24;
			object->heightPixels = 32;
			object->priority = 4;
		}
//Fallthrough
		case 1: //Check for contact
		{
			//If near the end of the level, lock screen
			int16_t boundary = gLevel->rightBoundaryTarget - gRenderSpec.width - 0x100;
			if (gLevel->camera->xPos >= boundary)
				gLevel->leftBoundaryTarget = boundary;
			
			//If the main player is near us, start spinning
			PLAYER *player = gLevel->playerList[0];
			
			if (player->x.pos >= object->x.pos && player->x.pos < (object->x.pos + 32))
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
			if (--scratch->spinTime < 0)
			{
				scratch->spinTime = 60;
				
				//Increment spin cycle, and if reached end...
				if (++object->anim >= 3)
					object->routine++;
			}
			
			//Handle our sparkling
			if (--scratch->sparkleTime < 0)
			{
				scratch->sparkleTime = 11;
				
				//Increment our sparkle index
				scratch->sparkle++;
				scratch->sparkle %= 8;
				
				//Create a sparkle object
				OBJECT *sparkle = new OBJECT(&ObjRing);
				sparkle->anim = 1;
				sparkle->x.pos = object->x.pos + goalpostSparklePos[scratch->sparkle][0];
				sparkle->y.pos = object->y.pos + goalpostSparklePos[scratch->sparkle][1];
				gLevel->objectList.link_back(sparkle);
			}
			break;
		}
		case 3: //Make players run to the right of the screen
		{
			for (size_t i = 0; i < gLevel->playerList.size(); i++)
			{
				//Get the player
				PLAYER *player = gLevel->playerList[i];
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
				if (i == 0 && player->x.pos >= gLevel->rightBoundaryTarget)
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
	object->DrawInstance(object->renderFlags, object->texture, object->mapping, object->highPriority, object->priority, object->mappingFrame, object->x.pos, object->y.pos);
}
