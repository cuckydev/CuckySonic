#include <stdlib.h>
#include <string.h>

#include "SpecialStage.h"
#include "Filesystem.h"
#include "Log.h"
#include "Error.h"
#include "Audio.h"

SPECIALSTAGE::SPECIALSTAGE(const char *name)
{
	//Clear memory
	memset(this, 0, sizeof(SPECIALSTAGE));
	LOG(("Loading special stage %s...\n", name));
	
	//Load the stage texture
	stageTexture = new TEXTURE("data/SpecialStage/Stage.bmp");
	if (stageTexture->fail != nullptr)
	{
		Error(fail = stageTexture->fail);
		return;
	}
	
	//Load the texture for the spheres and rings
	sphereTexture = new TEXTURE("data/SpecialStage/Spheres.bmp");
	if (sphereTexture->fail != nullptr)
	{
		Error(fail = sphereTexture->fail);
		return;
	}
	
	//Load the background texture (stage-specific)
	char *backgroundPath = AllocPath(name, ".background.bmp", nullptr);
	backgroundTexture = new TEXTURE(backgroundPath);
	delete[] backgroundPath;
	
	if (backgroundTexture->fail != nullptr)
	{
		Error(fail = backgroundTexture->fail);
		return;
	}
	
	//Open the layout file
	char *layoutPath = AllocPath(gBasePath, name, ".ssl");
	BACKEND_FILE *fp = OpenFile(layoutPath, "rb");
	delete[] layoutPath;
	
	if (fp == nullptr)
	{
		Error(fail = GetFileError());
		return;
	}
	
	//Read layout header
	width = ReadFile_BE16(fp);
	height = ReadFile_BE16(fp);
	
	layout = new uint8_t[width * height];
	if (layout == nullptr)
	{
		Error(fail = "Failed to allocate the internal stage layout");
		return;
	}
	
	//Read and update the stage's palette
	uint8_t r1 = ReadFile_Byte(fp); uint8_t g1 = ReadFile_Byte(fp); uint8_t b1 = ReadFile_Byte(fp);
	uint8_t r2 = ReadFile_Byte(fp); uint8_t g2 = ReadFile_Byte(fp); uint8_t b2 = ReadFile_Byte(fp);
	SetPaletteColour(&tile1, r1, g1, b1);
	SetPaletteColour(&tile2, r2, g2, b2);
	PalCycle();
	
	//Read the layout data
	ReadFile(fp, layout, width * height, 1);			//Actual sphere map on the stage
	playerState.direction = (ReadFile_BE16(fp)) >> 8;	//Original game sucks, read as a word into the byte's address (68000 is big-endian, so it only uses the high byte)
	playerState.xPosLong =   ReadFile_BE16(fp)  << 8;	//The original game stores the positions in the native 8.8 format, extend to 16.16
	playerState.yPosLong =   ReadFile_BE16(fp)  << 8;
	ringsLeft = ReadFile_BE16(fp);
	CloseFile(fp);
	
	//Open the perspective map file
	char *perspectivePath = AllocPath(gBasePath, "data/SpecialStage/Perspective.bin", nullptr);
	BACKEND_FILE *perspectiveFile = OpenFile(perspectivePath, "rb");
	delete[] perspectivePath;
	
	if (perspectiveFile == nullptr)
	{
		Error(fail = GetFileError());
		return;
	}
	
	//Read perspective data
	perspectiveMap = new uint8_t[GetFileSize(perspectiveFile)];
	if (perspectiveMap == nullptr)
	{
		Error(fail = "Failed to allocate memory for the perspective map");
		return;
	}
	
	ReadFile(perspectiveFile, perspectiveMap, GetFileSize(perspectiveFile), 1);
	CloseFile(perspectiveFile);
	
	//Initialize state
	rate = 0x1000;
	rateTimer = 30 * 60;
	
	//Load and play music
	AUDIO_LOCK;
	music = new MUSIC("SpecialStage", 0, 1.0f);
	music->playing = true;
	AUDIO_UNLOCK;
	
	LOG(("Success!\n"));
}

SPECIALSTAGE::~SPECIALSTAGE()
{
	//Free all data
	delete stageTexture;
	delete sphereTexture;
	delete backgroundTexture;
	delete[] layout;
	delete[] perspectiveMap;
	
	//Unload music
	AUDIO_LOCK;
	delete music;
	AUDIO_UNLOCK;
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
	
	if (frame >= 0x10)
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
	
	//Draw the stage
	int stageFrame = 0;
	if (animFrame >= 16)
		stageFrame = (animFrame - 16) + 1;
	
	RECT stageRect = {0, stageFrame * 240, stageTexture->width, 240};
	gSoftwareBuffer->DrawTexture(stageTexture, stageTexture->loadedPalette, &stageRect, SPECIALSTAGE_RENDERLAYER_STAGE, xCenter - stageTexture->width / 2, yCenter - 240 / 2, false, false);
}
