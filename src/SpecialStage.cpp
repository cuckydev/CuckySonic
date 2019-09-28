#include "SDL_rwops.h"
#include "SpecialStage.h"
#include "Path.h"
#include "Log.h"
#include "Error.h"
#include "Audio.h"

SPECIALSTAGE::SPECIALSTAGE(const char *name)
{
	//Clear memory
	memset(this, 0, sizeof(SPECIALSTAGE));
	LOG(("Loading special stage %s...\n", name));
	
	//Load the stage texture
	stageTexture = new TEXTURE(nullptr, "data/SpecialStage/Stage.bmp");
	if (stageTexture->fail != nullptr)
	{
		Error(fail = stageTexture->fail);
		return;
	}
	
	//Load the texture for the spheres and rings
	sphereTexture = new TEXTURE(nullptr, "data/SpecialStage/Spheres.bmp");
	if (sphereTexture->fail != nullptr)
	{
		Error(fail = sphereTexture->fail);
		return;
	}
	
	//Load the background texture (stage-specific)
	GET_APPEND_PATH(backTexture, name, ".background.bmp");
	backgroundTexture = new TEXTURE(nullptr, backTexture);
	if (backgroundTexture->fail != nullptr)
	{
		Error(fail = backgroundTexture->fail);
		return;
	}
	
	//Open the layout file
	GET_APPEND_PATH(layoutName, name, ".ssl");
	GET_GLOBAL_PATH(layoutPath, layoutName);
	
	SDL_RWops *fp = SDL_RWFromFile(layoutPath, "rb");
	if (fp == nullptr)
	{
		Error(fail = SDL_GetError());
		return;
	}
	
	//Read layout header
	width = SDL_ReadBE16(fp);
	height = SDL_ReadBE16(fp);
	
	layout = (uint8_t*)malloc(width * height);
	if (layout == nullptr)
	{
		Error(fail = "Failed to allocate the internal stage layout");
		return;
	}
	
	//Read and update the stage's palette
	uint8_t r1 = SDL_ReadU8(fp); uint8_t g1 = SDL_ReadU8(fp); uint8_t b1 = SDL_ReadU8(fp);
	uint8_t r2 = SDL_ReadU8(fp); uint8_t g2 = SDL_ReadU8(fp); uint8_t b2 = SDL_ReadU8(fp);
	SetPaletteColour(&tile1, r1, g1, b1);
	SetPaletteColour(&tile2, r2, g2, b2);
	PalCycle();
	
	//Read the layout data
	SDL_RWread(fp, layout, width * height, 1);			//Actual sphere map on the stage
	playerState.direction = (SDL_ReadBE16(fp)) >> 8;	//Original game sucks, read as a word into the byte's address (68000 is big-endian, so it only uses the high byte)
	playerState.xPosLong =   SDL_ReadBE16(fp)  << 8;	//The original game stores the positions in the native 8.8 format, extend to 16.16
	playerState.yPosLong =   SDL_ReadBE16(fp)  << 8;
	spheresLeft = SDL_ReadBE16(fp);
	SDL_RWclose(fp);
	
	//Open the perspective map file
	GET_GLOBAL_PATH(perspectivePath, "data/SpecialStage/Perspective.bin");
	
	SDL_RWops *perspectiveFile = SDL_RWFromFile(perspectivePath, "rb");
	if (perspectiveFile == nullptr)
	{
		Error(fail = SDL_GetError());
		return;
	}
	
	//Read perspective data
	perspectiveMap = (uint8_t*)malloc(SDL_RWsize(perspectiveFile));
	if (perspectiveMap == nullptr)
	{
		Error(fail = "Failed to allocate memory for the perspective map");
		return;
	}
	
	SDL_RWread(perspectiveFile, perspectiveMap, SDL_RWsize(perspectiveFile), 1);
	SDL_RWclose(perspectiveFile);
	
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
	free(layout);
	free(perspectiveMap);
}

//Stage update code
void SPECIALSTAGE::Update()
{
	
}

//Stage drawing code
void SPECIALSTAGE::PalCycle()
{
	uint16_t frame = animFrame;
	
	//Handle a different palette frame when turning
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
	
	static int a = 0;
	if (a++ & 0x1)
		animFrame = (animFrame + 1) & 0xF;
	backY += 3;
	
	//Update the stage's palette cycle
	PalCycle();
	
	//Draw the stage
	int stageFrame = 0;
	if (animFrame >= 16)
		stageFrame = (animFrame - 16) + 1;
	
	SDL_Rect stageRect = {0, stageFrame * 240, stageTexture->width, 240};
	gSoftwareBuffer->DrawTexture(stageTexture, stageTexture->loadedPalette, &stageRect, SPECIALSTAGE_RENDERLAYER_STAGE, xCenter - stageTexture->width / 2, yCenter - 240 / 2, false, false);
}
