#pragma once
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

//Software framebuffer class
class SOFTWAREBUFFER
{
	public:
		//Our framebuffer
		void *buffer;
		SDL_PixelFormat *format;
		
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
		
		bool RenderToScreen();
};

//Globals
extern SDL_Window *gWindow;
extern SDL_Renderer *gRenderer;
extern SOFTWAREBUFFER *gSoftwareBuffer;

bool InitializeRender();
void QuitRender();
bool SetPixelColor(unsigned int x, unsigned int y, uint8_t r, uint8_t g, uint8_t b);
