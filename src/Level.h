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
	const char *collisionAnglePath;
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

struct LAYOUT
{
	int width;
	int height;
	uint16_t *foreground;
	uint16_t *background;
};

struct COLLISIONTILE
{
	int8_t normal[0x10];
	int8_t rotated[0x10];
	uint8_t angle;
};

class LEVEL
{
	public:
		const char *fail;
		
		//Tile texture
		TEXTURE *tileTexture;
		
		//Layout data
		LEVELFORMAT format;
		LAYOUT layout;
		
		//Chunk mapping
		CHUNKMAPPING *chunkMapping;
		
		//Tile to collision maps
		uint16_t *normalMap;
		uint16_t *alternateMap;
		
		//Collision tiles
		COLLISIONTILE *collisionTile;
	public:
		LEVEL(int id);
		~LEVEL();
		
		void Update();
		void Draw();
};
