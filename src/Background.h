#pragma once
#include <stdint.h>
#include "Render.h"

//Background function thing
class BACKGROUND;
typedef void (*BACKGROUNDFUNCTION)(BACKGROUND*, bool, int, int);

//Background class
class BACKGROUND
{
	public:
		//Failure
		const char *fail;
		
		//Texture and scroll values
		TEXTURE *texture;
		
		//Function to use
		BACKGROUNDFUNCTION function;
	
	public:
		BACKGROUND(const char *name, BACKGROUNDFUNCTION setFunction);
		~BACKGROUND();
		
		void DrawStrip(SDL_Rect *src, int layer, int16_t y, int16_t fromX, int16_t toX);
		
		void Draw(bool doScroll, int16_t cameraX, int16_t cameraY);
};
