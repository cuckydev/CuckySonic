#include "SDL_rwops.h"
#include "Mappings.h"
#include "Path.h"

MAPPINGS::MAPPINGS(const char *path)
{
	fail = NULL;
	
	//Open the file given
	GET_GLOBAL_PATH(filepath, path);
	
	SDL_RWops *fp = SDL_RWFromFile(filepath, "rb");
	if (fp == NULL)
	{
		fail = SDL_GetError();
		return;
	}
	
	//Allocate our frames
	size = SDL_RWsize(fp) / (6 * 2);
	rect = (SDL_Rect*)calloc(size, sizeof(SDL_Rect));
	origin = (SDL_Point*)calloc(size, sizeof(SDL_Point));
	
	if (rect == NULL || origin == NULL)
	{
		SDL_RWclose(fp);
		
		free(rect); //Free in the case that one successfully allocated
		free(origin);
		
		fail = "Failed to allocate rect and origin arrays";
		return;
	}
	
	//Read from the file
	for (int i = 0; i < size; i++)
	{
		rect[i].x = SDL_ReadBE16(fp);
		rect[i].y = SDL_ReadBE16(fp);
		rect[i].w = SDL_ReadBE16(fp);
		rect[i].h = SDL_ReadBE16(fp);
		origin[i].x = SDL_ReadBE16(fp);
		origin[i].y = SDL_ReadBE16(fp);
	}
	
	//Close file
	SDL_RWclose(fp);
}

MAPPINGS::~MAPPINGS()
{
	//Free allocated data
	free(rect);
	free(origin);
}
