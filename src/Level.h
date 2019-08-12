#pragma once
#include <stdint.h>
#include "Render.h"

enum LEVELFORMAT
{
	LEVELFORMAT_CHUNK128,
	LEVELFORMAT_CHUNK128_SONIC2,
};

struct LEVELTABLE
{
	//Level data format
	LEVELFORMAT format;
	
	//Paths
	const char *layoutPath;
	const char *mappingPath;
	const char *normalMapPath;
	const char *alternateMapPath;
	const char *collisionNormalPath;
	const char *collisionRotatedPath;
	const char *angleMapPath;
};

struct CHUNKMAPPING
{
	struct
	{
		//Solidity
		bool altLRB : 1;
		bool altTop : 1;
		bool norLRB : 1;
		bool norTop : 1;
		bool yFlip : 1;
		bool xFlip : 1;
		uint16_t tile : 10;
	} tile[8 * 8];
};

class LEVEL
{
	public:
		const char *fail;
		
		//Tile texture
		TEXTURE *tileTexture;
		
		//Layout data
		LEVELFORMAT format;
		
		int layoutWidth;
		int layoutHeight;
		uint16_t *layoutForeground;
		uint16_t *layoutBackground;
		
		//Chunk mapping
		CHUNKMAPPING *chunkMapping;
		
		//Tile to collision maps
		uint16_t *normalMap;
		uint16_t *alternateMap;
		
	public:
		LEVEL(int id);
		~LEVEL();
		
		void Update();
		void Draw();
};
