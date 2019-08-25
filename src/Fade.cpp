#include "Render.h"

bool FadeInFromBlack(PALCOLOUR *palColour)
{
	//Fade in blue
	if (palColour->b < palColour->ogb)
	{
		int16_t nextB = palColour->b + 0x22;
		
		if (nextB >= palColour->ogb)
		{
			palColour->b = palColour->ogb;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->b = nextB;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade in green
	if (palColour->g < palColour->ogg)
	{
		int16_t nextG = palColour->g + 0x22;
		
		if (nextG >= palColour->ogg)
		{
			palColour->g = palColour->ogg;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->g = nextG;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade in red
	if (palColour->r < palColour->ogr)
	{
		int16_t nextR = palColour->r + 0x22;
		
		if (nextR >= palColour->ogr)
		{
			palColour->r = palColour->ogr;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->r = nextR;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	return true;
}

bool FadeOutToBlack(PALCOLOUR *palColour)
{
	//Fade out red
	if (palColour->r > 0)
	{
		int16_t nextR = palColour->r - 0x22;
		
		if (nextR <= 0)
		{
			palColour->r = 0;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->r = nextR;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade out green
	if (palColour->g > 0)
	{
		int16_t nextG = palColour->g - 0x22;
		
		if (nextG <= 0)
		{
			palColour->g = 0;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->g = nextG;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade out blue
	if (palColour->b > 0)
	{
		int16_t nextB = palColour->b - 0x22;
		
		if (nextB <= 0)
		{
			palColour->b = 0;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->b = nextB;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	return true;
}

bool FadeInFromWhite(PALCOLOUR *palColour)
{
	//Fade in blue
	if (palColour->b > palColour->ogb)
	{
		int16_t nextB = palColour->b - 0x22;
		
		if (nextB <= palColour->ogb)
		{
			palColour->b = palColour->ogb;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->b = nextB;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade in green
	if (palColour->g > palColour->ogg)
	{
		int16_t nextG = palColour->g - 0x22;
		
		if (nextG <= palColour->ogg)
		{
			palColour->g = palColour->ogg;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->g = nextG;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade in red
	if (palColour->r > palColour->ogr)
	{
		int16_t nextR = palColour->r - 0x22;
		
		if (nextR <= palColour->ogr)
		{
			palColour->r = palColour->ogr;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->r = nextR;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	return true;
}

bool FadeOutToWhite(PALCOLOUR *palColour)
{
	//Fade out red
	if (palColour->r < 0xFF)
	{
		int16_t nextR = palColour->r + 0x22;
		
		if (nextR <= 0xFF)
		{
			palColour->r = 0xFF;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->r = nextR;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade out green
	if (palColour->g < 0xFF)
	{
		int16_t nextG = palColour->g + 0x22;
		
		if (nextG <= 0xFF)
		{
			palColour->g = 0xFF;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->g = nextG;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	//Fade out blue
	if (palColour->b < 0xFF)
	{
		int16_t nextB = palColour->b + 0x22;
		
		if (nextB <= 0xFF)
		{
			palColour->b = 0xFF;
			RegenPaletteColour(palColour);
		}
		else
		{
			palColour->b = nextB;
			RegenPaletteColour(palColour);
			return false;
		}
	}
	
	return true;
}

bool PaletteFadeInFromBlack(PALETTE *palette)
{
	bool finished = true;
	for (int i = 0; i < 0x100; i++)
		finished = FadeInFromBlack(&palette->colour[i]) ? finished : false;
	return finished;
}

bool PaletteFadeOutToBlack(PALETTE *palette)
{
	bool finished = true;
	for (int i = 0; i < 0x100; i++)
		finished = FadeOutToBlack(&palette->colour[i]) ? finished : false;
	return finished;
}

bool PaletteFadeInFromWhite(PALETTE *palette)
{
	bool finished = true;
	for (int i = 0; i < 0x100; i++)
		finished = FadeInFromWhite(&palette->colour[i]) ? finished : false;
	return finished;
}

bool PaletteFadeOutToWhite(PALETTE *palette)
{
	bool finished = true;
	for (int i = 0; i < 0x100; i++)
		finished = FadeOutToWhite(&palette->colour[i]) ? finished : false;
	return finished;
}

void FillPaletteBlack(PALETTE *palette)
{
	for (int i = 0; i < 0x100; i++)
		ModifyPaletteColour(&palette->colour[i], 0x00, 0x00, 0x00);
}

void FillPaletteWhite(PALETTE *palette)
{
	for (int i = 0; i < 0x100; i++)
		ModifyPaletteColour(&palette->colour[i], 0xFF, 0xFF, 0xFF);
}
