#pragma once
#include <stdint.h>
#include "Render.h"
#include "Player.h"
#include "Object.h"
#include "Camera.h"

#define PLAYERS 1

enum LEVELFORMAT
{
	LEVELFORMAT_CHUNK128,
	LEVELFORMAT_CHUNK128_SONIC2,
};

typedef void (*OBJECTFUNCTION)(OBJECT*);

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
	
	//Objects
	const char *objectPath;
	OBJECTFUNCTION *objectFunctionList;
	const char *ringPath;
	
	//Start location
	uint16_t startX, startY;
};

struct CHUNKMAPPINGTILE
{
	//Solidity
	bool altLRB : 1;
	bool altTop : 1;
	bool norLRB : 1;
	bool norTop : 1;
	//Flip
	bool yFlip : 1;
	bool xFlip : 1;
	//Tile index
	uint16_t tile : 10;
};

struct CHUNKMAPPING
{
	CHUNKMAPPINGTILE tile[8 * 8];
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
		
		//Level ID
		int levelId;
		
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
		
		//Boundaries
		uint16_t leftBoundary;
		uint16_t rightBoundary;
		uint16_t topBoundary;
		uint16_t bottomBoundary;
		uint16_t bottomBoundary2;
		
		//Players and objects
		PLAYER *player[PLAYERS];
		OBJECT *objectList;
		CAMERA *camera;
		
	public:
		LEVEL(int id);
		~LEVEL();
		
		void DynamicBoundaries();
		
		void Update();
		void Draw();
};

extern LEVELTABLE gLevelTable[];
