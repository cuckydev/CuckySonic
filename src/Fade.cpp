#include "Render.h"

//Constants
#define FADE_INCREMENT 0x22	//How much to increment colour values during a fade

//[[Single colour operations]]
//Fading in/out black
bool FadeInFromBlack(COLOUR *colour)
{
	//Fade in blue
	if (colour->b < colour->mb)
	{
		int16_t nextB = colour->b + FADE_INCREMENT;
		
		if (nextB >= colour->mb)
		{
			colour->b = colour->mb;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->b = nextB;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade in green
	if (colour->g < colour->mg)
	{
		int16_t nextG = colour->g + FADE_INCREMENT;
		
		if (nextG >= colour->mg)
		{
			colour->g = colour->mg;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->g = nextG;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade in red
	if (colour->r < colour->mr)
	{
		int16_t nextR = colour->r + FADE_INCREMENT;
		
		if (nextR >= colour->mr)
		{
			colour->r = colour->mr;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->r = nextR;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	return true;
}

bool FadeOutToBlack(COLOUR *colour)
{
	//Fade out red
	if (colour->r > 0)
	{
		int16_t nextR = colour->r - FADE_INCREMENT;
		
		if (nextR <= 0)
		{
			colour->r = 0;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->r = nextR;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade out green
	if (colour->g > 0)
	{
		int16_t nextG = colour->g - FADE_INCREMENT;
		
		if (nextG <= 0)
		{
			colour->g = 0;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->g = nextG;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade out blue
	if (colour->b > 0)
	{
		int16_t nextB = colour->b - FADE_INCREMENT;
		
		if (nextB <= 0)
		{
			colour->b = 0;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->b = nextB;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	return true;
}

//Fading in/out white
bool FadeInFromWhite(COLOUR *colour)
{
	//Fade in blue
	if (colour->b > colour->mb)
	{
		int16_t nextB = colour->b - FADE_INCREMENT;
		
		if (nextB <= colour->mb)
		{
			colour->b = colour->mb;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->b = nextB;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade in green
	if (colour->g > colour->mg)
	{
		int16_t nextG = colour->g - FADE_INCREMENT;
		
		if (nextG <= colour->mg)
		{
			colour->g = colour->mg;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->g = nextG;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade in red
	if (colour->r > colour->mr)
	{
		int16_t nextR = colour->r - FADE_INCREMENT;
		
		if (nextR <= colour->mr)
		{
			colour->r = colour->mr;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->r = nextR;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	return true;
}

bool FadeOutToWhite(COLOUR *colour)
{
	//Fade out red
	if (colour->r < 0xFF)
	{
		int16_t nextR = colour->r + FADE_INCREMENT;
		
		if (nextR >= 0xFF)
		{
			colour->r = 0xFF;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->r = nextR;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade out green
	if (colour->g < 0xFF)
	{
		int16_t nextG = colour->g + FADE_INCREMENT;
		
		if (nextG >= 0xFF)
		{
			colour->g = 0xFF;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->g = nextG;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	//Fade out blue
	if (colour->b < 0xFF)
	{
		int16_t nextB = colour->b + FADE_INCREMENT;
		
		if (nextB >= 0xFF)
		{
			colour->b = 0xFF;
			colour->Regen(colour->r, colour->g, colour->b);
		}
		else
		{
			colour->b = nextB;
			colour->Regen(colour->r, colour->g, colour->b);
			return false;
		}
	}
	
	return true;
}

//[[Full palette operations]]
//Fading in/out black
bool PaletteFadeInFromBlack(PALETTE *palette)
{
	bool finished = true;
	for (size_t i = 0; i < palette->colours; i++)
		finished = FadeInFromBlack(&palette->colour[i]) ? finished : false;
	return finished;
}

bool PaletteFadeOutToBlack(PALETTE *palette)
{
	bool finished = true;
	for (size_t i = 0; i < palette->colours; i++)
		finished = FadeOutToBlack(&palette->colour[i]) ? finished : false;
	return finished;
}

//Fading in/out white
bool PaletteFadeInFromWhite(PALETTE *palette)
{
	bool finished = true;
	for (size_t i = 0; i < palette->colours; i++)
		finished = FadeInFromWhite(&palette->colour[i]) ? finished : false;
	return finished;
}

bool PaletteFadeOutToWhite(PALETTE *palette)
{
	bool finished = true;
	for (size_t i = 0; i < palette->colours; i++)
		finished = FadeOutToWhite(&palette->colour[i]) ? finished : false;
	return finished;
}

//Fill palette black / white
void FillPaletteBlack(PALETTE *palette)
{
	for (size_t i = 0; i < palette->colours; i++)
		palette->colour[i].SetColour(true, false, true, 0x00, 0x00, 0x00);
}

void FillPaletteWhite(PALETTE *palette)
{
	for (size_t i = 0; i < palette->colours; i++)
		palette->colour[i].SetColour(true, false, true, 0xFF, 0xFF, 0xFF);
}
