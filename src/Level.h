#pragma once
#include <stdint.h>
#include "Render.h"
#include "Player.h"
#include "Object.h"
#include "Camera.h"
#include "TitleCard.h"
#include "Hud.h"
#include "GameConstants.h"
#include "Audio.h"

#define PALETTE_CYCLES 8

#define OSCILLATORY_VALUES 16

//Object function definition
typedef void (*OBJECTFUNCTION)(OBJECT*);

//Level render layer
#define OBJECT_LAYERS 8
enum LEVEL_RENDERLAYER
{
	LEVEL_RENDERLAYER_BACKGROUND,
	LEVEL_RENDERLAYER_FOREGROUND_LOW,
	LEVEL_RENDERLAYER_OBJECT_0,
	LEVEL_RENDERLAYER_FOREGROUND_HIGH = LEVEL_RENDERLAYER_OBJECT_0 + OBJECT_LAYERS,
	LEVEL_RENDERLAYER_OBJECT_HIGH_0,
	LEVEL_RENDERLAYER_HUD = LEVEL_RENDERLAYER_OBJECT_HIGH_0 + OBJECT_LAYERS,
	LEVEL_RENDERLAYER_TITLECARD,
};

//Level formats
enum LEVELFORMAT
{
	LEVELFORMAT_CHUNK128,			//Chunk data with specified width and height
	LEVELFORMAT_CHUNK128_SONIC2,	//Chunk data that's always 128 x 16
	LEVELFORMAT_TILE,				//16x16 tile data with specified width and height
};

enum ARTFORMAT
{
	ARTFORMAT_BMP,	//Generic .bmp file
};

//Individual level definition table
enum ZONEID
{
	ZONEID_GHZ,
	ZONEID_EHZ,
};

enum LEVELID
{
	LEVELID_GHZ1,
	LEVELID_GHZ2,
	LEVELID_EHZ1,
	LEVELID_MAX,
};

struct LEVELTABLE
{
	//Zone name and act id
	ZONEID zone;
	int act;
	
	//Zone name and subtitle
	const char *name;
	const char *subtitle;
	
	//Level data format
	LEVELFORMAT format;
	ARTFORMAT artFormat;
	
	//Paths
	const char *levelReferencePath;		//For typically level specific stuff such as the layout and object path
	const char *chunkTileReferencePath;	//For the chunk and tile definitions
	const char *collisionReferencePath;	//For the collision data itself (height maps and angle maps)
	const char *artReferencePath;		//For the level's art
	
	//Music
	const char *music;
	
	//Object function list
	OBJECTFUNCTION *objectFunctionList;
	
	//Start position and boundaries
	uint16_t startX, startY;
	uint16_t leftBoundary, rightBoundary, topBoundary, bottomBoundary;
};

//Chunk mapping
struct TILE
{
	//Solidity
	bool altLRB : 1;
	bool altTop : 1;
	bool norLRB : 1;
	bool norTop : 1;
	
	//Flipping
	bool yFlip : 1;
	bool xFlip : 1;
	
	//Tile index
	uint16_t tile : 10;
};

struct CHUNKMAPPING
{
	TILE tile[8 * 8];
};

//Tile mapping
struct TILEMAPPING
{
	uint16_t normalColTile;
	uint16_t alternateColTile; 
};

//Layout
struct LAYOUT
{
	//Dimensions
	int width;
	int height;
	
	//Map data
	TILE *foreground;
};

//Collision tile data
struct COLLISIONTILE
{
	//Height and width (rotated) maps
	int8_t normal[0x10];
	int8_t rotated[0x10];
	
	//The tile's angle
	uint8_t angle;
};

//Palette cycle
struct PALETTECYCLE
{
	int timer = 0;
	int cycle = 0;
};

//Level struct
class LEVEL
{
	public:
		const char *fail;
		
		//Level ID
		LEVELID levelId;
		
		ZONEID zone;
		int act;
		
		//Level music
		const char *music;
		
		//Game update stuff
		int frameCounter; //Frames the level has been loaded
		
		bool updateTime; //If the timer should update (at the end of the level)
		bool updateStage; //If objects and other stuff should update (player not dead)
		
		//Oscillatory stuff
		bool oscillateDirection[OSCILLATORY_VALUES];
		uint16_t oscillate[OSCILLATORY_VALUES][2];
		
		//Art
		ARTFORMAT artFormat;
		TEXTURE *tileTexture;
		
		TEXTURE *backgroundTexture;
		uint16_t *backgroundScroll;
		
		PALETTECYCLE palCycle[PALETTE_CYCLES];
		
		//Chunk and tile data
		int chunks, tiles;
		CHUNKMAPPING *chunkMapping;
		TILEMAPPING *tileMapping;
		
		//Layout
		LEVELFORMAT format;
		LAYOUT layout;
		
		//Collision data
		int collisionTiles;
		COLLISIONTILE *collisionTile;
		
		//Boundaries
		uint16_t leftBoundary;
		uint16_t rightBoundary;
		uint16_t topBoundary;
		uint16_t bottomBoundary;
		uint16_t leftBoundaryTarget;
		uint16_t rightBoundaryTarget;
		uint16_t topBoundaryTarget;
		uint16_t bottomBoundaryTarget;
		
		//Players and objects
		PLAYER *playerList;
		OBJECT *objectList;
		CAMERA *camera;
		
		//Title card and HUD
		TITLECARD *titleCard;
		HUD *hud;
		
		//Object texture cache
		TEXTURE *objTextureCache;
		MAPPINGS *objMappingsCache;
		
		//State
		bool inTitleCard;
		bool fading;
		bool isFadingIn;
		bool specialFade;
		
	public:
		LEVEL(int id, int players, const char **playerPaths);
		~LEVEL();
		
		bool LoadMappings(LEVELTABLE *tableEntry);
		bool LoadLayout(LEVELTABLE *tableEntry);
		bool LoadCollisionTiles(LEVELTABLE *tableEntry);
		bool LoadObjects(LEVELTABLE *tableEntry);
		bool LoadArt(LEVELTABLE *tableEntry);
		void UnloadAll();
		
		void SetFade(bool fadeIn, bool isSpecial);
		bool UpdateFade();
		
		void DynamicEvents();
		
		TEXTURE *GetObjectTexture(const char *path);
		TEXTURE *GetObjectTexture(uint8_t *data, int dWidth, int dHeight); //just in case
		
		MAPPINGS *GetObjectMappings(const char *path);
		
		void PaletteUpdate();
		void GetBackgroundScroll(bool updateScroll, uint16_t *array, int16_t *cameraX, int16_t *cameraY);
		
		void OscillatoryInit();
		void OscillatoryUpdate();
		
		bool Update(bool checkTitleCard);
		void Draw();
};

extern LEVELTABLE gLevelTable[];
