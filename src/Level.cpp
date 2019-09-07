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
	NULL, NULL, NULL, &ObjPathSwitcher, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, &ObjGoalpost, NULL, NULL,
	NULL, &ObjBridge, NULL, NULL, NULL, NULL, NULL, NULL,
	&ObjGHZPlatform, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, &ObjRingSpawner, &ObjMonitor, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	&ObjMotobug, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

OBJECTFUNCTION objFuncSonic2[] = {
	NULL, NULL, NULL, &ObjPathSwitcher, NULL, NULL, &ObjSpiral, NULL,
	NULL, NULL, NULL, NULL, NULL, &ObjGoalpost, NULL, NULL,
	NULL, &ObjBridge, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, &ObjMonitor, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
};

//Our level table
LEVELTABLE gLevelTable[] = {
	{"Green Hill Zone", "Act 1", LEVELFORMAT_CHUNK128_SONIC2, ARTFORMAT_BMP, "data/Level/GHZ/ghz1", "data/Level/GHZ/ghz", "data/Level/sonic1", "data/Level/GHZ/ghz", MUSICID_GHZ, objFuncSonic1, 0x0050, 0x03B0, 0x0000, 0x25FF + (SCREEN_WIDTH - 320) / 2, 0x0000, 0x03E0},
	{"Emerald Hill Zone", "Act 1", LEVELFORMAT_CHUNK128_SONIC2, ARTFORMAT_BMP, "data/Level/EHZ/ehz1", "data/Level/EHZ/ehz", "data/Level/sonic2", "data/Level/EHZ/ehz", MUSICID_EHZ, objFuncSonic2, 0x0060, 0x028F, 0x0000, 0x2AE0 + (SCREEN_WIDTH - 320) / 2, 0x0000, 0x0400},
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
			if (mappingFile == NULL)
			{
				Error(fail = SDL_GetError());
				return true;
			}
			
			//Allocate the chunk mappings in memory
			chunks = (SDL_RWsize(mappingFile) / 2 / (8 * 8));
			chunkMapping = (CHUNKMAPPING*)malloc(sizeof(CHUNKMAPPING) * chunks);
			
			if (chunkMapping == NULL)
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
	if (layoutFile == NULL)
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
			
			if (layout.foreground == NULL)
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
			
			if (layout.foreground == NULL)
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
	rightBoundary = tableEntry->rightBoundary;
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
	
	if (norMapFile == NULL || altMapFile == NULL)
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
	
	if (colNormalFile == NULL || colRotatedFile == NULL || colAngleFile == NULL)
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
	
	if (collisionTile == NULL)
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
	
	if (objectFile == NULL)
	{
		Error(fail = SDL_GetError());
		return true;
	}
	
	//Read our object data
	int objects = SDL_RWsize(objectFile) / 6;
	
	for (int i = 0; i < objects; i++)
	{
		int16_t xPos = SDL_ReadBE16(objectFile);
		int16_t word2 = SDL_ReadBE16(objectFile);
		int16_t yPos = word2 & 0x0FFF;
		
		uint8_t id = SDL_ReadU8(objectFile);
		uint8_t subtype = SDL_ReadU8(objectFile);
		
		OBJECT *newObject = new OBJECT(&objectList, tableEntry->objectFunctionList[id & 0x7F]);
		if (newObject == NULL || newObject->fail)
		{
			if (newObject == NULL)
				Error(fail = "Failed to create object");
			else
				Error(fail = newObject->fail);
			return true;
		}
		
		//Apply data
		newObject->x.pos = xPos;
		newObject->y.pos = yPos;
		newObject->status.xFlip = (word2 & 0x8000) != 0;
		newObject->status.yFlip = (word2 & 0x4000) != 0;
		newObject->subtype = subtype;
	}
	
	SDL_RWclose(objectFile);
	
	//Open external ring file
	GET_APPEND_PATH(ringPath, globalPath, ".ring");
	
	SDL_RWops *ringFile = SDL_RWFromFile(ringPath, "rb");
	
	if (ringFile == NULL)
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
			if (newObject == NULL || newObject->fail)
			{
				if (newObject == NULL)
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
			
			backgroundTexture = new TEXTURE(NULL, backPath);
			if (backgroundTexture == NULL || backgroundTexture->fail)
			{
				Error(fail = backgroundTexture->fail);
				return true;
			}
			
			//Allocate background scroll array
			backgroundScroll = (uint16_t*)malloc(sizeof(uint16_t) * backgroundTexture->height);
			
			//Load our foreground tilemap
			GET_APPEND_PATH(artPath, tableEntry->artReferencePath, ".foreground.bmp");
			
			tileTexture = new TEXTURE(NULL, artPath);
			if (tileTexture == NULL || tileTexture->fail)
			{
				Error(fail = tileTexture->fail);
				return true;
			}
			break;
		default:
			Error(fail = "Unimplemented art format");
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
	if (tileTexture != NULL)
		delete tileTexture;
	if (backgroundTexture != NULL)
		delete backgroundTexture;
	
	//Unload players, objects, and camera
	for (PLAYER *player = playerList; player != NULL;)
	{
		PLAYER *next = player->next;
		delete player;
		player = next;
	}
	
	for (OBJECT *object = objectList; object != NULL;)
	{
		OBJECT *next = object->next;
		delete object;
		object = next;
	}
	
	if (camera != NULL)
		delete camera;
	if (titleCard != NULL)
		delete titleCard;
	if (hud != NULL)
		delete hud;
	
	//Unload object textures and mappings
	for (TEXTURE *texture = objTextureCache; texture != NULL;)
	{
		TEXTURE *next = texture->next;
		delete texture;
		texture = next;
	}
	
	for (MAPPINGS *mappings = objMappingsCache; mappings != NULL;)
	{
		MAPPINGS *next = mappings->next;
		delete mappings;
		mappings = next;
	}
}

//Assets to pre-load (Assets that are loaded by objects that are usually created mid-game)
const char *preloadTexture[] = {
	"data/Object/Explosion.bmp",
	NULL,
};

const char *preloadMappings[] = {
	"data/Object/Explosion.map",
	NULL,
};

//Level class
LEVEL::LEVEL(int id, int players, const char **playerPaths)
{
	LOG(("Loading level ID %d...\n", id));
	memset(this, 0, sizeof(LEVEL));
	
	//Set us as the global level
	gLevel = this;
	
	//Get our level table entry
	if (id < 0)
		Error(fail = "Invalid level ID given");
	
	LEVELTABLE *tableEntry = &gLevelTable[levelId = id];
	
	//Load data
	if (LoadMappings(tableEntry) || LoadLayout(tableEntry) || LoadCollisionTiles(tableEntry) || LoadObjects(tableEntry) || LoadArt(tableEntry))
	{
		//Unload any loaded data
		UnloadAll();
		return;
	}
	
	//Pre-load assets
	for (int i = 0; preloadTexture[i] != NULL; i++)
		GetObjectTexture(preloadTexture[i]);
	for (int i = 0; preloadMappings[i] != NULL; i++)
		GetObjectMappings(preloadMappings[i]);
	
	//Create our players
	PLAYER *follow = NULL;
	
	for (int i = 0; i < players; i++)
	{
		//Create our player
		PLAYER *newPlayer = new PLAYER(&playerList, playerPaths[i], follow, i);
		
		if (newPlayer->fail)
		{
			Error(fail = newPlayer->fail);
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
	if (camera == NULL)
	{
		Error(fail = "Failed to create our camera");
		UnloadAll();
		return;
	}
	
	//Title-card
	titleCard = new TITLECARD(tableEntry->name, tableEntry->subtitle);
	if (titleCard == NULL || titleCard->fail)
	{
		Error(fail = titleCard->fail);
		UnloadAll();
		return;
	}
	
	inTitleCard = true;
	
	//HUD
	hud = new HUD();
	if (hud == NULL || hud->fail)
	{
		Error(fail = hud->fail);
		UnloadAll();
		return;
	}
	
	//Initialize state
	OscillatoryInit();
	
	updateTime = true;
	updateStage = true;
	
	//Play music
	PlayMusic(musicId = tableEntry->music);
	
	//Clear time and rings
	gTime = 0;
	gRings = 0;
	
	LOG(("Success!\n"));
}

LEVEL::~LEVEL()
{
	LOG(("Unloading level... "));
	
	//Unload data
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
		if (isSpecial)
		{
			FillPaletteWhite(tileTexture->loadedPalette);
			FillPaletteWhite(backgroundTexture->loadedPalette);
			for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
				FillPaletteWhite(texture->loadedPalette);
		}
		else
		{
			FillPaletteBlack(tileTexture->loadedPalette);
			FillPaletteBlack(backgroundTexture->loadedPalette);
			for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
				FillPaletteBlack(texture->loadedPalette);
		}
	}
}

bool LEVEL::UpdateFade()
{
	bool finished = true;
	
	if (isFadingIn)
	{
		//Fade out all of our known palettes
		if (specialFade)
		{
			finished = PaletteFadeInFromWhite(tileTexture->loadedPalette) ? finished : false;
			finished = PaletteFadeInFromWhite(backgroundTexture->loadedPalette) ? finished : false;
			for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
				finished = PaletteFadeInFromWhite(texture->loadedPalette) ? finished : false;
		}
		else
		{
			finished = PaletteFadeInFromBlack(tileTexture->loadedPalette) ? finished : false;
			finished = PaletteFadeInFromBlack(backgroundTexture->loadedPalette) ? finished : false;
			for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
				finished = PaletteFadeInFromBlack(texture->loadedPalette) ? finished : false;
		}
	}
	else
	{
		//Fade out all of our known palettes
		if (specialFade)
		{
			finished = PaletteFadeOutToWhite(tileTexture->loadedPalette) ? finished : false;
			finished = PaletteFadeOutToWhite(backgroundTexture->loadedPalette) ? finished : false;
			for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
				finished = PaletteFadeOutToWhite(texture->loadedPalette) ? finished : false;
		}
		else
		{
			finished = PaletteFadeOutToBlack(tileTexture->loadedPalette) ? finished : false;
			finished = PaletteFadeOutToBlack(backgroundTexture->loadedPalette) ? finished : false;
			for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
				finished = PaletteFadeOutToBlack(texture->loadedPalette) ? finished : false;
		}
		
		//Fade the music out
		SetMusicVolume(max(GetMusicVolume() - (1.0f / 32.0f), 0.0f));
	}
	
	return finished;
}

//Dynamic events
void LEVEL::DynamicEvents()
{
	//Get our bottom boundary
	switch (levelId)
	{
		case 0: //Green Hill Zone
			if (levelId == 0)
			{
				//Update boundaries (GHZ1)
				if (camera->x < 0x1780)
					bottomBoundaryTarget = 0x3E0;
				else
					bottomBoundaryTarget = 0x4E0;
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
		if ((camera->y + SCREEN_HEIGHT) > bottomBoundaryTarget)
			bottomBoundary = (camera->y + SCREEN_HEIGHT);
		
		//Move
		bottomBoundary -= move;
		if (bottomBoundaryTarget > bottomBoundary)
			bottomBoundary = bottomBoundaryTarget;
	}
	else if (bottomBoundaryTarget > bottomBoundary)
	{
		//Move faster if in mid-air
		if ((camera->y + 8 + SCREEN_HEIGHT) >= bottomBoundary && playerList->status.inAir)
			move *= 4;
		
		//Move
		bottomBoundary += move;
		if (bottomBoundaryTarget < bottomBoundary)
			bottomBoundary = bottomBoundaryTarget;
	}
	
	//Set boundaries to target
	int16_t left = camera->x;
	int16_t right = camera->x + SCREEN_WIDTH;
	
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

//Texture cache
TEXTURE* LEVEL::GetObjectTexture(const char *path)
{
	for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
	{
		if (!strcmp(texture->source, path))
			return texture;
	}
	
	return new TEXTURE(&objTextureCache, path);
}

TEXTURE* LEVEL::GetObjectTexture(uint8_t *data, int dWidth, int dHeight)
{
	for (TEXTURE *texture = objTextureCache; texture != NULL; texture = texture->next)
	{
		if (texture->width == dWidth && texture->height == dHeight && !memcmp(texture->texture, data, dWidth * dHeight))
			return texture;
	}
	
	return new TEXTURE(&objTextureCache, data, dWidth, dHeight);
}

//Mappings cache
MAPPINGS* LEVEL::GetObjectMappings(const char *path)
{
	for (MAPPINGS *mappings = objMappingsCache; mappings != NULL; mappings = mappings->next)
	{
		if (!strcmp(mappings->source, path))
			return mappings;
	}
	
	return new MAPPINGS(&objMappingsCache, path);
}

//Palette update and background scroll
void LEVEL::PaletteUpdate()
{
	//Handle this stage's palette cycle
	if (tileTexture == NULL || tileTexture->loadedPalette == NULL)
		return;
	
	switch (levelId)
	{
		case 0: //Green Hill Zone
			GHZ_PaletteCycle(this);
			break;
		case 1: //Emerald Hill Zone
			EHZ_PaletteCycle(this);
			break;
	}
}

void LEVEL::GetBackgroundScroll(bool updateScroll, uint16_t *array, int16_t *cameraX, int16_t *cameraY)
{
	switch (levelId)
	{
		case 0: //Green Hill Zone
			GHZ_BackgroundScroll(updateScroll, array, cameraX, cameraY);
			break;
		case 1: //Emerald Hill Zone
			EHZ_BackgroundScroll(updateScroll, array, cameraX, cameraY);
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

//Level update and draw
bool LEVEL::Update(bool checkTitleCard)
{
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
	for (PLAYER *player = playerList; player != NULL; player = player->next)
		player->Update();
	
	if (updateStage)
	{
		//Update objects
		OBJECT *object = objectList;
		
		while (object != NULL)
		{
			//Remember our next object
			OBJECT *next = object->next;
			
			//Update
			if (object->Update())
				return false;
			
			//Check for deletion
			if (object->deleteFlag)
			{
				for (PLAYER *player = playerList; player != NULL; player = player->next)
					if (player->interact == object)
						player->interact = NULL;
				delete object;
			}
			
			//Advance to next object
			object = next;
		}
	}
	
	//Update camera
	if (camera != NULL)
		camera->Track(playerList);
	
	//Update level dynamic events
	if (playerList != NULL)
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
	if (backgroundTexture != NULL && camera != NULL)
	{
		//Get our background scroll
		int16_t backX = camera->x;
		int16_t backY = camera->y;
		
		GetBackgroundScroll(updateStage, backgroundScroll, &backX, &backY);
		
		//Draw each line
		for (int i = 0; i < backgroundTexture->height; i++)
		{
			for (int x = -(backgroundScroll[i] % backgroundTexture->width); x < SCREEN_WIDTH; x += backgroundTexture->width)
			{
				SDL_Rect backSrc = {0, i, backgroundTexture->width, 1};
				backgroundTexture->Draw(LEVEL_RENDERLAYER_BACKGROUND, backgroundTexture->loadedPalette, &backSrc, x, -backY + i, false, false);
			}
		}
	}
	
	//Draw foreground
	if (layout.foreground != NULL && tileTexture != NULL && camera != NULL)
	{
		int cLeft = max(camera->x / 16, 0);
		int cTop = max(camera->y / 16, 0);
		int cRight = min((camera->x + SCREEN_WIDTH) / 16, gLevel->layout.width - 1);
		int cBottom = min((camera->y + SCREEN_HEIGHT) / 16, gLevel->layout.height - 1);
		
		for (int ty = cTop; ty <= cBottom; ty++)
		{
			for (int tx = cLeft; tx <= cRight; tx++)
			{
				//Get tile
				TILE *tile = &layout.foreground[ty * layout.width + tx];
				
				if (tile->tile >= tiles || tile->tile >= tileTexture->height / 16)
					continue;
				
				//Draw tile
				SDL_Rect backSrc = {0, 16 * tile->tile, 16, 16};
				SDL_Rect frontSrc = {16, 16 * tile->tile, 16, 16};
				tileTexture->Draw(LEVEL_RENDERLAYER_FOREGROUND_LOW, tileTexture->loadedPalette, &backSrc, tx * 16 - camera->x, ty * 16 - camera->y, tile->xFlip, tile->yFlip);
				tileTexture->Draw(LEVEL_RENDERLAYER_FOREGROUND_HIGH, tileTexture->loadedPalette, &frontSrc, tx * 16 - camera->x, ty * 16 - camera->y, tile->xFlip, tile->yFlip);
			}
		}
	}
	
	//Draw players and objects
	for (PLAYER *player = playerList; player != NULL; player = player->next)
		player->DrawToScreen();
	
	for (OBJECT *object = objectList; object != NULL; object = object->next)
		object->DrawRecursive();
	
	//Draw HUD
	hud->Draw();
}
