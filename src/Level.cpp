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
	nullptr, &ObjBridge, nullptr, nullptr, nullptr, &ObjSwingingPlatform, nullptr, nullptr,
	&ObjGHZPlatform, nullptr, nullptr, nullptr, &ObjSonic1Scenery, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, &ObjRingSpawner, &ObjMonitor, nullptr,
	nullptr, nullptr, nullptr, &ObjChopper, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, &ObjPurpleRock, nullptr, nullptr, nullptr, nullptr,
	&ObjMotobug, &ObjSpring, nullptr, nullptr, &ObjGHZEdgeWall, nullptr, nullptr, nullptr,
	nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
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

//Our level table
LEVELTABLE gLevelTable[LEVELID_MAX] = {
	//ZONEID_GHZ
		/*LEVELID_GHZ1*/ {ZONEID_GHZ, 0, "Green Hill Zone", "Act 1", LEVELFORMAT_CHUNK128_SONIC2, OBJECTFORMAT_SONIC1, ARTFORMAT_BMP, "data/Level/GHZ/ghz1", "data/Level/GHZ/ghz", "data/Level/sonic1", "data/Level/GHZ/ghz", &GHZ_Background, &GHZ_PaletteCycle, objFuncSonic1, "GHZ", 0x0050, 0x03B0, 0x0000, 0x255F, 0x0000, 0x03E0},
		/*LEVELID_GHZ2*/ {ZONEID_GHZ, 1, "Green Hill Zone", "Act 2", LEVELFORMAT_CHUNK128_SONIC2, OBJECTFORMAT_SONIC1, ARTFORMAT_BMP, "data/Level/GHZ/ghz2", "data/Level/GHZ/ghz", "data/Level/sonic1", "data/Level/GHZ/ghz", &GHZ_Background, &GHZ_PaletteCycle, objFuncSonic1, "GHZ", 0x0050, 0x00FC, 0x0000, 0x214B, 0x0000, 0x03E0},
	//ZONEID_EHZ
		/*LEVELID_EHZ1*/ {ZONEID_EHZ, 0, "Emerald Hill Zone", "Act 1", LEVELFORMAT_CHUNK128_SONIC2, OBJECTFORMAT_SONIC2, ARTFORMAT_BMP, "data/Level/EHZ/ehz1", "data/Level/EHZ/ehz", "data/Level/sonic2", "data/Level/EHZ/ehz", &EHZ_Background, &EHZ_PaletteCycle, objFuncSonic2, "EHZ", 0x0060, 0x028F, 0x0000, 0x2A40, 0x0000, 0x0400},
};

//Loading functions
bool LEVEL::LoadMappings(LEVELTABLE *tableEntry)
{
	LOG(("Loading mappings... "));
	
	//Load chunk mappings
	switch (format)
	{
		case LEVELFORMAT_CHUNK128_SONIC2:
		case LEVELFORMAT_CHUNK128:
		{
			//Open our chunk mapping file
			char *mappingPath = AllocPath(gBasePath, tableEntry->chunkTileReferencePath, ".chk");
			BACKEND_FILE *mappingFile = OpenFile(mappingPath, "rb");
			delete[] mappingPath;
			
			if (mappingFile == nullptr)
			{
				Error(fail = GetFileError());
				return true;
			}
			
			//Allocate the chunk mappings in memory
			chunks = (GetFileSize(mappingFile) / 2 / (8 * 8));
			chunkMapping = (CHUNKMAPPING*)malloc(sizeof(CHUNKMAPPING) * chunks);
			
			if (chunkMapping == nullptr)
			{
				Error(fail = "Failed to allocate chunk mappings in memory");
				CloseFile(mappingFile);
				return true;
			}
			
			//Read the mapping data
			for (int i = 0; i < chunks; i++)
			{
				for (int v = 0; v < (8 * 8); v++)
				{
					uint16_t tmap = ReadFile_BE16(mappingFile);
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
			
			CloseFile(mappingFile);
			break;
		}
		
		default:
			LOG(("Level format %d doesn't use chunk mappings\n", format));
			break;
	}
	
	LOG(("Success!\n"));
	return false;
}

bool LEVEL::LoadLayout(LEVELTABLE *tableEntry)
{
	LOG(("Loading layout... "));
	
	//Open our layout file
	char *layoutPath = AllocPath(gBasePath, tableEntry->levelReferencePath, ".lay");
	BACKEND_FILE *layoutFile = OpenFile(layoutPath, "rb");
	delete[] layoutPath;
	
	if (layoutFile == nullptr)
	{
		Error(fail = GetFileError());
		return true;
	}
	
	//Read our layout file
	switch (format = tableEntry->format)
	{
		case LEVELFORMAT_CHUNK128_SONIC2:
		case LEVELFORMAT_CHUNK128:
			//Get our level dimensions (upscaled to tiles)
			if (format == LEVELFORMAT_CHUNK128)
			{
				layout.width = ReadFile_BE16(layoutFile) * 8;
				layout.height = ReadFile_BE16(layoutFile) * 8;
			}
			else
			{
				layout.width = 0x80 * 8;
				layout.height = 0x10 * 8;
			}
			
			//Allocate our layout
			layout.foreground = (TILE*)malloc(sizeof(TILE) * layout.width * layout.height);
			
			if (layout.foreground == nullptr)
			{
				Error(fail = "Failed to allocate layout in memory");
				CloseFile(layoutFile);
				return true;
			}
			
			//Read our layout file and convert to tiles
			for (int cy = 0; cy < layout.height; cy += 8)
			{
				//Read foreground line
				for (int cx = 0; cx < layout.width; cx += 8)
				{
					//Read our chunk index
					uint8_t chunk = ReadFile_Byte(layoutFile);
					
					//Get our tiles
					for (int tv = 0; tv < 8 * 8; tv++)
						layout.foreground[(cy + (tv / 8)) * layout.width + (cx + (tv % 8))] = chunkMapping[chunk].tile[tv];
				}
				
				//Skip background line (not used)
				SeekFile(layoutFile, layout.width / 8, FILESEEK_CUR);
			}
			
			CloseFile(layoutFile);
			break;
		case LEVELFORMAT_TILE:
			//Get our level dimensions
			layout.width = ReadFile_BE32(layoutFile);
			layout.height = ReadFile_BE32(layoutFile);
			
			//Allocate our layout
			layout.foreground = (TILE*)malloc(sizeof(TILE) * layout.width * layout.height);
			
			if (layout.foreground == nullptr)
			{
				Error(fail = "Failed to allocate layout in memory");
				CloseFile(layoutFile);
				return true;
			}
			
			//Read our layout file
			for (int tv = 0; tv < layout.width * layout.height; tv++)
			{
				uint16_t tmap = ReadFile_BE16(layoutFile);
				layout.foreground[tv].altLRB	= (tmap & 0x8000) != 0;
				layout.foreground[tv].altTop	= (tmap & 0x4000) != 0;
				layout.foreground[tv].norLRB	= (tmap & 0x2000) != 0;
				layout.foreground[tv].norTop	= (tmap & 0x1000) != 0;
				layout.foreground[tv].yFlip		= (tmap & 0x0800) != 0;
				layout.foreground[tv].xFlip		= (tmap & 0x0400) != 0;
				layout.foreground[tv].tile		= (tmap & 0x3FF);
			}
			
			CloseFile(layoutFile);
			break;
		default:
			Error(fail = "Unimplemented level format");
			CloseFile(layoutFile);
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
	char *normalMapPath = AllocPath(gBasePath, tableEntry->chunkTileReferencePath, ".nor");
	char *alternateMapPath = AllocPath(gBasePath, tableEntry->chunkTileReferencePath, ".alt");
	BACKEND_FILE *norMapFile = OpenFile(normalMapPath, "rb");
	BACKEND_FILE *altMapFile = OpenFile(alternateMapPath, "rb");
	delete[] alternateMapPath;
	delete[] normalMapPath;
	
	if (norMapFile == nullptr || altMapFile == nullptr)
	{
		Error(fail = GetFileError());
		if (norMapFile)
			CloseFile(norMapFile);
		if (altMapFile)
			CloseFile(altMapFile);
		return true;
	}
	
	//Read our tile collision map data
	tiles = GetFileSize(norMapFile);
	tileMapping = (TILEMAPPING*)malloc(sizeof(TILEMAPPING) * tiles);
	
	if (GetFileSize(norMapFile) != GetFileSize(altMapFile))
	{
		Error(fail = "Normal map and alternate map files don't match in size");
		if (norMapFile)
			CloseFile(norMapFile);
		if (altMapFile)
			CloseFile(altMapFile);
		return true;
	}
	
	for (int i = 0; i < tiles; i++)
	{
		tileMapping[i].normalColTile = ReadFile_Byte(norMapFile);
		tileMapping[i].alternateColTile = ReadFile_Byte(altMapFile);
	}
	
	CloseFile(norMapFile);
	CloseFile(altMapFile);
	
	//Open our collision tile files
	char *collisionNormalPath = AllocPath(gBasePath, tableEntry->collisionReferencePath, ".can");
	char *collisionRotatedPath = AllocPath(gBasePath, tableEntry->collisionReferencePath, ".car");
	char *collisionAnglePath = AllocPath(gBasePath, tableEntry->collisionReferencePath, ".ang");
	BACKEND_FILE *colNormalFile = OpenFile(collisionNormalPath, "rb");
	BACKEND_FILE *colRotatedFile = OpenFile(collisionRotatedPath, "rb");
	BACKEND_FILE *colAngleFile = OpenFile(collisionAnglePath, "rb");
	delete[] collisionAnglePath;
	delete[] collisionRotatedPath;
	delete[] collisionNormalPath;
	
	if (colNormalFile == nullptr || colRotatedFile == nullptr || colAngleFile == nullptr)
	{
		Error(fail = GetFileError());
		if (colNormalFile != nullptr)
			CloseFile(colNormalFile);
		if (colRotatedFile != nullptr)
			CloseFile(colRotatedFile);
		if (colAngleFile != nullptr)
			CloseFile(colAngleFile);
		return true;
	}
	
	//Allocate our collision tile data in memory
	if ((GetFileSize(colNormalFile) != GetFileSize(colRotatedFile)) || (GetFileSize(colAngleFile) != (GetFileSize(colNormalFile) / 0x10)))
	{
		Error(fail = "Collision tile data file sizes don't match each-other\n(Are the files compressed?)");
		
		free(layout.foreground);
		CloseFile(colNormalFile);
		CloseFile(colRotatedFile);
		CloseFile(colAngleFile);
		return true;
	}
	
	collisionTiles = GetFileSize(colNormalFile) / 0x10;
	collisionTile = (COLLISIONTILE*)malloc(sizeof(COLLISIONTILE) * collisionTiles);
	
	if (collisionTile == nullptr)
	{
		Error(fail = "Failed to allocate collision tile data in memory");
		CloseFile(colNormalFile);
		CloseFile(colRotatedFile);
		CloseFile(colAngleFile);
		return true;
	}
	
	//Read our collision tile data
	for (int i = 0; i < collisionTiles; i++)
	{
		for (int v = 0; v < 0x10; v++)
		{
			collisionTile[i].normal[v] = ReadFile_Byte(colNormalFile);
			collisionTile[i].rotated[v] = ReadFile_Byte(colRotatedFile);
		}
		
		collisionTile[i].angle = ReadFile_Byte(colAngleFile);
	}
	
	CloseFile(colNormalFile);
	CloseFile(colRotatedFile);
	CloseFile(colAngleFile);
	LOG(("Success!\n"));
	return false;
}

bool LEVEL::LoadObjects(LEVELTABLE *tableEntry)
{
	LOG(("Loading objects... "));
	
	//Open our object file
	char *objectPath = AllocPath(gBasePath, tableEntry->levelReferencePath, ".obj");
	BACKEND_FILE *objectFile = OpenFile(objectPath, "rb");
	delete[] objectPath;
	
	if (objectFile == nullptr)
	{
		Error(fail = GetFileError());
		return true;
	}
	
	//Read our object data
	switch (tableEntry->objectFormat)
	{
		case OBJECTFORMAT_SONIC1:
		case OBJECTFORMAT_SONIC2:
		{
			int objects = GetFileSize(objectFile) / 6;
			
			for (int i = 0; i < objects; i++)
			{
				int16_t xPos = ReadFile_BE16(objectFile);
				int16_t word2 = ReadFile_BE16(objectFile);
				int16_t yPos = word2 & 0x0FFF;
				
				uint8_t id = ReadFile_Byte(objectFile);
				uint8_t subtype = ReadFile_Byte(objectFile);
				
				if (tableEntry->objectFormat == OBJECTFORMAT_SONIC1)
					id &= 0x7F;
				
				//Create a new object with the loaded information
				OBJECT newObject(tableEntry->objectFunctionList[id]);
				if (newObject.fail)
				{
					Error(fail = newObject.fail);
					return true;
				}
				
				newObject.x.pos = xPos;
				newObject.y.pos = yPos;
				
				if (tableEntry->objectFormat == OBJECTFORMAT_SONIC1)
				{
					newObject.status.yFlip = (word2 & 0x8000) != 0;
					newObject.status.xFlip = (word2 & 0x4000) != 0;
				}
				else
				{
					newObject.status.yFlip = (word2 & 0x4000) != 0;
					newObject.status.xFlip = (word2 & 0x2000) != 0;
				}
				
				newObject.subtype = subtype;
				objectList.push_back(newObject);
			}
		}
	}
	
	CloseFile(objectFile);
	
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
			//Create ring object
			OBJECT newObject(&ObjRing);
			if (newObject.fail)
			{
				Error(fail = newObject.fail);
				CloseFile(ringFile);
				return true;
			}
			
			newObject.x.pos = xPos;
			newObject.y.pos = yPos;
			objectList.push_back(newObject);
			
			//Offset
			if (type & 0x8)
				yPos += 0x18;
			else
				xPos += 0x18;
		}
	}
	
	CloseFile(ringFile);
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
			char *artPath = AllocPath(tableEntry->artReferencePath, ".tileset.bmp", nullptr);
			tileTexture = new TEXTURE(artPath);
			delete[] artPath;
			
			if (tileTexture->fail)
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
	char *backPath = AllocPath(tableEntry->artReferencePath, ".background.bmp", nullptr);
	background = new BACKGROUND(backPath, tableEntry->backFunction);
	delete[] backPath;
	
	if (background->fail)
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
	free(layout.foreground);
	free(chunkMapping);
	free(tileMapping);
	free(collisionTile);
	
	//Unload textures
	if (tileTexture != nullptr)
		delete tileTexture;
	if (background != nullptr)
		delete background;
	
	//Unload players, objects, and camera
	playerList.clear();
	playerList.shrink_to_fit();
	objectList.clear();
	objectList.shrink_to_fit();
	coreObjectList.clear();
	coreObjectList.shrink_to_fit();
	
	if (camera != nullptr)
		delete camera;
	if (titleCard != nullptr)
		delete titleCard;
	if (hud != nullptr)
		delete hud;
	
	//Unload object textures and mappings
	objTextureCache.clear();
	objTextureCache.shrink_to_fit();
	objMappingsCache.clear();
	objMappingsCache.shrink_to_fit();
	
	//Lock audio device so we can safely unload all loaded music
	AUDIO_LOCK;
	
	//Unload music
	if (stageMusic != nullptr)
		delete stageMusic;
	if (bossMusic != nullptr)
		delete bossMusic;
	
	if (speedShoesMusic != nullptr)
		delete speedShoesMusic;
	if (invincibilityMusic != nullptr)
		delete invincibilityMusic;
	if (superMusic != nullptr)
		delete superMusic;
	if (goalMusic != nullptr)
		delete goalMusic;
	if (gameoverMusic != nullptr)
		delete gameoverMusic;
	if (extraLifeMusic != nullptr)
		delete extraLifeMusic;
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

//Assets to pre-load (Assets that are loaded by objects that are usually created mid-game)
const char *preloadTexture[] = {
	"data/Object/Explosion.bmp",
	"data/Object/Shield.bmp",
	"data/Object/InvincibilitySuperStars.bmp",
	nullptr,
};

const char *preloadMappings[] = {
	"data/Object/Explosion.map",
	"data/Object/MonitorContents.map",
	"data/Object/InstaShield.map",
	"data/Object/BlueShield.map",
	"data/Object/FireShield.map",
	"data/Object/ElectricShield.map",
	"data/Object/BubbleShield.map",
	"data/Object/InvincibilityStars.map",
	"data/Object/SuperStars.map",
	nullptr,
};

//Level class
LEVEL::LEVEL(int id, const char *players[])
{
	LOG(("Loading level ID %d...\n", id));
	
	//Reset memory, then reinitialize linked lists
	memset(this, 0, sizeof(LEVEL));
	
	playerList = std::deque<PLAYER>();
	objectList = std::deque<OBJECT>();
	coreObjectList = std::deque<OBJECT>();
	objTextureCache = std::deque<TEXTURE>();
	objMappingsCache = std::deque<MAPPINGS>();
	
	//Set us as the global level
	gLevel = this;
	
	//Copy zone and act id
	LEVELTABLE *tableEntry = &gLevelTable[levelId = (LEVELID)id];
	zone = tableEntry->zone;
	act = tableEntry->act;
	
	//Load data
	if (LoadMappings(tableEntry) || LoadLayout(tableEntry) || LoadCollisionTiles(tableEntry) || LoadObjects(tableEntry) || LoadArt(tableEntry))
	{
		//Unload any loaded data
		UnloadAll();
		return;
	}
	
	//Pre-load assets
	for (int i = 0; preloadTexture[i] != nullptr; i++)
		GetObjectTexture(preloadTexture[i]);
	for (int i = 0; preloadMappings[i] != nullptr; i++)
		GetObjectMappings(preloadMappings[i]);
	
	//Create our players
	PLAYER *follow = nullptr;
	
	for (int i = 0; *players != nullptr; i++, players++)
	{
		//Create our player
		PLAYER newPlayer(*players, follow, i);
		if (newPlayer.fail)
		{
			fail = newPlayer.fail;
			UnloadAll();
			return;
		}
		
		newPlayer.x.pos = tableEntry->startX - (i * 16);
		newPlayer.y.pos = tableEntry->startY;
		follow = &newPlayer;
		playerList.push_back(newPlayer);
	}
	
	//Create our camera
	camera = new CAMERA(&playerList[0]);
	
	//Title-card
	titleCard = new TITLECARD(tableEntry->name, tableEntry->subtitle);
	if (titleCard->fail)
	{
		fail = titleCard->fail;
		UnloadAll();
		return;
	}
	
	inTitleCard = true;
	
	//HUD
	hud = new HUD();
	if (hud->fail)
	{
		fail = hud->fail;
		UnloadAll();
		return;
	}
	
	//Initialize state
	OscillatoryInit();
	
	updateTime = true;
	updateStage = true;
	
	//Lock audio device so we can load new music
	AUDIO_LOCK;
	
	stageMusic = new MUSIC(tableEntry->music, 0, 1.0f);
	
	speedShoesMusic = new MUSIC("SpeedShoes", 0, 1.0f);
	invincibilityMusic = new MUSIC("Invincibility", 0, 1.0f);
	superMusic = new MUSIC("Super", 0, 1.0f);
	goalMusic = new MUSIC("Goal", 0, 1.0f);
	gameoverMusic = new MUSIC("GameOver", 0, 1.0f);
	extraLifeMusic = new MUSIC("ExtraLife", 0, 1.0f);
	
	AUDIO_UNLOCK;
	
	//Check if any music failed to load...
	if (stageMusic->fail || speedShoesMusic->fail || invincibilityMusic->fail || superMusic->fail || goalMusic->fail || gameoverMusic->fail || extraLifeMusic->fail)
	{
		fail = "Failed to load music";
		UnloadAll();
		return;
	}
	
	//Play stage music
	ChangePrimaryMusic(stageMusic);
	
	//Initialize scores
	InitializeScores();
	
	//Run players code (with input reset)
	ClearControllerInput();
	for (size_t i = 0; i < playerList.size(); i++)
		playerList[i].Update();
	
	//Update object and check for deletion
	for (size_t i = 0; i < objectList.size(); i++)
	{
		if (objectList[i].Update())
		{
			Error(fail = objectList[i].fail);
			UnloadAll();
			return;
		}
	}
	
	for (size_t i = 0; i < coreObjectList.size(); i++)
	{
		if (coreObjectList[i].Update())
		{
			Error(fail = coreObjectList[i].fail);
			UnloadAll();
			return;
		}
	}
	
	CHECK_LINKEDLIST_OBJECTDELETE(objectList);
	CHECK_LINKEDLIST_OBJECTDELETE(coreObjectList);
	
	//Set the camera to follow the player
	camera->Track(&playerList[0]);
	
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
			function(objTextureCache[i].loadedPalette);
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
		finished = function(objTextureCache[i].loadedPalette) ? finished : false;
	
	//Fade music out
	if (currentMusic != nullptr && !isFadingIn)
	{
		//Lock audio device so we can safely modify the music's volume
		AUDIO_LOCK;
		
		//Fade music out
		currentMusic->volume = mmax(currentMusic->volume - (1.0f / 32.0f), 0.0f);
		
		//Unlock audio device
		AUDIO_UNLOCK;
	}
	
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
				//Get this player
				PLAYER *player = &playerList[i];
				
				//Get which tile we're on
				if (player->x.pos < 0 || player->x.pos >= gLevel->layout.width * 16 || player->y.pos < 0 || player->y.pos >= gLevel->layout.height * 16)
					continue;
				TILE *tile = &gLevel->layout.foreground[(player->y.pos / 16) * gLevel->layout.width + (player->x.pos / 16)];;
				
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
					if (!player->status.pinballMode)
						player->ChkRoll();
					player->status.pinballMode = true;
				}
				else
					player->status.pinballMode = false;
			}
			break;
		default:
			break;
	}
	
	//Level specific events
	int16_t checkX = camera->x + (gRenderSpec.width - 320) / 2;
	
	switch (levelId)
	{
		case LEVELID_GHZ1: //Green Hill Zone Act 1
			if (checkX < 0x1780)
				bottomBoundaryTarget = 0x3E0;
			else
				bottomBoundaryTarget = 0x4E0;
			break;
		case LEVELID_GHZ2:
			if (checkX >= 0x2000)
				bottomBoundaryTarget = 0x3E0;
			else if (checkX >= 0x1500)
				bottomBoundaryTarget = 0x4E0;
			else if (checkX >= 0xED0)
				bottomBoundaryTarget = 0x2E0;
			else
				bottomBoundaryTarget = 0x3E0;
			break;
		default:
			break;
	}
	
	//Move up/down to the boundary
	int16_t move = 2;

	if (bottomBoundaryTarget < bottomBoundary)
	{
		//Move up to the boundary smoothly
		if ((camera->y + gRenderSpec.height) > bottomBoundaryTarget)
			bottomBoundary = (camera->y + gRenderSpec.height);
		
		//Move
		bottomBoundary -= move;
		if (bottomBoundaryTarget > bottomBoundary)
			bottomBoundary = bottomBoundaryTarget;
	}
	else if (bottomBoundaryTarget > bottomBoundary)
	{
		//Move faster if in mid-air
		if ((camera->y + 8 + gRenderSpec.height) >= bottomBoundary && playerList[0].status.inAir)
			move *= 4;
		
		//Move
		bottomBoundary += move;
		if (bottomBoundaryTarget < bottomBoundary)
			bottomBoundary = bottomBoundaryTarget;
	}
	
	//Set boundaries to target
	int16_t left = camera->x;
	int16_t right = camera->x + gRenderSpec.width;
	
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
TEXTURE *LEVEL::GetObjectTexture(const char *path)
{
	for (size_t i = 0; i < objTextureCache.size(); i++)
	{
		if (objTextureCache[i].source != nullptr && !strcmp(objTextureCache[i].source, path))
			return &(objTextureCache[i]);
	}
	
	TEXTURE newTexture(path);
	objTextureCache.push_back(newTexture);
	return &(objTextureCache.back());
}

MAPPINGS *LEVEL::GetObjectMappings(const char *path)
{
	for (size_t i = 0; i < objMappingsCache.size(); i++)
	{
		if (objMappingsCache[i].source != nullptr && !strcmp(objMappingsCache[i].source, path))
			return &(objMappingsCache[i]);
	}
	
	MAPPINGS newMappings(path);
	objMappingsCache.push_back(newMappings);
	return &(objMappingsCache.back());
}

LEVEL_RENDERLAYER LEVEL::GetObjectLayer(bool highPriority, int priority)
{
	return (LEVEL_RENDERLAYER)(highPriority ? (LEVEL_RENDERLAYER_OBJECT_HIGH_0 + priority) : (LEVEL_RENDERLAYER_OBJECT_LOW_0 + priority));
}

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

//Music functions
void LEVEL::SetPlayingMusic(MUSIC *music, bool resumeLastPosition, bool fade)
{
	//Stop last song
	if (currentMusic != nullptr)
		currentMusic->playing = false;
	
	//Play this song
	if (music != nullptr)
	{
		if (!resumeLastPosition)
			music->PlayAtPosition(0);
		else
			music->playing = true;
		music->volume = fade ? 0.0f : (currentMusic == nullptr ? music->volume : currentMusic->volume);
	}
	
	currentMusic = music;
}

void LEVEL::ChangePrimaryMusic(MUSIC *music)
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Check if we should play or not
	bool doRestore = !(currentMusic == primaryMusic || currentMusic == secondaryMusic);
	if (currentMusic == nullptr || currentMusic == primaryMusic)
		SetPlayingMusic(music, doRestore, false); //If was already playing primary music, start playing this music
	primaryMusic = music; //Set primary music to be this music
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::ChangeSecondaryMusic(MUSIC *music)
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Check if we should play or not
	bool doRestore = !(currentMusic == primaryMusic || currentMusic == secondaryMusic);
	if (currentMusic == nullptr || currentMusic == primaryMusic || currentMusic == secondaryMusic)
		SetPlayingMusic(music, doRestore, false); //If was already playing primary or secondary music (secondary music overrides primary music), start playing this music
	secondaryMusic = music; //Set secondary music to be this music
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::PlayJingleMusic(MUSIC *music)
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Play this song and update our music state
	SetPlayingMusic(music, false, false);
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::StopSecondaryMusic()
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Check if we should play the primary song or not
	if (currentMusic == secondaryMusic)
		SetPlayingMusic(primaryMusic, true, false);
	secondaryMusic = nullptr;
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::StopJingleMusic()
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Check what song we should resume (if playing a jingle)
	if (currentMusic != primaryMusic && primaryMusic != secondaryMusic)
	{
		if (secondaryMusic != nullptr)
			SetPlayingMusic(secondaryMusic, true, false);
		else
			SetPlayingMusic(primaryMusic, true, false);
	}
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::UpdateMusic()
{
	//Do not attempt to update music if there is no song playing
	if (currentMusic == nullptr)
	{
		LOG(("There's no music playing!\n"));
		return;
	}
	
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//If the song is temporary and has ended, resume
	if (currentMusic->playing == false)
	{
		//Play the previous song
		if (currentMusic != primaryMusic && currentMusic != secondaryMusic)
		{
			if (secondaryMusic != nullptr)
				SetPlayingMusic(secondaryMusic, true, true);	//Jingle to secondaryMusic
			else if (primaryMusic != nullptr)
				SetPlayingMusic(primaryMusic, true, true);		//Jingle to primaryMusic
		}
	}
	
	//Fade in the currently playing music
	if (currentMusic != nullptr)
		currentMusic->volume = mmin(currentMusic->volume + (1.0f / 180.0f), 1.0f);
	
	//Unlock the audio device
	AUDIO_UNLOCK;
}


//Level update and draw
bool LEVEL::Update()
{
	//Update the music
	UpdateMusic();
	
	//Update title card
	titleCard->UpdateAndDraw();
	if (inTitleCard)
		return true;
	
	//Quit if fading
	if (fading)
		return true;
	
	//Increment frame counter
	frameCounter++;
	
	//Update players
	if (updateStage)
	{
		//Update players and objects
		for (size_t i = 0; i < playerList.size(); i++)
			playerList[i].Update();
	
		for (size_t i = 0; i < objectList.size(); i++)
		{
			if (objectList[i].Update())
				return Error(fail = objectList[i].fail);
		}
		
		for (size_t i = 0; i < coreObjectList.size(); i++)
		{
			if (coreObjectList[i].Update())
				return Error(fail = coreObjectList[i].fail);
		}
	}
	else
	{
		//If not to update the stage, only update players and core objects
		for (size_t i = 0; i < playerList.size(); i++)
			playerList[i].Update();
		
		for (size_t i = 0; i < coreObjectList.size(); i++)
		{
			if (coreObjectList[i].Update())
				return Error(fail = coreObjectList[i].fail);
		}
	}
	
	//Check to delete objects
	CHECK_LINKEDLIST_OBJECTDELETE(objectList);
	CHECK_LINKEDLIST_OBJECTDELETE(coreObjectList);
	
	//Update camera
	if (camera != nullptr)
		camera->Track(&playerList[0]);
	
	//Update level dynamic events
	if (!playerList.empty())
		DynamicEvents();
	
	//Update all other level stuff
	OscillatoryUpdate();
	
	//Increase our time
	if (gLevel->updateTime)
		gTime++;
	return true;
}

void LEVEL::Draw()
{
	//Update palette cycling
	if (!fading)
	{
		//Cycle player palettes (super)
		for (size_t i = 0; i < playerList.size(); i++)
			playerList[i].SuperPaletteCycle();
		if (paletteFunction != nullptr)
			paletteFunction();
	}
	
	//Draw and scroll background
	if (background != nullptr && camera != nullptr)
		background->Draw(updateStage, camera->x, camera->y);
	
	//Draw foreground
	if (layout.foreground != nullptr && tileTexture != nullptr && camera != nullptr)
	{
		int cLeft = mmax(camera->x / 16, 0);
		int cTop = mmax(camera->y / 16, 0);
		int cRight = mmin((camera->x + gRenderSpec.width + 15) / 16, gLevel->layout.width - 1);
		int cBottom = mmin((camera->y + gRenderSpec.height + 15) / 16, gLevel->layout.height - 1);
		
		for (int ty = cTop; ty < cBottom; ty++)
		{
			for (int tx = cLeft; tx < cRight; tx++)
			{
				//Get tile
				TILE *tile = &layout.foreground[ty * layout.width + tx];
				
				if (tile->tile >= tiles || tile->tile >= tileTexture->height / 16)
					continue;
				
				//Draw tile
				RECT backSrc = {0, tile->tile * 16, 16, 16};
				RECT frontSrc = {16, tile->tile * 16, 16, 16};
				gSoftwareBuffer->DrawTexture(tileTexture, tileTexture->loadedPalette, &backSrc, LEVEL_RENDERLAYER_FOREGROUND_LOW, tx * 16 - camera->x, ty * 16 - camera->y, tile->xFlip, tile->yFlip);
				gSoftwareBuffer->DrawTexture(tileTexture, tileTexture->loadedPalette, &frontSrc, LEVEL_RENDERLAYER_FOREGROUND_HIGH, tx * 16 - camera->x, ty * 16 - camera->y, tile->xFlip, tile->yFlip);
			}
		}
	}
	
	//Draw players and objects
	for (size_t i = 0; i < playerList.size(); i++)
		playerList[i].Draw();
	for (size_t i = 0; i < objectList.size(); i++)
		objectList[i].Draw();
	for (size_t i = 0; i < coreObjectList.size(); i++)
		coreObjectList[i].Draw();
	
	//Draw HUD
	hud->Draw();
}
