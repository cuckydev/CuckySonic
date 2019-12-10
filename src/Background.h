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
		const char *fail = nullptr;
		
		//Texture and scroll values
		TEXTURE *texture = nullptr;
		
		//Function to use
		BACKGROUNDFUNCTION function = nullptr;
	
	public:
		BACKGROUND(std::string name, BACKGROUNDFUNCTION setFunction);
		~BACKGROUND();
		
		void DrawStrip(RECT *src, int layer, int y, int fromX, int toX);
		
		void Draw(bool doScroll, int cameraX, int cameraY);
};
