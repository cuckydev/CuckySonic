#include <stdlib.h>
#include <string.h>

#include "SpecialStage.h"
#include "Filesystem.h"
#include "Log.h"
#include "Error.h"
#include "Audio.h"

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
	FS_FILE *fp = new FS_FILE(gBasePath + name + ".ssl", "rb");
	if (fp->fail)
	{
		Error(fail = fp->fail);
		delete fp;
		return;
	}
	
	//Read layout header
	width = fp->ReadBE16();
	height = fp->ReadBE16();
	
	layout = new uint8_t[width * height];
	if (layout == nullptr)
	{
		Error(fail = "Failed to allocate the internal stage layout");
		delete fp;
		return;
	}
	
	//Read and update the stage's palette
	uint8_t r1 = fp->ReadU8(); uint8_t g1 = fp->ReadU8(); uint8_t b1 = fp->ReadU8();
	uint8_t r2 = fp->ReadU8(); uint8_t g2 = fp->ReadU8(); uint8_t b2 = fp->ReadU8();
	tile1.SetColour(true, true, true, r1, g1, b1);
	tile2.SetColour(true, true, true, r2, g2, b2);
	PalCycle();
	
	//Read the layout data
	fp->Read(layout, width * height, 1);			//Actual sphere map on the stage
	playerState.direction = (fp->ReadBE16()) >> 8;	//Original game sucks, read as a word into the byte's address (68000 is big-endian, so it only uses the high byte)
	playerState.xPosLong =   fp->ReadBE16()  << 8;	//The original game stores the positions in the native 8.8 format, extend to 16.16
	playerState.yPosLong =   fp->ReadBE16()  << 8;
	ringsLeft = fp->ReadBE16();
	delete fp;
	
	//Initialize state
	rate = 0x1000;
	rateTimer = 30 * 60;
	
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
void SPECIALSTAGE::Update()
{
	
}

//Stage drawing code
void SPECIALSTAGE::PalCycle()
{
	//Handle a different palette frame when turning
	uint16_t frame = animFrame;
	
	if (frame >= 16)
	{
		if (playerState.turn >= 0)
			return;
		else
			frame = (paletteFrame & 0xF);
	}
	
	//Get the base tile and strip
	int stripsLeft = animFrame & 0x7;			//How many strips to draw using the below, 0-7
	int alternateTile = (animFrame & 0x8) == 0;	//Set if drawing tile2, cleared if drawing tile1
	
	//Update all our strips
	int strip = 0;
	while (strip < 16)
	{
		//Draw this tile's strips
		while (stripsLeft-- > 0 && strip < 16)
			stageTexture->loadedPalette->colour[++strip] = (alternateTile ? tile2 : tile1);
		
		//Alternate between tiles
		stripsLeft = 8;
		alternateTile ^= 1;
	}
}

void SPECIALSTAGE::Draw()
{
	//Get origin position to draw from
	const int xCenter = gRenderSpec.width / 2;
	const int yCenter = gRenderSpec.height / 2;
	
	//Draw the background, given our scroll values
	for (int x = -(backX % backgroundTexture->width); x < gRenderSpec.width; x += backgroundTexture->width)
		for (int y = -(backY % backgroundTexture->height); y < gRenderSpec.height; y += backgroundTexture->height)
			gSoftwareBuffer->DrawTexture(backgroundTexture, backgroundTexture->loadedPalette, nullptr, SPECIALSTAGE_RENDERLAYER_BACKGROUND, x, y, false, false);
	
	//Draw the stage (first 16 are just a palette cycle using the first frame, next 8 are just turning animation)
	int stageFrame = 0;
	if (animFrame >= 16)
		stageFrame = animFrame - 16;
	
	RECT stageRect = {0, stageFrame * 240, stageTexture->width, 240};
	gSoftwareBuffer->DrawTexture(stageTexture, stageTexture->loadedPalette, &stageRect, SPECIALSTAGE_RENDERLAYER_STAGE, xCenter - stageTexture->width / 2, yCenter - 240 / 2, false, false);
}
