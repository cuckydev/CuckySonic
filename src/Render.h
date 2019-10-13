#pragma once
#include "SDL_render.h"
#include "SDL_pixels.h"
#include <stdint.h>
	
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
		
		//For linked lists (if applicable)
		TEXTURE **list;
		TEXTURE *next;
		
	public:
		TEXTURE(TEXTURE **linkedList, const char *path);
		TEXTURE(TEXTURE **linkedList, uint8_t *data, int dWidth, int dHeight);
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
		
		//For linked lists (if applicable)
		TEXTURE_FULLCOLOUR **list;
		TEXTURE_FULLCOLOUR *next;
		
	public:
		TEXTURE_FULLCOLOUR(TEXTURE_FULLCOLOUR **linkedList, const char *path);
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
		SDL_Texture *texture;
		
	public:
		SOFTWAREBUFFER(int bufWidth, int bufHeight);
		~SOFTWAREBUFFER();
		
		void DrawPoint(int layer, int x, int y, PALCOLOUR *colour);
		void DrawQuad(int layer, const SDL_Rect *quad, PALCOLOUR *colour);
		void DrawTexture(TEXTURE *texture, PALETTE *palette, const SDL_Rect *src, int layer, int x, int y, bool xFlip, bool yFlip);
		void DrawTexture(TEXTURE_FULLCOLOUR *texture, const SDL_Rect *src, int layer, int x, int y, bool xFlip, bool yFlip);
		
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

bool RefreshRenderer();
bool RenderCheckVSync();
bool InitializeRender();
void QuitRender();
