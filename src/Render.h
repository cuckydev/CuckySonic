#pragma once
#include <stdint.h>
#include <deque>

#ifdef BACKEND_SDL2
	#include "SDL_render.h"
	#include "SDL_pixels.h"
	#define BACKEND_TEXTURE		SDL_Texture
	#define BACKEND_PIXELFORMAT	SDL_PixelFormat
#endif

//Rect and point structures
struct RECT { int x, y, w, h; };
struct POINT { int x, y; };
	
//Palette line
struct PALCOLOUR
{
	uint32_t colour;		//The natively formatted colour
	uint8_t r, g, b;		//The original RGB colours
	uint8_t ogr, ogg, ogb;	//For palette effects such as fading
};

struct PALETTE
{
	PALCOLOUR colour[0x100];
};

void SetPaletteColour(PALCOLOUR *palColour, uint8_t r, uint8_t g, uint8_t b);
void ModifyPaletteColour(PALCOLOUR *palColour, uint8_t r, uint8_t g, uint8_t b);
void RegenPaletteColour(PALCOLOUR *palColour);

//Texture classes
class TEXTURE
{
	public:
		//Status
		const char *fail;
		
		//Source file (if applicable)
		const char *source;
		
		//Texture data
		uint8_t *texture;
		int width;
		int height;
		
		//Loaded palette
		PALETTE *loadedPalette;
		
	public:
		TEXTURE(std::deque<TEXTURE*> *linkedList, const char *path);
		TEXTURE(std::deque<TEXTURE*> *linkedList, uint8_t *data, int dWidth, int dHeight);
		~TEXTURE();
};

class TEXTURE_FULLCOLOUR
{
	public:
		//Status
		const char *fail;
		
		//Source file (if applicable)
		const char *source;
		
		//Texture data
		uint32_t *texture;
		int width;
		int height;
		
	public:
		TEXTURE_FULLCOLOUR(std::deque<TEXTURE_FULLCOLOUR*> *linkedList, const char *path);
		~TEXTURE_FULLCOLOUR();
};

//Render queue structure
#define RENDERLAYERS 0x100

enum RENDERQUEUE_TYPE
{
	RENDERQUEUE_TEXTURE,
	RENDERQUEUE_TEXTURE_FULLCOLOUR,
	RENDERQUEUE_SOLID,
};

struct RENDERQUEUE
{
	//Shared
	RENDERQUEUE_TYPE type;
	RECT dest;
	
	//Our union for different types
	union
	{
		struct
		{
			int srcX, srcY;
			PALETTE *palette;
			TEXTURE *texture;
			bool xFlip, yFlip;
		} texture;
		struct
		{
			int srcX, srcY;
			TEXTURE_FULLCOLOUR *texture;
			bool xFlip, yFlip;
		} textureFullColour;
		struct
		{
			PALCOLOUR *colour;
		} solid;
	};
	
	//Linked list
	RENDERQUEUE *next;
};

//Software framebuffer class
class SOFTWAREBUFFER
{
	public:
		//Failure
		const char *fail;
		
		//Render queue
		RENDERQUEUE *queue[RENDERLAYERS];
		
		//Dimensions and pixel drawn buffer
		int width;
		int height;
		
	private:
		BACKEND_TEXTURE *texture;
		
	public:
		SOFTWAREBUFFER(int bufWidth, int bufHeight);
		~SOFTWAREBUFFER();
		
		void DrawPoint(int layer, const POINT *point, PALCOLOUR *colour);
		void DrawQuad(int layer, const RECT *quad, PALCOLOUR *colour);
		void DrawTexture(TEXTURE *texture, PALETTE *palette, const RECT *src, int layer, int x, int y, bool xFlip, bool yFlip);
		void DrawTexture(TEXTURE_FULLCOLOUR *texture, const RECT *src, int layer, int x, int y, bool xFlip, bool yFlip);
		
		void Blit8 (PALCOLOUR *backgroundColour, uint8_t  *buffer, int pitch);
		void Blit16(PALCOLOUR *backgroundColour, uint16_t *buffer, int pitch);
		void Blit32(PALCOLOUR *backgroundColour, uint32_t *buffer, int pitch);
		
		bool RenderToScreen(PALCOLOUR *backgroundColour);
};

//Current render specifications
struct RENDERSPEC
{
	int width, height, scale;
};

//Globals
extern SOFTWAREBUFFER *gSoftwareBuffer;
extern RENDERSPEC gRenderSpec;

bool InitializeRender();
void QuitRender();
