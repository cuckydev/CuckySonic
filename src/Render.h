#pragma once
#include "SDL_render.h"
#include "SDL_pixels.h"
#include <stdint.h>

//Pixel function
typedef void (*PIXELFUNCTION)(uint8_t*, uint32_t);
	
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

//Texture class
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
		
		//For linked lists (if applicable)
		TEXTURE **list;
		TEXTURE *next;
		
	public:
		TEXTURE(TEXTURE **linkedList, const char *path);
		TEXTURE(TEXTURE **linkedList, uint8_t *data, int dWidth, int dHeight);
		~TEXTURE();
};
	
//Render queue structure
#define RENDERQUEUE_LENGTH 0x1000
#define RENDERLAYERS 0x100

enum RENDERQUEUE_TYPE
{
	RENDERQUEUE_TEXTURE,
	RENDERQUEUE_SOLID,
};

struct RENDERQUEUE
{
	//Shared
	RENDERQUEUE_TYPE type;
	SDL_Rect dest;
	
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
			PALCOLOUR *colour;
		} solid;
	};
};

//Software framebuffer class
class SOFTWAREBUFFER
{
	public:
		//Render queue
		RENDERQUEUE queue[RENDERLAYERS][RENDERQUEUE_LENGTH];
		RENDERQUEUE *queueEntry[RENDERLAYERS];
		
		//Dimensions and pixel drawn buffer
		int width;
		int height;
		
		//Status
		const char *fail;
		
	private:
		SDL_Texture *texture;
		
	public:
		SOFTWAREBUFFER(int bufWidth, int bufHeight);
		~SOFTWAREBUFFER();
		
		void DrawPoint(int layer, int x, int y, PALCOLOUR *colour);
		void DrawQuad(int layer, const SDL_Rect *quad, PALCOLOUR *colour);
		void DrawTexture(TEXTURE *texture, PALETTE *palette, const SDL_Rect *src, int layer, int x, int y, bool xFlip, bool yFlip);
		
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
extern SDL_Window *gWindow;
extern SDL_Renderer *gRenderer;
extern SOFTWAREBUFFER *gSoftwareBuffer;

extern SDL_PixelFormat *gNativeFormat;

extern RENDERSPEC gRenderSpec;

bool InitializeRender();
void QuitRender();
