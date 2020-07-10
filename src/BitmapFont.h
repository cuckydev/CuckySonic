#pragma once
#include <string>
#include "Render.h"

class BITMAPFONT
{
	private:
		TEXTURE *bitmap;				//Texture to draw with
		unsigned int x0, y0;			//Top left of font area
		unsigned int cw, ch, sx, sy;	//Character width, character height, seperation x, seperation y
		unsigned int cpl, tlc;			//Characters per line, character at top left (usually 0x20 to leave out command characters)
		
	public:
		BITMAPFONT(TEXTURE *useBitmap, unsigned int useX0, unsigned int useY0, unsigned int useCw, unsigned int useCh, unsigned int useSx, unsigned int useSy, unsigned int useCpl, unsigned int useTlc) : bitmap(useBitmap), x0(useX0), y0(useY0), cw(useCw), ch(useCh), sx(useSx), sy(useSy), cpl(useCpl), tlc(useTlc) { return; }
		~BITMAPFONT() { return; }
		
		inline void DrawString(std::string string, size_t layer, int x, int y)
		{
			//Draw every character of the string according to its size
			for(size_t i = 0; i < string.size(); i++)
			{
				//Get our rect according to this character (from the top left of the font area, using size, seperation, and character info)
				RECT thisCharRect = {
					(int)(x0 + ((string[i] - tlc) % cpl) * (cw + sx)),
					(int)(y0 + ((string[i] - tlc) / cpl) * (ch + sy)),
					(int)cw,
					(int)ch,
				};
				
				//Draw character and draw at next position
				if ((unsigned)string[i] >= tlc)
					gSoftwareBuffer->DrawTexture(bitmap, bitmap->loadedPalette, &thisCharRect, layer, x, y, false, false);
				x += cw;
			}
		}
};
