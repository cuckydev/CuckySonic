#pragma once
#include <stdint.h>
#include "Render.h"

class TITLECARD
{
	public:
		const char *fail;
		
		//Textures
		TEXTURE *texture;
		TEXTURE *fontTexture;
		
		//Current zone and frame
		const char *name;
		const char  *subtitle;
		
		int drawY;
		int frame;
		
	public:
		TITLECARD(const char *levelName, const char *levelSubtitle);
		~TITLECARD();
		
		void DrawText(const char *text, int x, int y);
		void UpdateAndDraw();
};