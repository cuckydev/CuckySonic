#include "SDL_rwops.h"

#include "Level.h"
#include "Path.h"
#include "Log.h"
#include "Error.h"

LEVELTABLE gLevelTable[] = {
	//Green Hill Zone Act 1
	{LEVELFORMAT_CHUNK128_SONIC2, "data/Level/GHZ/ghz1.lay", "data/Level/GHZ/ghz.chk", "data/Level/GHZ/ghz.nor", "data/Level/GHZ/ghz.alt", "data/Level/sonic1.can", "data/Level/sonic1.car", "data/Level/sonic1.ang"},
};

LEVEL::LEVEL(int id)
{
	LOG(("Loading level ID %d...\n", id));
	memset(this, 0, sizeof(LEVEL));
	
	//Get our level table entry
	if (id < 0)
		Error(fail = "Invalid level ID given");
	
	LEVELTABLE *tableEntry = &gLevelTable[id];
	
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
				layoutWidth = SDL_ReadBE16(layoutFile);
				layoutHeight = SDL_ReadBE16(layoutFile);
			}
			else
			{
				layoutWidth = 0x80;
				layoutHeight = 0x10;
			}
			
			LOG(("Dimensions: %dx%d chunks\n", layoutWidth, layoutHeight));
			
			//Allocate our layout
			layoutForeground = (uint16_t*)malloc(sizeof(uint16_t) * layoutWidth * layoutHeight);
			layoutBackground = (uint16_t*)malloc(sizeof(uint16_t) * layoutWidth * layoutHeight);
			if (layoutForeground == NULL || layoutBackground == NULL)
			{
				Error(fail = "Failed to allocate layout in memory");
				free(layoutForeground);
				free(layoutBackground);
				SDL_RWclose(layoutFile);
				return;
			}
			
			//Read data from the file
			for (int line = 0; line < layoutHeight; line++)
			{
				for (int fx = 0; fx < layoutWidth; fx++)
					layoutForeground[line * layoutHeight + fx] = SDL_ReadU8(layoutFile);
				for (int bx = 0; bx < layoutWidth; bx++)
					layoutBackground[line * layoutHeight + bx] = SDL_ReadU8(layoutFile);
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
				
				free(layoutForeground);
				free(layoutBackground);
				return;
			}
			
			//Allocate the chunk mappings in memory
			int chunks = SDL_RWsize(mappingFile) / 2 / (8 * 8);
			chunkMapping = (CHUNKMAPPING*)malloc(sizeof(CHUNKMAPPING) * chunks);
			
			LOG(("Chunks: %d\n", chunks));
			
			if (chunkMapping == NULL)
			{
				Error(fail = "Failed to allocate chunk mappings in memory");
				
				free(layoutForeground);
				free(layoutBackground);
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
		
		free(layoutForeground);
		free(layoutBackground);
		free(chunkMapping);
		if (norMapFile)
			SDL_RWclose(norMapFile);
		if (altMapFile)
			SDL_RWclose(altMapFile);
		return;
	}
	
	//Read our collision tile map
	if (SDL_RWsize(norMapFile) != SDL_RWsize(altMapFile))
	{
		Error(fail = "Normal collision tile map file size is different from the alternate one\n(Are the files compressed?)");
		
		free(layoutForeground);
		free(layoutBackground);
		free(chunkMapping);
		SDL_RWclose(norMapFile);
		SDL_RWclose(altMapFile);
		return;
	}
	
	LOG(("Success!\n"));
}

LEVEL::~LEVEL()
{
	LOG(("Unloading level... "));
	LOG(("Success!\n"));
}
