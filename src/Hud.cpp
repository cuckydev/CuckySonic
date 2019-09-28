#include "Hud.h"
#include "Level.h"
#include "Game.h"
#include "Error.h"

//Constructor and deconstructor
HUD::HUD()
{
	//Clear memory
	memset(this, 0, sizeof(HUD));
	
	//Load texture
	texture = gLevel->GetObjectTexture("data/HUD.bmp");
	if (texture == nullptr || texture->fail)
	{
		Error(fail = texture->fail);
		return;
	}
}

HUD::~HUD()
{
	//`texture` is freed in gLevel destructor
	return;
}

//Drawing functions
void HUD::DrawCharacter(int xPos, int yPos, int srcX)
{
	SDL_Rect src = {192 + srcX * 8, 0, 8, 16};
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &src, LEVEL_RENDERLAYER_HUD, xPos, yPos, false, false);
}

void HUD::DrawNumber(int xPos, int yPos, int number, unsigned int forceDigits, bool fromRight)
{
	int offset = 0;
	unsigned int exponent = 1;
	
	//Get how many digits we're using
	unsigned int numberDigit = number;
	while ((numberDigit /= 10) != 0)
	{
		//Increment position if drawing from right
		if (fromRight)
			xPos -= 8;
		
		//Increment digit
		offset++;
		exponent *= 10;
	}
	
	//Include forced digits
	while (offset < forceDigits)
	{
		//Increment position if drawing from right
		if (fromRight)
			xPos -= 8;
		
		//Increment digit
		offset++;
		exponent *= 10;
	}
	
	//Draw all digits
	while (offset >= 0)
	{
		
		//Get the digit that this is
		int a = 0;
		
		while (exponent <= number)
		{
			number -= exponent;
			a++;
		}
		
		//Draw digit
		DrawCharacter(xPos, yPos, a);
		xPos += 8;

		//Go to next digit
		offset--;
		exponent /= 10;
	}
}

void HUD::DrawElement(int xPos, int yPos, int srcX, int srcY)
{
	SDL_Rect src = {srcX * 48, srcY * 16, 48, 16};
	gSoftwareBuffer->DrawTexture(texture, texture->loadedPalette, &src, LEVEL_RENDERLAYER_HUD, xPos, yPos, false, false);
}

//Core draw function
void HUD::Draw()
{
	//Blink the time and ring labels
	bool scoreAlt = false;
	bool timeAlt = false;
	bool ringAlt = false;
	
	if (gLevel->frameCounter & 0x8)
	{
		if (gTime >= (60 * 60 * 9))
			timeAlt = true;
		if (gRings == 0)
			ringAlt = true;
	}
	
	//Draw score, time, and ring labels
	DrawElement(16, 8, 0, scoreAlt ? 1 : 0);
	DrawElement(16, 24, 1, timeAlt ? 1 : 0);
	DrawElement(16, 40, 2, ringAlt ? 1 : 0);
	
	//Draw our score and ring values
	DrawNumber(112, 8, gScore, 0, true);
	DrawNumber(88, 40, gRings, 0, true);
	
	//Draw time
	DrawNumber(112, 24, (gTime * 100 / 60) % 100, 1, true); //Milliseconds
	DrawCharacter(112 - 16, 24, 11); // "
	DrawNumber(112 - 24, 24, (gTime / 60) % 60, 1, true); //Seconds
	DrawCharacter(112 - 40, 24, 10); // '
	DrawNumber(112 - 48, 24, gTime / 60 / 60, 0, true); //Minutes
	
	//Draw lives
	DrawElement(16, gRenderSpec.height - 24, 3, 0);
	DrawNumber(44, gRenderSpec.height - 24 + 2, gLives, 0, false);
}