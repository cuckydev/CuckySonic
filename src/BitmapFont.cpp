#include "BitmapFont.h"

//Constructor and destructor
BITMAPFONT::BITMAPFONT(TEXTURE *useBitmap, unsigned int useX0, unsigned int useY0, unsigned int useCw, unsigned int useCh, unsigned int useSx, unsigned int useSy, unsigned int useCpl, unsigned int useTlc) : bitmap(useBitmap), x0(useX0), y0(useY0), cw(useCw), ch(useCh), sx(useSx), sy(useSy), cpl(useCpl), tlc(useTlc) { return; }
BITMAPFONT::~BITMAPFONT() { return; }

//Draw an std::string
void BITMAPFONT::DrawString(std::string string, int layer, int x, int y)
{
	//Draw every character of the string according to its size
	for(size_t i = 0; i < string.size(); i++)
	{
		RECT thisCharRect = {
			(int)(x0 + ((string[i] - tlc) % cpl) * (cw + sx)),
			(int)(y0 + ((string[i] - tlc) / cpl) * (ch + sy)),
			(int)cw,
			(int)ch,
		};
		gSoftwareBuffer->DrawTexture(bitmap, bitmap->loadedPalette, &thisCharRect, layer, x, y, false, false);
		x += cw;
	}
}

//Draw a C-style null terminated string
void BITMAPFONT::DrawText(const char *text, int layer, int x, int y)
{
	//Draw every character until a terminator key (\0)
	for (const char *current = text; *current != '\0'; current++)
	{
		RECT thisCharRect = {
			(int)(x0 + ((*current - tlc) % cpl) * (cw + sx)),
			(int)(y0 + ((*current - tlc) / cpl) * (ch + sy)),
			(int)cw,
			(int)ch,
		};
		gSoftwareBuffer->DrawTexture(bitmap, bitmap->loadedPalette, &thisCharRect, layer, x, y, false, false);
		x += cw;
	}
}
