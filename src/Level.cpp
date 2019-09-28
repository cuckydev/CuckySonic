#include "SDL_rwops.h"

#include "Level.h"
#include "LevelSpecific.h"
#include "Game.h"
#include "Fade.h"
#include "Path.h"
#include "Log.h"
#include "Audio.h"
#include "MathUtil.h"
#include "Error.h"

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
		/*LEVELID_GHZ1*/ {ZONEID_GHZ, 0, "Green Hill Zone", "Act 1", LEVELFORMAT_CHUNK128_SONIC2, OBJECTFORMAT_SONIC1, ARTFORMAT_BMP, "data/Level/GHZ/ghz1", "data/Level/GHZ/ghz", "data/Level/sonic1", "data/Level/GHZ/ghz", "GHZ", objFuncSonic1, 0x0050, 0x03B0, 0x0000, 0x255F, 0x0000, 0x03E0},
		/*LEVELID_GHZ2*/ {ZONEID_GHZ, 1, "Green Hill Zone", "Act 2", LEVELFORMAT_CHUNK128_SONIC2, OBJECTFORMAT_SONIC1, ARTFORMAT_BMP, "data/Level/GHZ/ghz2", "data/Level/GHZ/ghz", "data/Level/sonic1", "data/Level/GHZ/ghz", "GHZ", objFuncSonic1, 0x0050, 0x00FC, 0x0000, 0x214B, 0x0000, 0x03E0},
	//ZONEID_EHZ
		/*LEVELID_EHZ1*/ {ZONEID_EHZ, 0, "Emerald Hill Zone", "Act 1", LEVELFORMAT_CHUNK128_SONIC2, OBJECTFORMAT_SONIC2, ARTFORMAT_BMP, "data/Level/EHZ/ehz1", "data/Level/EHZ/ehz", "data/Level/sonic2", "data/Level/EHZ/ehz", "EHZ", objFuncSonic2, 0x0060, 0x028F, 0x0000, 0x2A40, 0x0000, 0x0400},
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
			GET_GLOBAL_PATH(globalPath, tableEntry->chunkTileReferencePath);
			GET_APPEND_PATH(mappingPath, globalPath, ".chk");
			
			SDL_RWops *mappingFile = SDL_RWFromFile(mappingPath, "rb");
			if (mappingFile == nullptr)
			{
				Error(fail = SDL_GetError());
				return true;
			}
			
			//Allocate the chunk mappings in memory
			chunks = (SDL_RWsize(mappingFile) / 2 / (8 * 8));
			chunkMapping = (CHUNKMAPPING*)malloc(sizeof(CHUNKMAPPING) * chunks);
			
			if (chunkMapping == nullptr)
			{
				Error(fail = "Failed to allocate chunk mappings in memory");
				SDL_RWclose(mappingFile);
				return true;
			}
			
			//Read the mapping data
			for (int i = 0; i < chunks; i++)
			{
				for (int v = 0; v < (8 * 8); v++)
				{
					uint16_t tmap = SDL_ReadBE16(mappingFile);
					chunkMapping[i].tile[v].altLRB	= (tmap & 0x8000) != 0;
					chunkMapping[i].tile[v].altTop	= (tmap & 0x4000) != 0;
					chunkMapping[i].tile[v].norLRB	= (tmap & 0x2000) != 0;
					chunkMapping[i].tile[v].norTop	= (tmap & 0x1000) != 0;
					chunkMapping[i].tile[v].yFlip	= (tmap & 0x0800) != 0;
					chunkMapping[i].tile[v].xFlip	= (tmap & 0x0400) != 0;
					chunkMapping[i].tile[v].tile	= (tmap & 0x3FF);
				}
			}
			
			SDL_RWclose(mappingFile);
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
	GET_GLOBAL_PATH(globalPath, tableEntry->levelReferencePath);
	GET_APPEND_PATH(layoutPath, globalPath, ".lay");
	
	SDL_RWops *layoutFile = SDL_RWFromFile(layoutPath, "rb");
	if (layoutFile == nullptr)
	{
		Error(fail = SDL_GetError());
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
				layout.width = SDL_ReadBE16(layoutFile) * 8;
				layout.height = SDL_ReadBE16(layoutFile) * 8;
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
				SDL_RWclose(layoutFile);
				return true;
			}
			
			//Read our layout file and convert to tiles
			for (int cy = 0; cy < layout.height; cy += 8)
			{
				//Read foreground line
				for (int cx = 0; cx < layout.width; cx += 8)
				{
					//Read our chunk index
					uint8_t chunk = SDL_ReadU8(layoutFile);
					
					//Get our tiles
					for (int tv = 0; tv < 8 * 8; tv++)
						layout.foreground[(cy + (tv / 8)) * layout.width + (cx + (tv % 8))] = chunkMapping[chunk].tile[tv];
				}
				
				//Skip background line (not used)
				SDL_RWseek(layoutFile, layout.width / 8, RW_SEEK_CUR);
			}
			
			SDL_RWclose(layoutFile);
			break;
		case LEVELFORMAT_TILE:
			//Get our level dimensions
			layout.width = SDL_ReadBE32(layoutFile);
			layout.height = SDL_ReadBE32(layoutFile);
			
			//Allocate our layout
			layout.foreground = (TILE*)malloc(sizeof(TILE) * layout.width * layout.height);
			
			if (layout.foreground == nullptr)
			{
				Error(fail = "Failed to allocate layout in memory");
				SDL_RWclose(layoutFile);
				return true;
			}
			
			//Read our layout file
			for (int tv = 0; tv < layout.width * layout.height; tv++)
			{
				uint16_t tmap = SDL_ReadBE16(layoutFile);
				layout.foreground[tv].altLRB	= (tmap & 0x8000) != 0;
				layout.foreground[tv].altTop	= (tmap & 0x4000) != 0;
				layout.foreground[tv].norLRB	= (tmap & 0x2000) != 0;
				layout.foreground[tv].norTop	= (tmap & 0x1000) != 0;
				layout.foreground[tv].yFlip		= (tmap & 0x0800) != 0;
				layout.foreground[tv].xFlip		= (tmap & 0x0400) != 0;
				layout.foreground[tv].tile		= (tmap & 0x3FF);
			}
			
			SDL_RWclose(layoutFile);
			break;
		default:
			Error(fail = "Unimplemented level format");
			SDL_RWclose(layoutFile);
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
	GET_GLOBAL_PATH(globalPath, tableEntry->chunkTileReferencePath);
	GET_APPEND_PATH(normalMapPath, globalPath, ".nor");
	GET_APPEND_PATH(alternateMapPath, globalPath, ".alt");
	
	SDL_RWops *norMapFile = SDL_RWFromFile(normalMapPath, "rb");
	SDL_RWops *altMapFile = SDL_RWFromFile(alternateMapPath, "rb");
	
	if (norMapFile == nullptr || altMapFile == nullptr)
	{
		Error(fail = SDL_GetError());
		if (norMapFile)
			SDL_RWclose(norMapFile);
		if (altMapFile)
			SDL_RWclose(altMapFile);
		return true;
	}
	
	//Read our tile collision map data
	tiles = SDL_RWsize(norMapFile);
	tileMapping = (TILEMAPPING*)malloc(sizeof(TILEMAPPING) * tiles);
	
	if (SDL_RWsize(norMapFile) != SDL_RWsize(altMapFile))
	{
		Error(fail = "Normal map and alternate map files don't match in size");
		if (norMapFile)
			SDL_RWclose(norMapFile);
		if (altMapFile)
			SDL_RWclose(altMapFile);
		return true;
	}
	
	for (int i = 0; i < tiles; i++)
	{
		tileMapping[i].normalColTile = SDL_ReadU8(norMapFile);
		tileMapping[i].alternateColTile = SDL_ReadU8(altMapFile);
	}
	
	SDL_RWclose(norMapFile);
	SDL_RWclose(altMapFile);
	
	//Open our collision tile files
	GET_GLOBAL_PATH(globalPath2, tableEntry->collisionReferencePath);
	GET_APPEND_PATH(collisionNormalPath, globalPath2, ".can");
	GET_APPEND_PATH(collisionRotatedPath, globalPath2, ".car");
	GET_APPEND_PATH(collisionAnglePath, globalPath2, ".ang");
	
	SDL_RWops *colNormalFile = SDL_RWFromFile(collisionNormalPath, "rb");
	SDL_RWops *colRotatedFile = SDL_RWFromFile(collisionRotatedPath, "rb");
	SDL_RWops *colAngleFile = SDL_RWFromFile(collisionAnglePath, "rb");
	
	if (colNormalFile == nullptr || colRotatedFile == nullptr || colAngleFile == nullptr)
	{
		Error(fail = SDL_GetError());
		if (colNormalFile)
			SDL_RWclose(colNormalFile);
		if (colRotatedFile)
			SDL_RWclose(colRotatedFile);
		if (colAngleFile)
			SDL_RWclose(colAngleFile);
		return true;
	}
	
	//Allocate our collision tile data in memory
	if ((SDL_RWsize(colNormalFile) != SDL_RWsize(colRotatedFile)) || (SDL_RWsize(colAngleFile) != (SDL_RWsize(colNormalFile) / 0x10)))
	{
		Error(fail = "Collision tile data file sizes don't match each-other\n(Are the files compressed?)");
		
		free(layout.foreground);
		SDL_RWclose(colNormalFile);
		SDL_RWclose(colRotatedFile);
		SDL_RWclose(colAngleFile);
		return true;
	}
	
	collisionTiles = SDL_RWsize(colNormalFile) / 0x10;
	collisionTile = (COLLISIONTILE*)malloc(sizeof(COLLISIONTILE) * collisionTiles);
	
	if (collisionTile == nullptr)
	{
		Error(fail = "Failed to allocate collision tile data in memory");
		SDL_RWclose(colNormalFile);
		SDL_RWclose(colRotatedFile);
		SDL_RWclose(colAngleFile);
		return true;
	}
	
	//Read our collision tile data
	for (int i = 0; i < collisionTiles; i++)
	{
		for (int v = 0; v < 0x10; v++)
		{
			collisionTile[i].normal[v] = SDL_ReadU8(colNormalFile);
			collisionTile[i].rotated[v] = SDL_ReadU8(colRotatedFile);
		}
		
		collisionTile[i].angle = SDL_ReadU8(colAngleFile);
	}
	
	SDL_RWclose(colNormalFile);
	SDL_RWclose(colRotatedFile);
	SDL_RWclose(colAngleFile);
	LOG(("Success!\n"));
	return false;
}

bool LEVEL::LoadObjects(LEVELTABLE *tableEntry)
{
	LOG(("Loading objects... "));
	
	//Open our object file
	GET_GLOBAL_PATH(globalPath, tableEntry->levelReferencePath);
	GET_APPEND_PATH(objectPath, globalPath, ".obj");
	
	SDL_RWops *objectFile = SDL_RWFromFile(objectPath, "rb");
	
	if (objectFile == nullptr)
	{
		Error(fail = SDL_GetError());
		return true;
	}
	
	//Read our object data
	switch (tableEntry->objectFormat)
	{
		case OBJECTFORMAT_SONIC1:
		case OBJECTFORMAT_SONIC2:
		{
			int objects = SDL_RWsize(objectFile) / 6;
			
			for (int i = 0; i < objects; i++)
			{
				int16_t xPos = SDL_ReadBE16(objectFile);
				int16_t word2 = SDL_ReadBE16(objectFile);
				int16_t yPos = word2 & 0x0FFF;
				
				uint8_t id = SDL_ReadU8(objectFile);
				uint8_t subtype = SDL_ReadU8(objectFile);
				
				if (tableEntry->objectFormat == OBJECTFORMAT_SONIC1)
					id &= 0x7F;
				
				OBJECT *newObject = new OBJECT(&objectList, tableEntry->objectFunctionList[id]);
				if (newObject == nullptr || newObject->fail)
				{
					if (newObject == nullptr)
						Error(fail = "Failed to create object");
					else
						Error(fail = newObject->fail);
					return true;
				}
				
				//Apply data
				newObject->x.pos = xPos;
				newObject->y.pos = yPos;
				
				if (tableEntry->objectFormat == OBJECTFORMAT_SONIC1)
				{
					newObject->status.yFlip = (word2 & 0x8000) != 0;
					newObject->status.xFlip = (word2 & 0x4000) != 0;
				}
				else
				{
					newObject->status.yFlip = (word2 & 0x4000) != 0;
					newObject->status.xFlip = (word2 & 0x2000) != 0;
				}
				
				newObject->subtype = subtype;
			}
		}
	}
	
	SDL_RWclose(objectFile);
	
	//Open external ring file
	GET_APPEND_PATH(ringPath, globalPath, ".ring");
	
	SDL_RWops *ringFile = SDL_RWFromFile(ringPath, "rb");
	
	if (ringFile == nullptr)
	{
		Error(fail = SDL_GetError());
		return true;
	}
	
	//Read external ring data
	int rings = SDL_RWsize(ringFile) / 4;
	
	for (int i = 0; i < rings; i++)
	{
		int16_t xPos = SDL_ReadBE16(ringFile);
		int16_t word2 = SDL_ReadBE16(ringFile);
		int16_t yPos = word2 & 0x0FFF;
		
		int type = (word2 & 0xF000) >> 12;
		
		for (int v = 0; v <= (type & 0x7); v++)
		{
			//Create ring object
			OBJECT *newObject = new OBJECT(&objectList, &ObjRing);
			if (newObject == nullptr || newObject->fail)
			{
				if (newObject == nullptr)
					Error(fail = "Failed to create object");
				else
					Error(fail = newObject->fail);
				return true;
			}
			
			newObject->x.pos = xPos;
			newObject->y.pos = yPos;
			
			//Offset
			if (type & 0x8)
				yPos += 0x18;
			else
				xPos += 0x18;
		}
	}
	
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
			//Load our background image
			GET_APPEND_PATH(backPath, tableEntry->artReferencePath, ".background.bmp");
			
			backgroundTexture = new TEXTURE(nullptr, backPath);
			if (backgroundTexture->fail)
			{
				Error(fail = backgroundTexture->fail);
				return true;
			}
			
			//Load our foreground tilemap
			GET_APPEND_PATH(artPath, tableEntry->artReferencePath, ".foreground.bmp");
			
			tileTexture = new TEXTURE(nullptr, artPath);
			if (tileTexture->fail)
			{
				Error(fail = tileTexture->fail);
				return true;
			}
			break;
		default:
			Error(fail = "Unimplemented art format");
			return true;
	}
	
	//Load background scroll
	GET_APPEND_PATH(scrollPath, tableEntry->artReferencePath, ".bsc");
	
	backgroundScroll = new BACKGROUNDSCROLL(scrollPath, backgroundTexture);
	if (backgroundScroll->fail)
	{
		Error(fail = backgroundScroll->fail);
		return true;
	}
	
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
	if (backgroundTexture != nullptr)
		delete backgroundTexture;
	if (backgroundScroll != nullptr)
		delete backgroundScroll;
	
	//Unload players, objects, and camera
	for (PLAYER *player = playerList; player != nullptr;)
	{
		PLAYER *next = player->next;
		delete player;
		player = next;
	}
	
	for (OBJECT *object = objectList; object != nullptr;)
	{
		OBJECT *next = object->next;
		delete object;
		object = next;
	}
	
	if (camera != nullptr)
		delete camera;
	if (titleCard != nullptr)
		delete titleCard;
	if (hud != nullptr)
		delete hud;
	
	//Unload object textures and mappings
	for (TEXTURE *texture = objTextureCache; texture != nullptr;)
	{
		TEXTURE *next = texture->next;
		delete texture;
		texture = next;
	}
	
	for (MAPPINGS *mappings = objMappingsCache; mappings != nullptr;)
	{
		MAPPINGS *next = mappings->next;
		delete mappings;
		mappings = next;
	}
	
	//Lock audio device so we can safely unload all loaded music
	AUDIO_LOCK;
	
	//Unload music
	if (stageMusic != nullptr)
		delete stageMusic;
	if (bossMusic != nullptr)
		delete bossMusic;
	if (goalMusic != nullptr)
		delete goalMusic;
	if (speedShoesMusic != nullptr)
		delete speedShoesMusic;
	if (invincibilityMusic != nullptr)
		delete invincibilityMusic;
	if (extraLifeMusic != nullptr)
		delete extraLifeMusic;
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

//Assets to pre-load (Assets that are loaded by objects that are usually created mid-game)
const char *preloadTexture[] = {
	"data/Object/Explosion.bmp",
	"data/Object/Shield.bmp",
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
	nullptr,
};

//Level class
LEVEL::LEVEL(int id, int players, const char **playerPaths)
{
	LOG(("Loading level ID %d...\n", id));
	memset(this, 0, sizeof(LEVEL));
	
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
	
	for (int i = 0; i < players; i++)
	{
		//Create our player
		PLAYER *newPlayer = new PLAYER(&playerList, playerPaths[i], follow, i);
		
		if (newPlayer->fail)
		{
			fail = newPlayer->fail;
			UnloadAll();
			return;
		}
		
		//Set position and set the next player to follow us
		newPlayer->x.pos = tableEntry->startX - (i * 18);
		newPlayer->y.pos = tableEntry->startY;
		follow = newPlayer;
	}
	
	//Create our camera
	camera = new CAMERA(playerList);
	
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
	extraLifeMusic = new MUSIC("ExtraLife", 0, 1.0f);
	
	//Unlock audio device
	AUDIO_UNLOCK;
	
	//Play stage music
	ChangePrimaryMusic(stageMusic);
	
	//Initialize scores
	InitializeScores();
	
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
		function(tileTexture->loadedPalette);
		function(backgroundTexture->loadedPalette);
		for (TEXTURE *texture = objTextureCache; texture != nullptr; texture = texture->next)
			function(texture->loadedPalette);
	}
}

bool LEVEL::UpdateFade()
{
	bool finished = true;
	
	bool (*function)(PALETTE *palette) = (isFadingIn ? (specialFade ? &PaletteFadeInFromWhite : &PaletteFadeInFromBlack) : (specialFade ? &PaletteFadeOutToWhite : &PaletteFadeOutToBlack));
	
	//Fade all palettes
	finished = function(tileTexture->loadedPalette) ? finished : false;
	finished = function(backgroundTexture->loadedPalette) ? finished : false;
	for (TEXTURE *texture = objTextureCache; texture != nullptr; texture = texture->next)
		finished = function(texture->loadedPalette) ? finished : false;
	
	//Fade music out
	if (currentMusic != nullptr && !isFadingIn)
	{
		//Lock audio device so we can safely modify the music's volume
		AUDIO_LOCK;
		
		//Fade music out
		currentMusic->volume = max(currentMusic->volume - (1.0f / 32.0f), 0.0f);
		
		//Unlock audio device
		AUDIO_UNLOCK;
	}
	
	return finished;
}

//Dynamic events
void LEVEL::DynamicEvents()
{
	//Get our bottom boundary
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
		if ((camera->y + 8 + gRenderSpec.height) >= bottomBoundary && playerList->status.inAir)
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
TEXTURE* LEVEL::GetObjectTexture(const char *path)
{
	for (TEXTURE *texture = objTextureCache; texture != nullptr; texture = texture->next)
	{
		if (!strcmp(texture->source, path))
			return texture;
	}
	
	return new TEXTURE(&objTextureCache, path);
}

MAPPINGS* LEVEL::GetObjectMappings(const char *path)
{
	for (MAPPINGS *mappings = objMappingsCache; mappings != nullptr; mappings = mappings->next)
	{
		if (!strcmp(mappings->source, path))
			return mappings;
	}
	
	return new MAPPINGS(&objMappingsCache, path);
}

LEVEL_RENDERLAYER LEVEL::GetObjectLayer(bool highPriority, int priority)
{
	return (LEVEL_RENDERLAYER)(highPriority ? (LEVEL_RENDERLAYER_OBJECT_HIGH_0 + priority) : (LEVEL_RENDERLAYER_OBJECT_LOW_0 + priority));
}

//Palette update and background scroll
void LEVEL::PaletteUpdate()
{
	//Handle this stage's palette cycle
	if (tileTexture == nullptr || tileTexture->loadedPalette == nullptr)
		return;
	
	switch (zone)
	{
		case ZONEID_GHZ: //Green Hill Zone
			GHZ_PaletteCycle(this);
			break;
		case ZONEID_EHZ: //Emerald Hill Zone
			EHZ_PaletteCycle(this);
			break;
	}
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
	
	//Check if we should play or not...
	if (currentMusic == nullptr || currentMusic == primaryMusic)
		SetPlayingMusic(primaryMusic = music, true, false); //If was already playing primary music, start playing this music
	else
		primaryMusic = music; //Set primary music to be this music
	
	//Update our music state
	musicIsTemporary = false;
	musicFadeAtEnd = false;
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::ChangeSecondaryMusic(MUSIC *music)
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Check if we should play or not...
	if (currentMusic == nullptr || currentMusic == primaryMusic || currentMusic == secondaryMusic)
		SetPlayingMusic(secondaryMusic = music, true, false); //If was already playing primary or secondary music (secondary music overrides primary music), start playing this music
	else
		secondaryMusic = music; //Set secondary music to be this music
	
	//Update our music state
	musicIsTemporary = true;
	musicFadeAtEnd = false;
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::PlayJingleMusic(MUSIC *music)
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Play this song and update our music state
	SetPlayingMusic(music, false, false);
	musicIsTemporary = true;
	musicFadeAtEnd = true;
	
	//Unlock audio device
	AUDIO_UNLOCK;
}

void LEVEL::StopSecondaryMusic()
{
	//Lock the audio device so we can safely change which song is playing and volume
	AUDIO_LOCK;
	
	//Check if we should play the primary song or not...
	if (currentMusic == secondaryMusic)
		SetPlayingMusic(primaryMusic, true, false);
	secondaryMusic = nullptr;
	
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
	AUDIO_UNLOCK;
	
	//If the song is temporary and has ended, resume
	if (currentMusic->playing == false && musicIsTemporary)
	{
		//Play the previous song
		if (currentMusic == secondaryMusic)
			SetPlayingMusic(primaryMusic, true, false);		//secondaryMusic to primaryMusic
		else if (secondaryMusic != nullptr)
			SetPlayingMusic(secondaryMusic, true, true);	//Jingle to secondaryMusic
		else if (primaryMusic != nullptr)
			SetPlayingMusic(primaryMusic, true, true);		//Jingle to primaryMusic
		else
			currentMusic = nullptr;
	}
	
	//Fade in the currently playing music
	if (currentMusic != nullptr)
		currentMusic->volume = min(currentMusic->volume + (1.0f / 180.0f), 1.0f);
	
	//Unlock the audio device
	AUDIO_UNLOCK;
}


//Level update and draw
bool LEVEL::Update(bool checkTitleCard)
{
	//Update the music
	UpdateMusic();
	
	//Update title card
	if (checkTitleCard)
	{
		titleCard->UpdateAndDraw();
		if (inTitleCard)
			return true;
	}
	
	//Quit if fading
	if (fading)
		return true;
	
	//Increment frame counter
	frameCounter++;
	
	//Update players
	for (PLAYER *player = playerList; player != nullptr; player = player->next)
		player->Update();
	
	if (updateStage)
	{
		//Update objects
		OBJECT *object = objectList;
		
		while (object != nullptr)
		{
			//Remember our next object
			OBJECT *next = object->next;
			
			//Update
			if (object->Update())
				return false;
			
			//Check for deletion
			if (object->deleteFlag)
			{
				for (PLAYER *player = playerList; player != nullptr; player = player->next)
					if (player->interact == object)
						player->interact = nullptr;
				delete object;
			}
			
			//Advance to next object
			object = next;
		}
	}
	
	//Update camera
	if (camera != nullptr)
		camera->Track(playerList);
	
	//Update level dynamic events
	if (playerList != nullptr)
		DynamicEvents();
	
	//Update all other level stuff
	OscillatoryUpdate();
	PaletteUpdate();
	
	//Increase our time
	if (gLevel->updateTime)
		gTime++;
	return true;
}

void LEVEL::Draw()
{
	//Draw background
	if (backgroundTexture != nullptr && backgroundScroll != nullptr && camera != nullptr)
	{
		//Get our background scroll
		int16_t backX, backY;
		backgroundScroll->GetScroll(camera->x, camera->y, &backX, &backY);
		
		//Draw each line
		int upperLine = max(backY, 0);
		int lowerLine = min(backY + gRenderSpec.height, backgroundTexture->height);
		
		SDL_Rect backSrc = {0, upperLine, backgroundTexture->width, 1};
		for (int i = upperLine; i < lowerLine; i++)
		{
			for (int x = -((backgroundScroll->scrollArray[i] + backX) % backgroundTexture->width); x < gRenderSpec.width; x += backgroundTexture->width)
				gSoftwareBuffer->DrawTexture(backgroundTexture, backgroundTexture->loadedPalette, &backSrc, LEVEL_RENDERLAYER_BACKGROUND, x, i - backY, false, false);
			backSrc.y++;
		}
	}
	
	//Draw foreground
	if (layout.foreground != nullptr && tileTexture != nullptr && camera != nullptr)
	{
		int cLeft = max(camera->x / 16, 0);
		int cTop = max(camera->y / 16, 0);
		int cRight = min((camera->x + gRenderSpec.width + 15) / 16, gLevel->layout.width - 1);
		int cBottom = min((camera->y + gRenderSpec.height + 15) / 16, gLevel->layout.height - 1);
		
		for (int ty = cTop; ty < cBottom; ty++)
		{
			for (int tx = cLeft; tx < cRight; tx++)
			{
				//Get tile
				TILE *tile = &layout.foreground[ty * layout.width + tx];
				
				if (tile->tile >= tiles || tile->tile >= tileTexture->height / 16)
					continue;
				
				//Draw tile
				SDL_Rect backSrc = {0, tile->tile * 16, 16, 16};
				SDL_Rect frontSrc = {16, tile->tile * 16, 16, 16};
				gSoftwareBuffer->DrawTexture(tileTexture, tileTexture->loadedPalette, &backSrc, LEVEL_RENDERLAYER_FOREGROUND_LOW, tx * 16 - camera->x, ty * 16 - camera->y, tile->xFlip, tile->yFlip);
				gSoftwareBuffer->DrawTexture(tileTexture, tileTexture->loadedPalette, &frontSrc, LEVEL_RENDERLAYER_FOREGROUND_HIGH, tx * 16 - camera->x, ty * 16 - camera->y, tile->xFlip, tile->yFlip);
			}
		}
	}
	
	//Draw players and objects
	for (PLAYER *player = playerList; player != nullptr; player = player->next)
		player->DrawToScreen();
	
	for (OBJECT *object = objectList; object != nullptr; object = object->next)
		object->DrawRecursive();
	
	//Draw HUD
	hud->Draw();
}
