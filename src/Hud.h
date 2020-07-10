#pragma once
#include "BitmapFont.h"

class HUD
{
	public:
		//Failure
		const char *fail = nullptr;
		
		//Texture
		TEXTURE *texture;
		BITMAPFONT *font;
		
	public:
		HUD();
		~HUD();
		
		void DrawLabel(int xPos, int yPos, int srcX, int srcY);
		void Draw();
};
