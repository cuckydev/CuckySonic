#include "SDL_rwops.h"

#include "Level.h"
#include "Path.h"
#include "Log.h"
#include "Error.h"

LEVELTABLE gLevelTable[] = {
	//Green Hill Zone Act 1
	{LEVELFORMAT_CHUNK128_SONIC2, "data/Level/GHZ/ghz1.lay", "data/Level/GHZ/ghz.chk", "data/Level/GHZ/ghz.nor", "data/Level/GHZ/ghz.alt", "data/Level/sonic1.can", "data/Level/sonic1.car", "data/Level/sonic1.ang", 0x50, 0x3B0},
	//Spring Yard Zone Act 1
	{LEVELFORMAT_CHUNK128_SONIC2, "data/Level/SYZ/syz1.lay", "data/Level/SYZ/syz.chk", "data/Level/SYZ/syz.nor", "data/Level/SYZ/syz.alt", "data/Level/sonic1.can", "data/Level/sonic1.car", "data/Level/sonic1.ang", 0x50, 0x3B0},
	//Emerald Hill Zone Act 1
	{LEVELFORMAT_CHUNK128_SONIC2, "data/Level/EHZ/ehz1.lay", "data/Level/EHZ/ehz.chk", "data/Level/EHZ/ehz.nor", "data/Level/EHZ/ehz.alt", "data/Level/sonic2.can", "data/Level/sonic2.car", "data/Level/sonic2.ang", 0x60, 0x28F},
};

uint16_t gLevelLeftBoundary;
uint16_t gLevelRightBoundary;
uint16_t gLevelTopBoundary;
uint16_t gLevelBottomBoundary;
uint16_t gLevelBottomBoundary2;

LEVEL::LEVEL(int id)
{
	LOG(("Loading level ID %d...\n", id));
	memset(this, 0, sizeof(LEVEL));
	
	//Get our level table entry
	if (id < 0)
		Error(fail = "Invalid level ID given");
	
	LEVELTABLE *tableEntry = &gLevelTable[levelId = id];
	
	//Open our layout file
	GET_GLOBAL_PATH(layoutPath, tableEntry->layoutPath);
	
	SDL_RWops *layoutFile = SDL_RWFromFile(layoutPath, "rb");
	if (layoutFile == NULL)
	{
		Error(fail = SDL_GetError());
		return;
	}
	
	//Read our layout file
	format = tableEntry->format;
	
	switch (format)
	{
		case LEVELFORMAT_CHUNK128_SONIC2:
		case LEVELFORMAT_CHUNK128:
			//Get our level dimensions
			if (format == LEVELFORMAT_CHUNK128)
			{
				layout.width = SDL_ReadBE16(layoutFile);
				layout.height = SDL_ReadBE16(layoutFile);
			}
			else
			{
				layout.width = 0x80;
				layout.height = 0x10;
			}
			
			LOG(("Dimensions: %dx%d chunks\n", layout.width, layout.height));
			
			//Allocate our layout
			layout.foreground = (uint16_t*)malloc(sizeof(uint16_t) * layout.width * layout.height);
			layout.background = (uint16_t*)malloc(sizeof(uint16_t) * layout.width * layout.height);
			if (layout.foreground == NULL || layout.background == NULL)
			{
				Error(fail = "Failed to allocate layout in memory");
				
				free(layout.foreground);
				free(layout.background);
				SDL_RWclose(layoutFile);
				return;
			}
			
			//Read data from the file
			for (int line = 0; line < layout.height; line++)
			{
				for (int fx = 0; fx < layout.width; fx++)
					layout.foreground[line * layout.width + fx] = SDL_ReadU8(layoutFile);
				for (int bx = 0; bx < layout.width; bx++)
					layout.background[line * layout.width + bx] = SDL_ReadU8(layoutFile);
			}
			
			SDL_RWclose(layoutFile);
			break;
		default:
			Error(fail = "Unimplemented level format");
			
			SDL_RWclose(layoutFile);
			return;
	}
	
	switch (format)
	{
		case LEVELFORMAT_CHUNK128_SONIC2:
		case LEVELFORMAT_CHUNK128:
		{
			//Open our chunk mapping file
			GET_GLOBAL_PATH(mappingPath, tableEntry->mappingPath);
			
			SDL_RWops *mappingFile = SDL_RWFromFile(mappingPath, "rb");
			if (mappingFile == NULL)
			{
				Error(fail = SDL_GetError());
				
				free(layout.foreground);
				free(layout.background);
				return;
			}
			
			//Allocate the chunk mappings in memory
			const int chunks = SDL_RWsize(mappingFile) / 2 / (8 * 8);
			chunkMapping = (CHUNKMAPPING*)malloc(sizeof(CHUNKMAPPING) * chunks);
			
			LOG(("Chunks: %d\n", chunks));
			
			if (chunkMapping == NULL)
			{
				Error(fail = "Failed to allocate chunk mappings in memory");
				
				free(layout.foreground);
				free(layout.background);
				SDL_RWclose(mappingFile);
				return;
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
					chunkMapping[i].tile[v].tile	= tmap & 0x3FF;
				}
			}
			
			SDL_RWclose(mappingFile);
			break;
		}
		
		default:
			LOG(("This format doesn't use chunk mappings\n"));
			break;
	}
	
	//Open our collision tile map files
	GET_GLOBAL_PATH(normalMapPath, tableEntry->normalMapPath);
	GET_GLOBAL_PATH(alternateMapPath, tableEntry->alternateMapPath);
	
	SDL_RWops *norMapFile = SDL_RWFromFile(normalMapPath, "rb");
	SDL_RWops *altMapFile = SDL_RWFromFile(alternateMapPath, "rb");
	
	if (norMapFile == NULL || altMapFile == NULL)
	{
		Error(fail = SDL_GetError());
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		if (norMapFile)
			SDL_RWclose(norMapFile);
		if (altMapFile)
			SDL_RWclose(altMapFile);
		return;
	}
	
	//Allocate our maps in memory
	if (SDL_RWsize(norMapFile) != SDL_RWsize(altMapFile))
	{
		Error(fail = "Normal collision tile map file size is different from the alternate one\n(Are the files compressed?)");
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		SDL_RWclose(norMapFile);
		SDL_RWclose(altMapFile);
		return;
	}
	
	const int tiles = SDL_RWsize(norMapFile);
	normalMap = (uint16_t*)malloc(sizeof(uint16_t) * tiles);
	alternateMap = (uint16_t*)malloc(sizeof(uint16_t) * tiles);
	
	LOG(("Tiles: %d\n", tiles));
	
	if (normalMap == NULL || alternateMap == NULL)
	{
		Error(fail = "Failed to allocate collision tile maps in memory");
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		free(normalMap);
		free(alternateMap);
		SDL_RWclose(norMapFile);
		SDL_RWclose(altMapFile);
		return;
	}
	
	//Read our collision tile map data
	for (int i = 0; i < tiles; i++)
	{
		normalMap[i] = SDL_ReadU8(norMapFile);
		alternateMap[i] = SDL_ReadU8(altMapFile);
	}
	
	SDL_RWclose(norMapFile);
	SDL_RWclose(altMapFile);
	
	//Open our collision tile files
	GET_GLOBAL_PATH(collisionNormalPath, tableEntry->collisionNormalPath);
	GET_GLOBAL_PATH(collisionRotatedPath, tableEntry->collisionRotatedPath);
	GET_GLOBAL_PATH(collisionAnglePath, tableEntry->collisionAnglePath);
	
	SDL_RWops *colNormalFile = SDL_RWFromFile(collisionNormalPath, "rb");
	SDL_RWops *colRotatedFile = SDL_RWFromFile(collisionRotatedPath, "rb");
	SDL_RWops *colAngleFile = SDL_RWFromFile(collisionAnglePath, "rb");
	
	if (colNormalFile == NULL || colRotatedFile == NULL || colAngleFile == NULL)
	{
		Error(fail = SDL_GetError());
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		free(normalMap);
		free(alternateMap);
		if (colNormalFile)
			SDL_RWclose(colNormalFile);
		if (colRotatedFile)
			SDL_RWclose(colRotatedFile);
		if (colAngleFile)
			SDL_RWclose(colAngleFile);
		return;
	}
	
	//Allocate our collision tile data in memory
	if ((SDL_RWsize(colNormalFile) != SDL_RWsize(colRotatedFile)) || (SDL_RWsize(colAngleFile) != (SDL_RWsize(colNormalFile) / 0x10)))
	{
		Error(fail = "Collision tile data file sizes don't match each-other\n(Are the files compressed?)");
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		free(normalMap);
		free(alternateMap);
		SDL_RWclose(colNormalFile);
		SDL_RWclose(colRotatedFile);
		SDL_RWclose(colAngleFile);
		return;
	}
	
	const int collisionTiles = SDL_RWsize(colNormalFile) / 0x10;
	collisionTile = (COLLISIONTILE*)malloc(sizeof(COLLISIONTILE) * collisionTiles);
	
	LOG(("Collision tiles: %d\n", collisionTiles));
	
	if (collisionTile == NULL)
	{
		Error(fail = "Failed to allocate collision tile data in memory");
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		free(normalMap);
		free(alternateMap);
		SDL_RWclose(colNormalFile);
		SDL_RWclose(colRotatedFile);
		SDL_RWclose(colAngleFile);
		return;
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
	
	//Load our tile art (TEMP: COLLISION)
	PALETTE *newPalette = new PALETTE;
	uint8_t *texData = (uint8_t*)malloc(sizeof(uint8_t) * tiles * (16 * 16));
	if (texData == NULL || newPalette == NULL)
	{
		Error(fail = "Failed to allocate art data in memory");
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		free(normalMap);
		free(alternateMap);
		free(collisionTile);
		free(texData);
		if (newPalette)
			delete newPalette;
		return;
	}
	
	SetPaletteColour(&newPalette->colour[1], 0xFF, 0xFF, 0xFF); //Both solid
	SetPaletteColour(&newPalette->colour[2], 0x80, 0x80, 0x80); //Normal solid
	SetPaletteColour(&newPalette->colour[3], 0x40, 0x40, 0x40); //Alternate solid
	SetPaletteColour(&newPalette->colour[4], 0x00, 0xFF, 0x00); //Collision debug
	
	for (int tile = 0; tile < tiles; tile++)
	{
		uint8_t *tilePointer = texData + (sizeof(uint8_t) * tile * (16 * 16));
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				bool norContact = false;
				bool altContact = false;
				COLLISIONTILE *norTile = &collisionTile[normalMap[tile]];
				COLLISIONTILE *altTile = &collisionTile[alternateMap[tile]];
				
				//Get normal contact
				if (norTile->normal[x] > 0 && y >= (0x10 - norTile->normal[x]))
					norContact = true;
				else if (norTile->normal[x] < 0 && norTile->normal[x] + y < 0)
					norContact = true;
				if (norTile->rotated[y] > 0 && x >= (0x10 - norTile->rotated[y]))
					norContact = true;
				else if (norTile->rotated[y] < 0 && norTile->rotated[y] + x < 0)
					norContact = true;
				
				//Get alternate contact
				if (altTile->normal[x] > 0 && y >= (0x10 - altTile->normal[x]))
					altContact = true;
				else if (altTile->normal[x] < 0 && altTile->normal[x] + y < 0)
					altContact = true;
				if (altTile->rotated[y] > 0 && x >= (0x10 - altTile->rotated[y]))
					altContact = true;
				else if (altTile->rotated[y] < 0 && altTile->rotated[y] + x < 0)
					altContact = true;
				
				//Write our pixel
				if (norContact == false && altContact == false)
					*tilePointer = 0;
				else if (norContact == true && altContact == true)
					*tilePointer = 1;
				else if (norContact == true && altContact == false)
					*tilePointer = 2;
				else if (norContact == false && altContact == true)
					*tilePointer = 3;
				tilePointer++;
			}
		}
	}
	
	tileTexture = new TEXTURE(texData, 16, tiles * 16);
	if (tileTexture->fail)
	{
		Error(fail = tileTexture->fail);
		
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		free(normalMap);
		free(alternateMap);
		free(collisionTile);
		free(texData);
		delete newPalette;
		return;
	}
	
	tileTexture->loadedPalette = newPalette;
	free(texData);
	
	//Set level boundaries
	gLevelLeftBoundary = 0;
	gLevelTopBoundary = 0;
	
	switch (format)
	{
		case LEVELFORMAT_CHUNK128_SONIC2:
		case LEVELFORMAT_CHUNK128:
			gLevelRightBoundary = layout.width * 128;
			gLevelBottomBoundary = layout.height * 128;
			gLevelBottomBoundary2 = gLevelBottomBoundary;
			break;
		default:
			gLevelRightBoundary = 0x7FFF;
			gLevelBottomBoundary = 0x7FFF;
			gLevelBottomBoundary2 = gLevelBottomBoundary;
			break;
	}
	
	//Create our players
	PLAYER *follow = NULL;
	
	for (int i = 0; i < PLAYERS; i++)
	{
		player[i] = new PLAYER("data/Sonic/sonic", follow, 0);
		
		if (player[i]->fail)
		{
			Error(fail = player[i]->fail);
		
			free(layout.foreground);
			free(layout.background);
			free(chunkMapping);
			free(normalMap);
			free(alternateMap);
			free(collisionTile);
			delete tileTexture;
			
			for (int v = 0; v < i; v++)
				delete player[v];
			return;
		}
		
		player[i]->x.pos = tableEntry->startX - (i * 18);
		player[i]->y.pos = tableEntry->startY;
		follow = player[i];
	}
	
	//Load our objects
	
	//Create our camera
	camera = new CAMERA(player[0]);
	if (camera == NULL)
	{
		Error(fail = "Failed to create our camera");
	
		free(layout.foreground);
		free(layout.background);
		free(chunkMapping);
		free(normalMap);
		free(alternateMap);
		free(collisionTile);
		delete tileTexture;
		
		for (int i = 0; i < PLAYERS; i++)
			delete player[i];
		return;
	}
	
	//Handle dynamic boundaries
	DynamicBoundaries();
	gLevelBottomBoundary2 = gLevelBottomBoundary;
	
	LOG(("Success!\n"));
}

LEVEL::~LEVEL()
{
	LOG(("Unloading level... "));
	
	//Free our level data
	free(layout.foreground);
	free(layout.background);
	free(chunkMapping);
	free(normalMap);
	free(alternateMap);
	free(collisionTile);
	delete tileTexture;
	
	//Free players and objects
	for (int i = 0; i < PLAYERS; i++)
		delete player[i];
	delete camera;
	
	LOG(("Success!\n"));
}

void LEVEL::DynamicBoundaries()
{
	//Get our bottom boundary
	switch (levelId)
	{
		case 0: //Green Hill Zone Act 1
			if (camera->x < 0x1780)
				gLevelBottomBoundary = 0x3E0;
			else
				gLevelBottomBoundary = 0x4E0;
			break;
		default:
			break;
	}
	
	//Move up/down to the boundary
	int16_t move = 2;

	if (gLevelBottomBoundary < gLevelBottomBoundary2)
	{
		//Move up to the boundary smoothly
		if ((camera->y + SCREEN_HEIGHT) > gLevelBottomBoundary)
			gLevelBottomBoundary2 = (camera->y + SCREEN_HEIGHT);
		
		//Move
		gLevelBottomBoundary2 -= move;
	}
	else if (gLevelBottomBoundary > gLevelBottomBoundary2)
	{
		//Move faster if in mid-air
		if ((camera->y + 8 + SCREEN_HEIGHT) >= gLevelBottomBoundary2 && player[0]->status.inAir)
			move *= 4;
		
		//Move
		gLevelBottomBoundary2 += move;
	}
}

void LEVEL::Update()
{
	//Update players
	for (int i = 0; i < PLAYERS; i++)
		player[i]->Update();
	
	//Update camera
	camera->Track(player[0]);
	
	//Update level boundaries
	DynamicBoundaries();
}

void LEVEL::Draw()
{
	//Draw stage (temp)
	switch (format)
	{
		case LEVELFORMAT_CHUNK128_SONIC2:
		case LEVELFORMAT_CHUNK128:
		{
			int chunkLeft = camera->x / 128;
			int chunkTop = camera->y / 128;
			int chunkRight = (camera->x + SCREEN_WIDTH) / 128;
			int chunkBottom = (camera->y + SCREEN_HEIGHT) / 128;
			
			for (int x = chunkLeft; x <= chunkRight; x++)
			{
				if (x < 0 || x >= layout.width)
					continue;
				
				for (int y = chunkTop; y <= chunkBottom; y++)
				{
					if (y < 0 || y >= layout.height)
						continue;
					
					for (int tx = 0; tx < 8; tx++)
					{
						for (int ty = 0; ty < 8; ty++)
						{
							CHUNKMAPPINGTILE *tile = &chunkMapping[layout.foreground[y * layout.width + x]].tile[ty * 8 + tx];
							SDL_Rect tileSrc = {0, tile->tile * 16, 16, 16};
							tileTexture->Draw(tileTexture->loadedPalette, &tileSrc, x * 128 + tx * 16 - camera->x, y * 128 + ty * 16 - camera->y, tile->xFlip, tile->yFlip);
						}
					}
				}
			}
			break;
		}
		default:
			LOG(("Unhandled format for drawing\n"));
			break;
	}
	
	//Draw players (we draw backwards, we want the leader to be drawn first)
	for (int i = PLAYERS - 1; i >= 0; i--)
		player[i]->Draw();
}
