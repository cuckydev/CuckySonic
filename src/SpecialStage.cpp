#include <stdlib.h>
#include <string.h>

#include "SpecialStage.h"
#include "Filesystem.h"
#include "Log.h"
#include "Error.h"
#include "Audio.h"
#include "Input.h"
#include "MathUtil.h"

#define SPEEDUP_TIME (30 * 60)

//Special stage constructor and destructor
SPECIALSTAGE::SPECIALSTAGE(std::string name)
{
	LOG(("Loading special stage %s...\n", name.c_str()));
	
	//Load the stage texture
	stageTexture = new TEXTURE("data/SpecialStage/Stage.bmp");
	if (stageTexture->fail)
	{
		Error(fail = stageTexture->fail);
		return;
	}
	
	//Load the texture for the spheres and rings
	sphereTexture = new TEXTURE("data/SpecialStage/Spheres.bmp");
	if (sphereTexture->fail)
	{
		Error(fail = sphereTexture->fail);
		return;
	}
	
	//Load the background texture (stage-specific)
	backgroundTexture = new TEXTURE(name + ".background.bmp");
	if (backgroundTexture->fail)
	{
		Error(fail = backgroundTexture->fail);
		return;
	}
	
	//Open the layout file
	FS_FILE fp(gBasePath + name + ".ssl", "rb");
	if (fp.fail)
	{
		Error(fail = fp.fail);
		return;
	}
	
	//Read layout header
	width = fp.ReadBE16();
	height = fp.ReadBE16();
	
	layout = new uint8_t[width * height];
	if (layout == nullptr)
	{
		Error(fail = "Failed to allocate the internal stage layout");
		return;
	}
	
	//Read and update the stage's palette
	uint8_t r1 = fp.ReadU8(); uint8_t g1 = fp.ReadU8(); uint8_t b1 = fp.ReadU8();
	uint8_t r2 = fp.ReadU8(); uint8_t g2 = fp.ReadU8(); uint8_t b2 = fp.ReadU8();
	tile1.SetColour(true, true, true, r1, g1, b1);
	tile2.SetColour(true, true, true, r2, g2, b2);
	RotatePalette();
	
	//Read the layout data
	fp.Read(layout, width * height, 1);		//Actual sphere map on the stage
	player.angle = (fp.ReadBE16()) >> 8;	//Original game sucks, read as a word into the byte's address (68000 is big-endian, so it only uses the high byte)
	player.xLong =   fp.ReadBE16();
	player.yLong =   fp.ReadBE16();
	ringsLeft = fp.ReadBE16();
	
	//Initialize state
	rate = 0x1000;
	rateTimer = SPEEDUP_TIME;
	
	LOG(("Success!\n"));
}

SPECIALSTAGE::~SPECIALSTAGE()
{
	//Free all data
	delete stageTexture;
	delete sphereTexture;
	delete backgroundTexture;
	delete[] layout;
}

//Stage update code
void SPECIALSTAGE::SpeedupStage()
{
	//Speed up every 30 seconds
	if (rateTimer == 0 || --rateTimer == 0)
	{
		//Reset timer and speed up
		rateTimer = SPEEDUP_TIME;
		if (rate != 0x2000)
			rate += 0x400;
	}
}
void SPECIALSTAGE::MovePlayer()
{
	//Handle player movement
	uint16_t movingPosition = (player.angle & 0x40) ? player.xLong : player.yLong;
	
	//Update turning if not touched a spring
	if (player.jumping < 0x80)
	{
		if (player.turn != 0 && (movingPosition & 0xE0) == 0)
		{
			//Turn and check if we're finished (facing in a cardinal direction)
			player.angle += player.turn;
			if ((player.angle & 0x3F) != 0)
				return;
			
			//Stop turning
			player.turn = 0;
			if (player.velocity != 0)
				player.turnLock = true;
		}
		
		//Stop turn lock
		if ((movingPosition & 0xE0) != 0)
			player.turnLock = false;
	}
	
	//Handle movement conditions
	if (player.clearRoutine == 0)
	{
		if (player.bumperLock == false)
		{
			//Start moving if up is pressed
			if (gController[0].held.up)
			{
				player.advancing = true;
				player.started = true;
			}
			
			//Acceleration
			int16_t nextVel = player.velocity;
			if (player.started == true)
			{
				if (player.advancing == false || player.velocity < 0)
				{
					nextVel -= 0x200;
					if (nextVel <= -rate)
						nextVel = rate;
				}
				else
				{
					nextVel += 0x200;
					if (nextVel >= rate)
						nextVel = rate;
				}
			}
			
			//Check if we should turn
			if (player.turnLock == false)
			{
				if (gController[0].held.left)
					player.turn =  4;
				if (gController[0].held.right)
					player.turn = -4;
			}
			
			//Apply our new velocity
			player.velocity = nextVel;
			
			if (player.bumperLock == true)
			{
				
			}
			else
			{
				//Move twice as fast if hit a spring
				if (player.jumping == 0x81)
					nextVel *= 2;
				
				//Apply our velocity onto our position
				int32_t sin = GetSin(player.angle) * nextVel; sin = ((sin & 0xFFFF0000) >> 16) | ((sin & 0x0000FFFF) << 16);
				int32_t cos = GetCos(player.angle) * nextVel; cos = ((cos & 0xFFFF0000) >> 16) | ((cos & 0x0000FFFF) << 16);
				player.xLong -= sin;
				player.yLong -= cos;
			}
		}
	}
}

void SPECIALSTAGE::Update()
{
	//Update stage and players
	SpeedupStage();
	MovePlayer();
}

//Stage drawing code
static const uint8_t ssPalCycleMap[] =	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
										 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
void SPECIALSTAGE::RotatePalette()
{
	//Get our frame to index into the cycle
	uint16_t frame = animFrame;
	if (frame >= 0x20)
		frame = paletteFrame;
	frame = 0x20 - (frame & 0x1F);
	
	//Copy our palette
	const uint8_t *mapIndex = ssPalCycleMap + frame;
	for (int i = 0; i < 0x20; i++)
		stageTexture->loadedPalette->colour[1 + i] = (*mapIndex++) ? tile2 : tile1;
}

void SPECIALSTAGE::UpdateStageFrame()
{
	//Get our moving position
	int16_t movingPosition = 0;
	if (player.angle & 0x40)
		movingPosition = player.xLong + 0x100 + (player.yLong & 0x100);
	else
		movingPosition = player.yLong + (player.xLong & 0x100);
	
	//Reverse if angle is positive
	if (player.angle < 0x80)
		movingPosition = 0xF - movingPosition;
	
	//Get our animation frame from our moving position
	movingPosition = (movingPosition & 0x1F0) >> 4;
	animFrame = movingPosition;
	paletteFrame = movingPosition;
	
	//Use turning frames if turning
	uint8_t midTurn = player.angle & 0x3C;
	if (midTurn != 0)
	{
		animFrame = (midTurn >> 2) + 0x1F;
		paletteFrame &= 0x1E;
	}
}

void SPECIALSTAGE::UpdateBackgroundPosition()
{
	//Get our movement delta
	int16_t move;
	if (player.angle & 0x40)
		move = player.xLong - player.lastXLong;
	else
		move = player.yLong - player.lastYLong;
	
	//Apply movement delta and angle to background position
	if (player.angle >= 0x80)
		backY -= move >> 2;
	else
		backY += move >> 2;
	backX = player.angle << 2;
	
	//Remember last position
	player.lastXLong = player.xLong;
	player.lastYLong = player.yLong;
}

static const int ssStageMap[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

void SPECIALSTAGE::Draw()
{
	//Get origin position to draw from
	const int xCenter = gRenderSpec.width / 2;
	const int yCenter = gRenderSpec.height / 2;
	
	//Update stage frame
	UpdateStageFrame();
	
	//Draw and update the background
	UpdateBackgroundPosition();
	for (int x = -(-backX % (unsigned)backgroundTexture->width); x < gRenderSpec.width; x += backgroundTexture->width)
		for (int y = -(-backY % (unsigned)backgroundTexture->height); y < gRenderSpec.height; y += backgroundTexture->height)
			gSoftwareBuffer->DrawTexture(backgroundTexture, backgroundTexture->loadedPalette, nullptr, SPECIALSTAGE_RENDERLAYER_BACKGROUND, x, y, false, false);
	
	//Draw the stage (first 16 are just a palette cycle using the first frame, next 8 are just turning animation)
	RotatePalette();
	
	RECT stageRect = {0, ssStageMap[animFrame] * 240, stageTexture->width, 240};
	gSoftwareBuffer->DrawTexture(stageTexture, stageTexture->loadedPalette, &stageRect, SPECIALSTAGE_RENDERLAYER_STAGE, xCenter - stageTexture->width / 2, yCenter - 240 / 2, false, false);
	
	for (int x = 0; x < 0x20; x++)
	{
		POINT point = {x, 0};
		gSoftwareBuffer->DrawPoint(0, &point, &stageTexture->loadedPalette->colour[1 + x]);
	}
}
