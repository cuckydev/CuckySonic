#pragma once
#include <string>
#include <stdint.h>
#include "BitmapFont.h"

enum TITLECARD_LINE
{
	LINE_CUCKYSONIC_LABEL,
	LINE_LEVEL_NAME,
	LINE_LEVEL_SUBTITLE,
	LINE_MAX,
};

class TITLECARD
{
	public:
		//Failure
		const char *fail = nullptr;
		
		//Loaded texture
		TEXTURE *texture;
		
		//State
		bool activeLock = true;
		unsigned int frame = 0;
		
		//Text
		std::string name;
		std::string subtitle;
		
		BITMAPFONT *nameFont;
		BITMAPFONT *subtitleFont;
		
		//Focus position
		int focusX, focusY;
		
		//Lines
		struct LINEPOS
		{
			//Position and speed
			int x, y;
			int xsp, ysp;
			int xAcc, yAcc;
			int xMin, xMax, yMin, yMax;
		} line[LINE_MAX];
		
	public:
		TITLECARD(std::string levelName, std::string levelSubtitle);
		~TITLECARD();
		void DrawRibbon(const RECT *rect, int x, int y, int width);
		void UpdateAndDraw();
};