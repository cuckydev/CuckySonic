#pragma once
#include "SDL_render.h"
#include "SDL_pixels.h"
#include <stdint.h>

//Screen size constants (TODO: make variable versions of this)
#define SCREEN_WIDTH	398
#define SCREEN_HEIGHT	224
#define SCREEN_SCALE	2

//Set pixel macro
#define SET_BUFFER_PIXEL(buffer, bytes, pixel, value)					\
	switch (bytes)														\
	{																	\
		case 1:															\
			((uint8_t*)buffer)[pixel] = value;							\
			break;														\
		case 2:															\
			((uint16_t*)buffer)[pixel] = value;							\
			break;														\
		case 3:															\
			((uint8_t*)buffer)[pixel * 3 + 0] = ((uint8_t*)&value)[0];	\
			((uint8_t*)buffer)[pixel * 3 + 1] = ((uint8_t*)&value)[1];	\
			((uint8_t*)buffer)[pixel * 3 + 2] = ((uint8_t*)&value)[2];	\
			break;														\
		case 4:															\
			((uint32_t*)buffer)[pixel] = value;							\
			break;														\
		default:														\
			return Error("Illegal framebuffer byte size");				\
			break;														\
	}
	
//Palette line
struct PALCOLOR
{
	uint32_t colour;		//The natively formatted colour
	uint8_t r, g, b;		//The original RGB colours
	uint8_t ogr, ogg, ogb;	//For palette effects such as fading
};

struct PALETTE
{
	PALCOLOR colour[0x100];
};

inline void SetPaletteColour(PALCOLOR *palColour, uint8_t r, uint8_t g, uint8_t b);
inline void ModifyPaletteColour(PALCOLOR *palColour, uint8_t r, uint8_t g, uint8_t b);

//Texture class
class TEXTURE
{
	private:
		//Texture data
		uint8_t *texture;
		size_t width;
		size_t height;
		
	public:
		//Status
		const char *fail;
		
	public:
		TEXTURE(const char *path);
		~TEXTURE();
		
		bool Draw(PALETTE *palette, SDL_Rect *src, int x, int y);
};
	
//Render queue structure
#define RENDERQUEUE_LENGTH 0x200

struct RENDERQUEUE
{
	uint8_t renderType;
	SDL_Rect dest;
	union
	{
		struct
		{
			int srcX, srcY;
			PALETTE *palette;
			TEXTURE *texture;
		} texture;
		struct
		{
			PALCOLOR *colour;
		} solid;
	};
};

//Software framebuffer class
class SOFTWAREBUFFER
{
	public:
		//Our framebuffer
		void *buffer;
		SDL_PixelFormat *format;
		
		//Render queue
		RENDERQUEUE queue[RENDERQUEUE_LENGTH];
		RENDERQUEUE *queueEntry;
		
		//Dimensions
		size_t width;
		size_t height;
		
		//Status
		const char *fail;
		
	private:
		SDL_Texture *texture;
		
	public:
		SOFTWAREBUFFER(uint32_t bufFormat, size_t bufWidth, size_t bufHeight);
		~SOFTWAREBUFFER();
		
		inline uint32_t RGB(uint8_t r, uint8_t g, uint8_t b);
		
		bool RenderToScreen();
};

//Globals
extern SDL_Window *gWindow;
extern SDL_Renderer *gRenderer;
extern SOFTWAREBUFFER *gSoftwareBuffer;

bool InitializeRender();
void QuitRender();
