#pragma once
#include "Render.h"

class HUD
{
	public:
		//Failure
		const char *fail;
		
		//Texture
		TEXTURE *texture;
		
	public:
		HUD();
		~HUD();
		
		void DrawCharacter(int xPos, int yPos, int srcX);
		void DrawNumber(int xPos, int yPos, int number, int forceDigits);
		void DrawElement(int xPos, int yPos, int srcX, int srcY);
		
		void Draw();
};
