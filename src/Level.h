#pragma once
#include <string>
#include <stdint.h>

#include "LinkedList.h"
#include "Render.h"
#include "Audio.h"
#include "GameConstants.h"
#include "CommonMacros.h"
#include "LevelSpecific.h"
#include "Player.h"
#include "Object.h"
#include "Camera.h"
#include "TitleCard.h"
#include "Hud.h"
#include "Background.h"

#define OSCILLATORY_VALUES 16

//Level render layer
#define OBJECT_LAYERS 8
enum LEVEL_RENDERLAYER
{
	LEVEL_RENDERLAYER_TITLECARD,
	LEVEL_RENDERLAYER_HUD,
	LEVEL_RENDERLAYER_OBJECT_HIGH_0,
	LEVEL_RENDERLAYER_FOREGROUND_HIGH = LEVEL_RENDERLAYER_OBJECT_HIGH_0 + OBJECT_LAYERS,
	LEVEL_RENDERLAYER_OBJECT_LOW_0,
	LEVEL_RENDERLAYER_FOREGROUND_LOW = LEVEL_RENDERLAYER_OBJECT_LOW_0 + OBJECT_LAYERS,
	LEVEL_RENDERLAYER_BACKGROUND,
};

//Level formats
enum LEVELFORMAT
{
	LEVELFORMAT_CHUNK128,			//Chunk data with specified width and height
	LEVELFORMAT_CHUNK128_SONIC2,	//Chunk data that's always 128 x 16
	LEVELFORMAT_TILE,				//16x16 tile data with specified width and height
};

enum OBJECTFORMAT
{
	OBJECTFORMAT_SONIC1,
	OBJECTFORMAT_SONIC2,
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
	//Stage identification (name, zone, etc.)
	ZONEID zone;
	std::string name;
	std::string subtitle;
	
	//Formats
	LEVELFORMAT format;
	OBJECTFORMAT objectFormat;
	ARTFORMAT artFormat;
	
	//Paths
	const char *levelReferencePath;		//For typically level specific stuff such as the layout and object path
	const char *chunkTileReferencePath;	//For the chunk and tile definitions
	const char *collisionReferencePath;	//For the collision data itself (height maps and angle maps)
	const char *artReferencePath;		//For the level's art
	const char *music;
	
	//Level specific functions and lists
	const char **preloadTexture;
	const char **preloadMappings;
	BACKGROUNDFUNCTION backFunction;
	PALETTECYCLEFUNCTION paletteFunction;
	OBJECTFUNCTION *objectFunctionList;
	
	//Start position and boundaries
	int16_t startX, startY;
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
	uint8_t srcChunk;
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
	size_t width = 0;
	size_t height = 0;
	TILE *foreground = nullptr;
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

//Object load
struct OBJECT_LOAD
{
	//Object data
	OBJECTFUNCTION function = nullptr;
	OBJECT_STATUS status;
	POSDEF(x)
	POSDEF(y)
	unsigned int subtype = 0;
	
	//Current status
	OBJECT *loaded = nullptr;
	bool loadRange = false;
	bool specificBit = false;
};

//Level struct
class LEVEL
{
	public:
		const char *fail = nullptr;
		
		//Level IDs
		LEVELID levelId;
		ZONEID zone;
		
		//Loaded music
		MUSIC *stageMusic = nullptr;
		MUSIC *bossMusic = nullptr;
		
		MUSIC *speedShoesMusic = nullptr;
		MUSIC *invincibilityMusic = nullptr;
		MUSIC *superMusic = nullptr;
		MUSIC *extraLifeMusic = nullptr;
		MUSIC *goalMusic = nullptr;
		MUSIC *gameoverMusic = nullptr;
		
		//Current music state
		MUSIC *primaryMusic = nullptr;	//stageMusic or bossMusic											(played below secondaryMusic)
		MUSIC *secondaryMusic = nullptr;	//speedShoesMusic, invincibilityMusic, goalMusic, or gameoverMusic	(played below jingles)
		
		MUSIC *currentMusic = nullptr;	//Any of the loaded songs
		
		//Oscillatory stuff
		bool oscillateDirection[OSCILLATORY_VALUES];
		uint16_t oscillate[OSCILLATORY_VALUES][2];
		
		//Art
		TEXTURE *tileTexture = nullptr;
		BACKGROUND *background = nullptr;
		PALETTECYCLEFUNCTION paletteFunction = nullptr;
		
		//Chunk and tile data
		size_t chunks = 0, tiles = 0;
		CHUNKMAPPING *chunkMapping = nullptr;
		TILEMAPPING *tileMapping = nullptr;
		
		//Stage layout
		LAYOUT layout;
		
		//Collision data
		size_t collisionTiles = 0;
		COLLISIONTILE *collisionTile = nullptr;
		
		//Boundaries
		uint16_t leftBoundary = 0;
		uint16_t rightBoundary = 0;
		uint16_t topBoundary = 0;
		uint16_t bottomBoundary = 0;
		
		uint16_t leftBoundaryTarget = 0;
		uint16_t rightBoundaryTarget = 0;
		uint16_t topBoundaryTarget = 0;
		uint16_t bottomBoundaryTarget = 0;
		
		//Players and objects
		LINKEDLIST<PLAYER*> playerList;
		LINKEDLIST<OBJECT*> coreObjectList;
		LINKEDLIST<OBJECT_LOAD*> objectLoadList;
		LINKEDLIST<OBJECT*> objectList;
		
		//Title card, camera, and HUD
		CAMERA *camera = nullptr;
		TITLECARD *titleCard = nullptr;
		HUD *hud = nullptr;
		
		//Object texture cache
		LINKEDLIST<TEXTURE*> objTextureCache;
		LINKEDLIST<MAPPINGS*> objMappingsCache;
		
		//Other state stuff
		int frameCounter = 0;		//Frames the level has been loaded
		
		bool updateTime = true;		//If the timer should update (at the end of the level)
		bool updateStage = true;	//If objects and other stuff should update (player not dead)
		bool fading = false;		//If we're currently fading in / out
		bool isFadingIn = false;	//If we're fading in or not
		bool specialFade = false;	//Fading to / from white (fades to Special Stage)
		
	public:
		//Constructor and destructor
		LEVEL(int id, const char *players[]);
		~LEVEL();
		
		//Level loading functions
		bool LoadMappings(LEVELTABLE *tableEntry);
		bool LoadLayout(LEVELTABLE *tableEntry);
		bool LoadCollisionTiles(LEVELTABLE *tableEntry);
		bool LoadObjects(LEVELTABLE *tableEntry);
		bool LoadArt(LEVELTABLE *tableEntry);
		void UnloadAll();
		
		//Fading
		void SetFade(bool fadeIn, bool isSpecial);
		bool UpdateFade();
		
		//Dynamic events
		void DynamicEvents();
		
		//Object texture and mapping cache functions
		TEXTURE *GetObjectTexture(const char *path);
		MAPPINGS *GetObjectMappings(const char *path);
		
		//Object load functions
		OBJECT_LOAD *GetObjectLoad(OBJECT *object);
		void LinkObjectLoad(OBJECT *object);
		void ReleaseObjectLoad(OBJECT *object);
		void UnrefObjectLoad(OBJECT *object);
		
		void CheckObjectLoad();
		
		//Object layer function
		LEVEL_RENDERLAYER GetObjectLayer(bool highPriority, int priority);
		
		//Palette update function
		void PaletteUpdate();
		
		//Oscillatory table functions
		void OscillatoryInit();
		void OscillatoryUpdate();
		
		//Music functions
		void SetPlayingMusic(MUSIC *music, bool resumeLastPosition, bool fade);
		void ChangePrimaryMusic(MUSIC *music);
		void ChangeSecondaryMusic(MUSIC *music);
		void PlayJingleMusic(MUSIC *music);
		
		void StopSecondaryMusic();
		void StopJingleMusic();
		
		void UpdateMusic();
		
		//Update and draw functions
		bool UpdateStage();
		bool Update();
		void Draw();
};

extern LEVELTABLE gLevelTable[];
