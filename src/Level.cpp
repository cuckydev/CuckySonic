#include <stdlib.h>
#include <string.h>

#include "Filesystem.h"
#include "Audio.h"
#include "Level.h"
#include "MathUtil.h"
#include "Game.h"
#include "Fade.h"
#include "Error.h"
#include "Log.h"

//Object function lists
#include "Objects.h"

OBJECTFUNCTION objFuncSonic1[] = {
	nullptr, nullptr, nullptr, &ObjPathSwitcher, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, &ObjGoalpost, nullptr, nullptr,
	nullptr, &ObjBridge, nullptr, nullptr, nullptr, &ObjGHZSwingingPlatform, nullptr, &ObjGHZSpikeLog,
	&ObjGHZPlatform, nullptr, &ObjGHZLedge, nullptr, &ObjSonic1Scenery, nullptr, nullptr, &ObjCrabmeat,
	nullptr, nullptr, &ObjBuzzBomber, nullptr, nullptr, &ObjRingSpawner, &ObjMonitor, nullptr,
	nullptr, nullptr, nullptr, &ObjChopper, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &ObjGHZSpikes, nullptr,
	nullptr, nullptr, nullptr, &ObjGHZPurpleRock, &ObjGHZSmashableWall, nullptr, nullptr, nullptr,
	&ObjMotobug, &ObjSpring, &ObjNewtron, nullptr, &ObjGHZEdgeWall, nullptr, nullptr, nullptr,
	nullptr, &ObjGHZWaterfallSound, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

OBJECTFUNCTION objFuncSonic2[] = {
	nullptr, nullptr, nullptr, &ObjPathSwitcher, nullptr, nullptr, &ObjSpiral, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, &ObjGoalpost, nullptr, nullptr,
	nullptr, &ObjBridge, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &ObjMonitor, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, &ObjSpring, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

//Preload lists - Generic
std::string preloadTexture[] = {
	//Stage objects
	"data/Object/Generic.bmp",
	//Player objects / effects
	"data/Object/PlayerGeneric.bmp",
	"",
};

std::string preloadMappings[] = {
	//Stage objects
	"data/Object/Ring.map",
	"data/Object/Monitor.map",
	"data/Object/MonitorContents.map",
	"data/Object/YellowSpring.map",
	"data/Object/RedSpring.map",
	"data/Object/Goalpost.map",
	"data/Object/Explosion.map",
	"data/Object/Score.map",
	
	//Player objects
	"data/Object/SpindashDust.map",
	"data/Object/SkidDust.map",
	"data/Object/DropdashDust.map",
	"data/Object/InvincibilityStars.map",
	"data/Object/SuperStars.map",
	"data/Object/DoubleSpinAttack.map",
	"data/Object/BlueBarrier.map",
	"data/Object/FlameBarrier.map",
	"data/Object/LightningBarrier.map",
	"data/Object/AquaBarrier.map",
	"",
};

//Preload lists - Green Hill Zone
std::string preloadTexture_GHZ[] = {
	//Stage objects
	"data/Object/GHZGeneric.bmp",
	//Badniks
	"data/Object/Sonic1Badnik.bmp",
	"",
};

std::string preloadMappings_GHZ[] = {
	//Stage objects
	"data/Object/GHZBridge.map",
	"data/Object/GHZPlatform.map",
	"data/Object/GHZLedge.map",
	"data/Object/GHZSwingingPlatform.map",
	"data/Object/GHZSpikes.map",
	"data/Object/GHZEdgeWall.map",
	"data/Object/GHZSmashableWall.map",
	"data/Object/GHZSpikeLog.map",
	"data/Object/GHZPurpleRock.map",
	//Badniks
	"data/Object/BuzzBomber.map",
	"data/Object/Motobug.map",
	"data/Object/Crabmeat.map",
	"data/Object/Chopper.map",
	"data/Object/NewtronBlue.map",
	"data/Object/NewtronGreen.map",
	"data/Object/Missile.map",
	"",
};

//Preload lists - Emerald Hill Zone
std::string preloadTexture_EHZ[] = {
	//Stage objects
	"data/Object/EHZGeneric.bmp",
	"",
};

std::string preloadMappings_EHZ[] = {
	//Stage objects
	"data/Object/EHZBridge.map",
	"",
};

//Our level table
LEVELTABLE gLevelTable[LEVELID_MAX] = {
	//ZONEID_GHZ
		/*LEVELID_GHZ1*/ {ZONEID_GHZ, "Green Hill Zone", "Act 1",
							LEVELFORMAT_CHUNK128, OBJECTFORMAT_SONIC1, ARTFORMAT_BMP,
							"data/Level/GHZ/ghz1", "data/Level/GHZ/ghz", "data/Level/sonic1", "data/Level/GHZ/ghz", "GHZ",
							preloadTexture_GHZ, preloadMappings_GHZ, &GHZ_Background, &GHZ_PaletteCycle, objFuncSonic1,
							0x0050, 0x03B0, 0x0000, 0x44CB, 0x0000, 0x03E0},
		/*LEVELID_GHZ2*/ {ZONEID_GHZ, "Green Hill Zone", "Act 2",
							LEVELFORMAT_CHUNK128, OBJECTFORMAT_SONIC1, ARTFORMAT_BMP,
							"data/Level/GHZ/ghz2", "data/Level/GHZ/ghz", "data/Level/sonic1", "data/Level/GHZ/ghz", "GHZ",
							preloadTexture_GHZ, preloadMappings_GHZ, &GHZ_Background, &GHZ_PaletteCycle, objFuncSonic1,
							0x0050, 0x03B0, 0x0000, 0x3A40, 0x0000, 0x03E0},
	
	//ZONEID_EHZ
		/*LEVELID_EHZ1*/ {ZONEID_EHZ, "Emerald Hill Zone", "Act 1",
							LEVELFORMAT_CHUNK128, OBJECTFORMAT_SONIC2, ARTFORMAT_BMP,
							"data/Level/EHZ/ehz1", "data/Level/EHZ/ehz", "data/Level/sonic2", "data/Level/EHZ/ehz", "EHZ",
							preloadTexture_EHZ, preloadMappings_EHZ, &EHZ_Background, &EHZ_PaletteCycle, objFuncSonic2,
							0x0060, 0x028F, 0x0000, 0x2A40, 0x0000, 0x0400},
		/*LEVELID_EHZ2*/ {ZONEID_EHZ, "Emerald Hill Zone", "Act 2",
							LEVELFORMAT_CHUNK128, OBJECTFORMAT_SONIC2, ARTFORMAT_BMP,
							"data/Level/EHZ/ehz2", "data/Level/EHZ/ehz", "data/Level/sonic2", "data/Level/EHZ/ehz", "EHZ",
							preloadTexture_EHZ, preloadMappings_EHZ, &EHZ_Background, &EHZ_PaletteCycle, objFuncSonic2,
							0x0060, 0x028F, 0x0000, 0x29E0, 0x0000, 0x0500},
};

//Loading functions
bool LEVEL::LoadMappings(LEVELTABLE *tableEntry)
{
	LOG(("Loading mappings... "));
	
	//Load chunk mappings
	switch (tableEntry->format)
	{
		case LEVELFORMAT_CHUNK128:
		{
			//Open our chunk mapping file
			FS_FILE mappingFile(gBasePath + tableEntry->chunkTileReferencePath + ".chk", "rb");
			if (mappingFile.fail != nullptr)
			{
				Error(fail = mappingFile.fail);
				return true;
			}
			
			//Allocate the chunk mappings in memory
			chunks = (mappingFile.GetSize() / 2 / (8 * 8));
			chunkMapping = new CHUNKMAPPING[chunks];
			
			if (chunkMapping == nullptr)
			{
				Error(fail = "Failed to allocate chunk mappings in memory");
				return true;
			}
			
			//Read the mapping data
			for (size_t i = 0; i < chunks; i++)
			{
				for (int v = 0; v < (8 * 8); v++)
				{
					uint16_t tmap = mappingFile.ReadBE16();
					chunkMapping[i].tile[v].altLRB	= (tmap & 0x8000) != 0;
					chunkMapping[i].tile[v].altTop	= (tmap & 0x4000) != 0;
					chunkMapping[i].tile[v].norLRB	= (tmap & 0x2000) != 0;
					chunkMapping[i].tile[v].norTop	= (tmap & 0x1000) != 0;
					chunkMapping[i].tile[v].yFlip	= (tmap & 0x0800) != 0;
					chunkMapping[i].tile[v].xFlip	= (tmap & 0x0400) != 0;
					chunkMapping[i].tile[v].tile	= (tmap & 0x3FF);
					chunkMapping[i].tile[v].srcChunk = i;
				}
			}
			break;
		}
		
		default:
			LOG(("Level format %d doesn't use chunk mappings\n", tableEntry->format));
			break;
	}
	
	LOG(("Success!\n"));
	return false;
}

bool LEVEL::LoadLayout(LEVELTABLE *tableEntry)
{
	LOG(("Loading layout... "));
	
	//Open our layout file
	FS_FILE layoutFile(gBasePath + tableEntry->levelReferencePath + ".lay", "rb");
	if (layoutFile.fail != nullptr)
	{
		Error(fail = layoutFile.fail);
		return true;
	}
	
	//Read our layout file
	switch (tableEntry->format)
	{
		case LEVELFORMAT_CHUNK128:
			//Get our level dimensions (upscaled to tiles)
			layout.width = layoutFile.ReadBE16() * 8;
			layout.height = layoutFile.ReadBE16() * 8;
			
			//Allocate our layout
			layout.foreground = new TILE[layout.width * layout.height];
			if (layout.foreground == nullptr)
			{
				Error(fail = "Failed to allocate layout in memory");
				return true;
			}
			
			//Read our layout file and convert to tiles
			for (size_t cy = 0; cy < layout.height; cy += 8)
			{
				//Read foreground line
				for (size_t cx = 0; cx < layout.width; cx += 8)
				{
					//Read our chunks as their 8x8 tiles
					uint8_t chunk = layoutFile.ReadU8();
					for (size_t tv = 0; tv < 8 * 8; tv++)
						layout.foreground[(cy + (tv / 8)) * layout.width + (cx + (tv % 8))] = chunkMapping[chunk].tile[tv];
				}
			}
			break;
		case LEVELFORMAT_TILE16:
			//Get our level dimensions
			layout.width = layoutFile.ReadBE32();
			layout.height = layoutFile.ReadBE32();
			
			//Allocate our layout
			layout.foreground = new TILE[layout.width * layout.height];
			if (layout.foreground == nullptr)
			{
				Error(fail = "Failed to allocate layout in memory");
				return true;
			}
			
			//Read our layout file
			for (size_t tv = 0; tv < layout.width * layout.height; tv++)
			{
				uint16_t tmap = layoutFile.ReadBE16();
				layout.foreground[tv].altLRB	= (tmap & 0x8000) != 0;
				layout.foreground[tv].altTop	= (tmap & 0x4000) != 0;
				layout.foreground[tv].norLRB	= (tmap & 0x2000) != 0;
				layout.foreground[tv].norTop	= (tmap & 0x1000) != 0;
				layout.foreground[tv].yFlip		= (tmap & 0x0800) != 0;
				layout.foreground[tv].xFlip		= (tmap & 0x0400) != 0;
				layout.foreground[tv].tile		= (tmap & 0x3FF);
			}
			break;
		default:
			Error(fail = "Unimplemented level format");
			return true;
	}
	
	//Initialize boundaries
	leftBoundary = tableEntry->leftBoundary;
	rightBoundary = tableEntry->rightBoundary + gRenderSpec.width / 2;
	topBoundary = tableEntry->topBoundary;
	bottomBoundary = tableEntry->bottomBoundary;
	
	leftBoundaryTarget = leftBoundary;
	rightBoundaryTarget = rightBoundary;
	topBoundaryTarget = topBoundary;
	bottomBoundaryTarget = bottomBoundary;
	
	LOG(("Success!\n"));
	return false;
}

bool LEVEL::LoadCollisionTiles(LEVELTABLE *tableEntry)
{
	LOG(("Loading collision tiles... "));
	
	//Open our tile collision map files
	FS_FILE norMapFile(gBasePath + tableEntry->chunkTileReferencePath + ".nor", "rb");
	FS_FILE altMapFile(gBasePath + tableEntry->chunkTileReferencePath + ".alt", "rb");
	
	if (norMapFile.fail != nullptr || altMapFile.fail != nullptr)
	{
		if (norMapFile.fail != nullptr)
			Error(fail = norMapFile.fail);
		if (altMapFile.fail != nullptr)
			Error(fail = altMapFile.fail);
		return true;
	}
	
	//Read our tile collision map data
	tiles = norMapFile.GetSize();
	tileMapping = new TILEMAPPING[tiles];
	
	if (altMapFile.GetSize() != tiles)
	{
		Error(fail = "Normal map and alternate map files don't match in size");
		return true;
	}
	
	for (size_t i = 0; i < tiles; i++)
	{
		tileMapping[i].normalColTile = norMapFile.ReadU8();
		tileMapping[i].alternateColTile = altMapFile.ReadU8();
	}
	
	//Open our collision tile files
	FS_FILE colNormalFile(gBasePath + tableEntry->collisionReferencePath + ".can", "rb");
	FS_FILE colRotatedFile(gBasePath + tableEntry->collisionReferencePath + ".car", "rb");
	FS_FILE colAngleFile(gBasePath + tableEntry->collisionReferencePath + ".ang", "rb");
	
	if (colNormalFile.fail != nullptr || colRotatedFile.fail != nullptr || colAngleFile.fail != nullptr)
	{
		if (colNormalFile.fail != nullptr)
			Error(fail = colNormalFile.fail);
		if (colRotatedFile.fail != nullptr)
			Error(fail = colRotatedFile.fail);
		if (colAngleFile.fail != nullptr)
			Error(fail = colAngleFile.fail);
		return true;
	}
	
	//Allocate our collision tile data in memory
	if ((colNormalFile.GetSize() != colRotatedFile.GetSize()) || (colAngleFile.GetSize() != (colNormalFile.GetSize() / 0x10)))
	{
		Error(fail = "Collision tile data file sizes don't match each-other (Are the files compressed?)");
		return true;
	}
	
	collisionTiles = colNormalFile.GetSize() / 0x10;
	collisionTile = new COLLISIONTILE[collisionTiles];
	
	if (collisionTile == nullptr)
	{
		Error(fail = "Failed to allocate collision tile data in memory");
		return true;
	}
	
	//Read our collision tile data
	for (size_t i = 0; i < collisionTiles; i++)
	{
		for (int v = 0; v < 0x10; v++)
		{
			collisionTile[i].normal[v] = colNormalFile.ReadU8();
			collisionTile[i].rotated[v] = colRotatedFile.ReadU8();
		}
		collisionTile[i].angle = colAngleFile.ReadU8();
	}
	
	LOG(("Success!\n"));
	return false;
}

bool LEVEL::LoadObjects(LEVELTABLE *tableEntry)
{
	LOG(("Loading objects... "));
	
	//Open our object file
	FS_FILE objectFile(gBasePath + tableEntry->levelReferencePath + ".obj", "rb");
	if (objectFile.fail != nullptr)
	{
		Error(fail = objectFile.fail);
		return true;
	}
	
	//Read our object data
	switch (tableEntry->objectFormat)
	{
		case OBJECTFORMAT_SONIC1:
		case OBJECTFORMAT_SONIC2:
		{
			int objects = objectFile.GetSize() / 6;
			
			for (int i = 0; i < objects; i++)
			{
				//Read data from the file
				int16_t xPos = objectFile.ReadBE16();
				int16_t word2 = objectFile.ReadBE16();
				int16_t yPos = word2 & 0x0FFF;
				
				uint8_t id = objectFile.ReadU8();
				uint8_t subtype = objectFile.ReadU8();
				
				//Read flags from word2
				bool releaseDestroyed;
				bool yFlip = (word2 & 0x8000) != 0;
				bool xFlip = (word2 & 0x4000) != 0;
				
				if (tableEntry->objectFormat == OBJECTFORMAT_SONIC1)
				{
					//Release destroyed is stored as the highest significant bit of the id in Sonic 1
					releaseDestroyed = (id & 0x80) != 0;
					id &= 0x7F;
				}
				else
				{
					//Release destroyed is stored as the highest significant bit of word2 in Sonic 2
					releaseDestroyed = (word2 & 0x8000) != 0;
				}
				
				if (tableEntry->objectFormat == OBJECTFORMAT_SONIC1)
				{
					yFlip = (word2 & 0x8000) != 0;
					xFlip = (word2 & 0x4000) != 0;
				}
				else
				{
					yFlip = (word2 & 0x4000) != 0;
					xFlip = (word2 & 0x2000) != 0;
				}
				
				//Create and link object load from data
				OBJECT_LOAD *objectLoad = new OBJECT_LOAD;
				objectLoad->function = tableEntry->objectFunctionList[id];
				objectLoad->status = {xFlip, yFlip, releaseDestroyed, false, false};
				objectLoad->xLong = xPos << 16;
				objectLoad->yLong = yPos << 16;
				objectLoad->subtype = subtype;
				
				objectLoad->loaded = nullptr;
				objectLoad->loadRange = false;
				objectLoad->specificBit = false;
				
				objectLoadList.link_back(objectLoad);
			}
		}
	}
	
	/*
	//Open external ring file
	char *ringPath = AllocPath(gBasePath, tableEntry->levelReferencePath, ".ring");
	BACKEND_FILE *ringFile = OpenFile(ringPath, "rb");
	delete[] ringPath;
	
	if (ringFile == nullptr)
	{
		Error(fail = GetFileError());
		return true;
	}
	
	//Read external ring data
	int rings = GetFileSize(ringFile) / 4;
	
	for (int i = 0; i < rings; i++)
	{
		int16_t xPos = ReadFile_BE16(ringFile);
		int16_t word2 = ReadFile_BE16(ringFile);
		int16_t yPos = word2 & 0x0FFF;
		
		int type = (word2 & 0xF000) >> 12;
		
		for (int v = 0; v <= (type & 0x7); v++)
		{
			//Create and link object load from data
			OBJECT_LOAD *objectLoad = new OBJECT_LOAD;
			objectLoad->function = &ObjRing;
			objectLoad->status = {false, false, false, false, false};
			objectLoad->xLong = xPos << 16;
			objectLoad->yLong = yPos << 16;
			objectLoad->subtype = 0;
			
			objectLoad->loaded = nullptr;
			objectLoad->loadRange = false;
			objectLoad->specificBit = false;
			
			objectLoadList.link_back(objectLoad);
			
			//Offset next position
			if (type & 0x8)
				yPos += 0x18;
			else
				xPos += 0x18;
		}
	}
	
	CloseFile(ringFile);
	*/
	LOG(("Success!\n"));
	return false;
}

bool LEVEL::LoadArt(LEVELTABLE *tableEntry)
{
	LOG(("Loading level art...\n"));
	
	//Read our art data
	switch (tableEntry->artFormat)
	{
		case ARTFORMAT_BMP:
		{
			//Load our foreground tilemap
			tileTexture = new TEXTURE(tableEntry->artReferencePath + ".tileset.bmp");
			if (tileTexture->fail != nullptr)
			{
				Error(fail = tileTexture->fail);
				return true;
			}
			break;
		}
		default:
		{
			Error(fail = "Unimplemented art format");
			return true;
		}
	}
	
	//Load background art
	background = new BACKGROUND(tableEntry->artReferencePath + ".background.bmp", tableEntry->backFunction);
	if (background->fail != nullptr)
	{
		Error(fail = background->fail);
		return true;
	}
	
	//Set palette cycle function
	paletteFunction = tableEntry->paletteFunction;
	
	LOG(("Success!\n"));
	return false;
}

//Unload data function
void LEVEL::UnloadAll()
{
	//Free memory
	delete[] layout.foreground;
	delete[] chunkMapping;
	delete[] tileMapping;
	delete[] collisionTile;
	
	//Unload textures
	if (tileTexture != nullptr)
		delete tileTexture;
	if (background != nullptr)
		delete background;
	
	//Unload players, objects, and camera
	CLEAR_INSTANCE_LINKEDLIST(playerList);
	CLEAR_INSTANCE_LINKEDLIST(objectList);
	CLEAR_INSTANCE_LINKEDLIST(coreObjectList);
	CLEAR_INSTANCE_LINKEDLIST(objectLoadList);
	
	if (camera != nullptr)
		delete camera;
	if (titleCard != nullptr)
		delete titleCard;
	if (hud != nullptr)
		delete hud;
	
	//Unload object textures and mappings
	CLEAR_INSTANCE_LINKEDLIST(objTextureCache);
	CLEAR_INSTANCE_LINKEDLIST(objMappingsCache);
}

//Level class
LEVEL::LEVEL(int id, const char *players[])
{
	LOG(("Loading level ID %d...\n", id));
	
	//Set us as the global level
	gLevel = this;
	
	//Get data from this table entry
	LEVELTABLE *tableEntry = &gLevelTable[levelId = (LEVELID)id];
	zone = tableEntry->zone;
	
	//Load data
	if (LoadMappings(tableEntry) || LoadLayout(tableEntry) || LoadCollisionTiles(tableEntry) || LoadObjects(tableEntry) || LoadArt(tableEntry))
	{
		//Unload any loaded data
		UnloadAll();
		return;
	}
	
	//Preload generic assets
	for (int i = 0; preloadTexture[i] != ""; i++)
	{
		TEXTURE *tex = GetObjectTexture(preloadTexture[i]);
		if (tex->fail != nullptr)
		{
			fail = tex->fail;
			UnloadAll();
			return;
		}
	}
			
	for (int i = 0; preloadMappings[i] != ""; i++)
	{
		MAPPINGS *map = GetObjectMappings(preloadMappings[i]);
		if (map->fail != nullptr)
		{
			fail = map->fail;
			UnloadAll();
			return;
		}
	}
	
	//Preload stage's assets
	for (int i = 0; tableEntry->preloadTexture[i] != ""; i++)
	{
		TEXTURE *tex = GetObjectTexture(tableEntry->preloadTexture[i]);
		if (tex->fail != nullptr)
		{
			fail = tex->fail;
			UnloadAll();
			return;
		}
	}
			
	for (int i = 0; tableEntry->preloadMappings[i] != ""; i++)
	{
		MAPPINGS *map = GetObjectMappings(tableEntry->preloadMappings[i]);
		if (map->fail != nullptr)
		{
			fail = map->fail;
			UnloadAll();
			return;
		}
	}
	
	//Create our players
	PLAYER *follow = nullptr;
	
	for (int i = 0; *players != nullptr; i++, players++)
	{
		//Create our player
		PLAYER *newPlayer = new PLAYER(*players, tableEntry->startX - (i * 0x20), tableEntry->startY, follow, i);
		if (newPlayer->fail != nullptr)
		{
			fail = newPlayer->fail;
			UnloadAll();
			return;
		}
		
		if (follow == nullptr)
			follow = newPlayer;
		playerList.link_back(newPlayer);
	}
	
	//Create our camera
	camera = new CAMERA(playerList[0]);
	
	//Title-card
	titleCard = new TITLECARD(tableEntry->name, tableEntry->subtitle);
	if (titleCard->fail != nullptr)
	{
		fail = titleCard->fail;
		UnloadAll();
		return;
	}
	
	//HUD
	hud = new HUD();
	if (hud->fail != nullptr)
	{
		fail = hud->fail;
		UnloadAll();
		return;
	}
	
	//Initialize oscillatory values
	OscillatoryInit();
	
	//Initialize scores
	InitializeScores();
	
	//Load objects near the player
	CheckObjectLoad();
	
	//Update stage for initialization
	ClearControllerInput();
	UpdateStage();
	
	LOG(("Success!\n"));
}

LEVEL::~LEVEL()
{
	LOG(("Unloading level... "));
	UnloadAll();
	LOG(("Success!\n"));
}

//Fading functions
void LEVEL::SetFade(bool fadeIn, bool isSpecial)
{
	//Set our fading state
	fading = true;
	isFadingIn = fadeIn;
	specialFade = isSpecial;
	
	//Set our palettes accordingly
	if (fadeIn)
	{
		void (*function)(PALETTE *palette) = (specialFade ? &FillPaletteWhite : &FillPaletteBlack);
		
		if (tileTexture != nullptr)
			function(tileTexture->loadedPalette);
		if (background != nullptr)
			function(background->texture->loadedPalette);
		for (size_t i = 0; i < objTextureCache.size(); i++)
			function(objTextureCache[i]->loadedPalette);
	}
}

bool LEVEL::UpdateFade()
{
	bool finished = true;
	
	bool (*function)(PALETTE *palette) = (isFadingIn ? (specialFade ? &PaletteFadeInFromWhite : &PaletteFadeInFromBlack) : (specialFade ? &PaletteFadeOutToWhite : &PaletteFadeOutToBlack));
	
	//Fade all palettes
	if (tileTexture != nullptr)
		finished = function(tileTexture->loadedPalette) ? finished : false;
	if (background != nullptr)
		finished = function(background->texture->loadedPalette) ? finished : false;
	for (size_t i = 0; i < objTextureCache.size(); i++)
		finished = function(objTextureCache[i]->loadedPalette) ? finished : false;
	return finished;
}

//Dynamic events
void LEVEL::DynamicEvents()
{
	//Zone specific events
	switch (zone)
	{
		case ZONEID_GHZ:
			//Handle S-Tube force rolling
			for (size_t i = 0; i < playerList.size(); i++)
			{
				//Get this player and the tile we're on
				PLAYER *player = playerList[i];
				if (player->x.pos < 0 || player->x.pos >= (int16_t)(gLevel->layout.width * 16) || player->y.pos < 0 || player->y.pos >= (int16_t)(gLevel->layout.height * 16))
					continue;
				TILE *tile = &gLevel->layout.foreground[(size_t)(player->y.pos / 16) * gLevel->layout.width + (size_t)(player->x.pos / 16)];
				
				//If this is an S-tube chunk tile, roll
				bool doRoll = false;
				
				switch (tile->srcChunk)
				{
					case 0x75: case 0x76: case 0x77: case 0x78:
					case 0x79: case 0x7A: case 0x7B: case 0x7C:
						doRoll = true;
						break;
				}
				
				if (doRoll)
				{
					if (!player->spindashForceRoll.forceRoll)
						player->PerformRoll();
					player->spindashForceRoll.forceRoll = true;
				}
				else
					player->spindashForceRoll.forceRoll = false;
			}
			break;
		default:
			break;
	}
	
	//Level specific events
	uint16_t checkX = camera->xPos + (gRenderSpec.width - 320) / 2;
	uint16_t checkY = camera->yPos + (gRenderSpec.height - 224) / 2;
	
	switch (levelId)
	{
		case LEVELID_GHZ1: //Green Hill Zone Act 1
			if (checkX >= 0x2000 + 0x2380)
				bottomBoundaryTarget = 0x300 + 0x200 + 0xE0;
			else if (checkX >= 0x1500 + 0x2380)
				bottomBoundaryTarget = 0x400 + 0x200 + 0xE0;
			else if (checkX >= 0xED0 + 0x2380)
				bottomBoundaryTarget = 0x200 + 0x200 + 0xE0;
			else if (checkX >= 0x2380)
				bottomBoundaryTarget = 0x300 + 0x200 + 0xE0;
			else if (checkX >= 0x1780)
				bottomBoundaryTarget = 0x400 + 0xE0;
			else
				bottomBoundaryTarget = 0x300 + 0xE0;
			break;
		case LEVELID_GHZ2: //Green Hill Zone Act 2
			switch (dynamicEventRoutine)
			{
				case 0:
					if (checkX >= 0x3700)
					{
						bottomBoundaryTarget = 0x5F8;
					}
					else if (checkX >= 0x2E00)
					{
						bottomBoundaryTarget = 0x580;
					}
					else if (checkX >= 0x2980)
					{
						bottomBoundaryTarget = 0x500;
					}
					else
					{
						bottomBoundaryTarget = 0x300 + 0xE0;
						if (checkX < 0x380)
							break;
						bottomBoundaryTarget = 0x310 + 0xE0;
						if (checkX < 0x960)
							break;
						if (checkY >= 0x280)
						{
							bottomBoundaryTarget = 0x400 + 0xE0;
							if (checkX < 0x1380)
							{
								bottomBoundaryTarget = 0x4C0 + 0xE0;
								bottomBoundary = 0x4C0 + 0xE0;
							}
							if (checkX < 0x1700)
								break;
						}
						bottomBoundaryTarget = 0x300 + 0xE0;
						dynamicEventRoutine = 1;
					}
					break;
				case 1:
					if (checkX < 0x960 || checkX >= 0x2980)
						dynamicEventRoutine = 0;
					break;
			}
			break;
		default:
			break;
	}
	
	//Move up/down to the boundary
	int16_t move = 2;

	if (bottomBoundaryTarget < bottomBoundary)
	{
		//Move up to the boundary smoothly
		if ((camera->yPos + gRenderSpec.height) > bottomBoundaryTarget)
			bottomBoundary = (camera->yPos + gRenderSpec.height);
		
		//Move
		bottomBoundary -= move;
		if (bottomBoundaryTarget > bottomBoundary)
			bottomBoundary = bottomBoundaryTarget;
	}
	else if (bottomBoundaryTarget > bottomBoundary)
	{
		//Move faster if in mid-air
		if ((camera->yPos + 8 + gRenderSpec.height) >= bottomBoundary && playerList[0]->status.inAir)
			move *= 4;
		
		//Move
		bottomBoundary += move;
		if (bottomBoundaryTarget < bottomBoundary)
			bottomBoundary = bottomBoundaryTarget;
	}
	
	//Set boundaries to target
	int16_t left = camera->xPos;
	int16_t right = camera->xPos + gRenderSpec.width;
	
	if (leftBoundary < leftBoundaryTarget)
	{
		if (left < leftBoundaryTarget)
			leftBoundary = left;
		else
			leftBoundary = leftBoundaryTarget;
	}
	else
	{
		leftBoundary = leftBoundaryTarget;
	}
	
	if (rightBoundary > rightBoundaryTarget)
	{
		if (right > rightBoundaryTarget)
			rightBoundary = right;
		else
			rightBoundary = rightBoundaryTarget;
	}
	else
	{
		rightBoundary = rightBoundaryTarget;
	}
}

//Texture cache and mappings cache
TEXTURE *LEVEL::GetObjectTexture(std::string path)
{
	for (size_t i = 0; i < objTextureCache.size(); i++)
	{
		if (objTextureCache[i]->source == path)
			return objTextureCache[i];
	}
	
	TEXTURE *newTexture = new TEXTURE(path);
	objTextureCache.link_back(newTexture);
	return newTexture;
}

MAPPINGS *LEVEL::GetObjectMappings(std::string path)
{
	for (size_t i = 0; i < objMappingsCache.size(); i++)
	{
		if (objMappingsCache[i]->source == path)
			return objMappingsCache[i];
	}
	
	MAPPINGS *newMappings = new MAPPINGS(path);
	objMappingsCache.link_back(newMappings);
	return newMappings;
}

//Object load functions
OBJECT_LOAD *LEVEL::GetObjectLoad(OBJECT *object)
{
	//Return the object load that holds our object or nullptr
	for (size_t i = 0; i < objectLoadList.size(); i++)
		if (objectLoadList[i]->loaded == object)
			return objectLoadList[i];
	return nullptr;
}

void LEVEL::LinkObjectLoad(OBJECT *object)
{
	//Define our object load struct and link it
	OBJECT_LOAD *objectLoad = new OBJECT_LOAD;
	objectLoad->function = object->function;
	objectLoad->status = object->status;
	objectLoad->xLong = object->xLong;
	objectLoad->yLong = object->yLong;
	objectLoad->subtype = object->subtype;
	
	objectLoad->loaded = object;
	objectLoad->loadRange = false;
	objectLoad->specificBit = false;
	
	objectLoadList.link_back(objectLoad);
}

void LEVEL::ReleaseObjectLoad(OBJECT *object)
{
	//Remove object from object load list
	for (size_t i = 0; i < objectLoadList.size(); i++)
	{
		if (objectLoadList[i]->loaded == object)
		{
			delete objectLoadList[i];
			objectLoadList.erase(i--);
		}
	}
}

void LEVEL::UnrefObjectLoad(OBJECT *object)
{
	//Remove references to object
	for (size_t i = 0; i < objectLoadList.size(); i++)
	{
		if (objectLoadList[i]->loaded == object)
			objectLoadList[i]->loaded = nullptr;
	}
}

void LEVEL::CheckObjectLoad()
{
	//Check all object loads if they should be loaded
	for (size_t i = 0; i < objectLoadList.size(); i++)
	{
		//Check if this object load is in load range
		uint16_t xOff = (objectLoadList[i]->x.pos & 0xFF80) - ((camera->xPos - 0x80) & 0xFF80);
		bool isLoadRange = xOff <= upperRound(0x80 + gRenderSpec.width + 0x80, 0x80);
		
		//Check if we're just now in range, and load object if so
		if (isLoadRange == true && objectLoadList[i]->loadRange == false && objectLoadList[i]->loaded == nullptr)
		{
			//Load the object if in-range
			OBJECT *newObject = new OBJECT(objectLoadList[i]->function);
			newObject->status = objectLoadList[i]->status;
			newObject->xLong = objectLoadList[i]->xLong;
			newObject->yLong = objectLoadList[i]->yLong;
			newObject->subtype = objectLoadList[i]->subtype;
			objectLoadList[i]->loaded = newObject;
			
			gLevel->objectList.link_back(newObject);
		}
		
		//Update the object load's state
		objectLoadList[i]->loadRange = isLoadRange;
	}
}

//Object layer function
LEVEL_RENDERLAYER LEVEL::GetObjectLayer(bool highPriority, int priority) { return (LEVEL_RENDERLAYER)(highPriority ? (LEVEL_RENDERLAYER_OBJECT_HIGH_0 + priority) : (LEVEL_RENDERLAYER_OBJECT_LOW_0 + priority)); }

//Oscillatory Update
static const bool oscillatoryInitialDirection[OSCILLATORY_VALUES] = {false, false, true, true, true, true, true, false, false, false, false, false, false};

static const uint16_t oscillatoryInitial[OSCILLATORY_VALUES][2] = {
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
	{0x50F0, 0x011E},
	{0x2080, 0x00B4},
	{0x3080, 0x010E},
	{0x5080, 0x01C2},
	{0x7080, 0x0276},
	{0x0080, 0x0000},
	{0x0080, 0x0000},
};

static const uint16_t oscillatorySettings[OSCILLATORY_VALUES][2] = {
//frequency, amplitude
	{0x0002, 0x0010},
	{0x0002, 0x0018},
	{0x0002, 0x0020},
	{0x0002, 0x0030},
	{0x0004, 0x0020},
	{0x0008, 0x0008},
	{0x0008, 0x0040},
	{0x0004, 0x0040},
	{0x0002, 0x0050},
	{0x0002, 0x0050},
	{0x0002, 0x0020},
	{0x0003, 0x0030},
	{0x0005, 0x0050},
	{0x0007, 0x0070},
	{0x0002, 0x0010},
	{0x0002, 0x0010},
};

void LEVEL::OscillatoryInit()
{
	//Copy our initial oscillation values
	for (int i = 0; i < OSCILLATORY_VALUES; i++)
	{
		oscillateDirection[i] = oscillatoryInitialDirection[i];
		oscillate[i][0] = oscillatoryInitial[i][0];
		oscillate[i][1] = oscillatoryInitial[i][1];
	}
}

void LEVEL::OscillatoryUpdate()
{
	//Update all of our oscillation values
	for (int i = 0; i < OSCILLATORY_VALUES; i++)
	{
		//Get our frequency and amplitude
		uint16_t frequency = oscillatorySettings[i][0];
		uint16_t amplitude = oscillatorySettings[i][1];
		
		//Check if our direction is reversed
		if (oscillateDirection[i] == false)
		{
			//Increment according to our frequency
			oscillate[i][1] += frequency;
			oscillate[i][0] += (int16_t)oscillate[i][1];
			
			//Reverse if reached amplitude
			if ((uint8_t)(oscillate[i][0] >> 8) > (uint8_t)amplitude)
				oscillateDirection[i] = true;
		}
		else
		{
			//Increment according to our frequency
			oscillate[i][1] -= frequency;
			oscillate[i][0] += (int16_t)oscillate[i][1];
			
			//Reverse if reached amplitude
			if ((uint8_t)(oscillate[i][0] >> 8) <= (uint8_t)amplitude)
				oscillateDirection[i] = false;
		}
	}
}

//Level update and draw
bool LEVEL::UpdateStage()
{
	if (updateStage)
	{
		//Update players and objects
		for (size_t i = 0; i < playerList.size(); i++)
			playerList[i]->Update();
	
		for (size_t i = 0; i < objectList.size(); i++)
		{
			if (objectList[i]->Update())
			{
				fail = objectList[i]->fail;
				return true;
			}
		}
		
		for (size_t i = 0; i < coreObjectList.size(); i++)
		{
			if (coreObjectList[i]->Update())
			{
				fail = coreObjectList[i]->fail;
				return true;
			}
		}
	}
	else
	{
		//If not to update the stage, only update players and core objects
		for (size_t i = 0; i < playerList.size(); i++)
			playerList[i]->Update();
		
		for (size_t i = 0; i < coreObjectList.size(); i++)
		{
			if (coreObjectList[i]->Update())
			{
				fail = coreObjectList[i]->fail;
				return true;
			}
		}
	}
	
	//Check for object deletion
	CHECK_LINKEDLIST_OBJECTDELETE(objectList)
	CHECK_LINKEDLIST_OBJECTDELETE(coreObjectList)
	
	//Update camera
	if (camera != nullptr)
		camera->Track(playerList[0]);
	return false;
}

bool LEVEL::Update()
{
	//Update title card
	titleCard->UpdateAndDraw();
	if (titleCard->activeLock)
		return false;
	
	//Quit if fading
	if (fading)
		return false;
	
	//Update the stage
	if (UpdateStage())
		return true;
	frameCounter++;
	
	//Update level dynamic events
	if (playerList.size())
		DynamicEvents();
	
	//Load objects and update oscillatory values
	CheckObjectLoad();
	OscillatoryUpdate();
	
	//Increase our time
	if (gLevel->updateTime)
		gTime++;
	return false;
}

void LEVEL::Draw()
{
	//Update palette cycling
	if (!fading)
	{
		//Cycle player palettes (super)
		for (size_t i = 0; i < playerList.size(); i++)
			playerList[i]->SuperPaletteCycle();
		if (paletteFunction != nullptr)
			paletteFunction();
	}
	
	//Draw and scroll background
	if (background != nullptr && camera != nullptr)
		background->Draw(updateStage, camera->xPos, camera->yPos);
	
	//Draw foreground
	if (layout.foreground != nullptr && tileTexture != nullptr && camera != nullptr)
	{
		size_t cLeft = mmax(camera->xPos / 16, 0);
		size_t cTop = mmax(camera->yPos / 16, 0);
		size_t cRight = mmin(upperRound(camera->xPos + (size_t)gRenderSpec.width, 16) / 16, (size_t)gLevel->layout.width - 1);
		size_t cBottom = mmin(upperRound(camera->yPos + (size_t)gRenderSpec.height, 16) / 16, (size_t)gLevel->layout.height - 1);
		
		for (size_t ty = cTop; ty < cBottom; ty++)
		{
			for (size_t tx = cLeft; tx < cRight; tx++)
			{
				//Get tile
				TILE *tile = &layout.foreground[ty * layout.width + tx];
				if (tile->tile >= tiles || tile->tile >= tileTexture->height / 16)
					continue;
				
				//Draw tile
				RECT backSrc = {0, tile->tile * 16, 16, 16};
				RECT frontSrc = {16, tile->tile * 16, 16, 16};
				gSoftwareBuffer->DrawTexture(tileTexture, tileTexture->loadedPalette, &backSrc, LEVEL_RENDERLAYER_FOREGROUND_LOW, tx * 16 - camera->xPos, ty * 16 - camera->yPos, tile->xFlip, tile->yFlip);
				gSoftwareBuffer->DrawTexture(tileTexture, tileTexture->loadedPalette, &frontSrc, LEVEL_RENDERLAYER_FOREGROUND_HIGH, tx * 16 - camera->xPos, ty * 16 - camera->yPos, tile->xFlip, tile->yFlip);
			}
		}
	}
	
	//Draw players and objects
	for (size_t i = 0; i < playerList.size(); i++)
		playerList[i]->DrawToScreen();
	for (size_t i = 0; i < objectList.size(); i++)
		objectList[i]->Draw();
	for (size_t i = 0; i < coreObjectList.size(); i++)
		coreObjectList[i]->Draw();
	
	//Draw HUD
	hud->Draw();
}
