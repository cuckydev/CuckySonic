#pragma once
#include <string>
#include "Render.h"

class BITMAPFONT
{
	public:
		TEXTURE *bitmap;				//Texture to draw with
		unsigned int x0, y0;			//Top left of font area
		unsigned int cw, ch, sx, sy;	//Character width, character height, seperation x, seperation y
		unsigned int cpl, tlc;			//Characters per line, character at top left (usually 0x20 to leave out command characters)
		
	public:
		BITMAPFONT(TEXTURE *useBitmap, unsigned int useX0, unsigned int useY0, unsigned int useCw, unsigned int useCh, unsigned int useSx, unsigned int useSy, unsigned int useCpl, unsigned int useTlc);
		~BITMAPFONT();
		void DrawString(std::string string, int layer, int x, int y);
		void DrawText(const char *text, int layer, int x, int y);
};
