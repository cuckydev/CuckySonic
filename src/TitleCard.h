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
		int act;
		
		int frame;
		
	public:
		TITLECARD(const char *zoneName, int actNumber);
		~TITLECARD();
		
		void DrawText(const char *text, int x, int y);
		void UpdateAndDraw();
};