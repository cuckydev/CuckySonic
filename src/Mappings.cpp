#include <stdlib.h>
#include <string.h>

#include "SDL_rwops.h"
#include "Mappings.h"
#include "Path.h"
#include "Log.h"

MAPPINGS::MAPPINGS(MAPPINGS **linkedList, const char *path)
{
	LOG(("Loading mappings from %s... ", path));
	
	memset(this, 0, sizeof(MAPPINGS));
	
	//Open the file given
	source = path;
	
	GET_GLOBAL_PATH(filepath, path);
	SDL_RWops *fp = SDL_RWFromFile(filepath, "rb");
	if (fp == nullptr)
	{
		fail = SDL_GetError();
		return;
	}
	
	//Allocate our frames
	size = SDL_RWsize(fp) / (6 * 2);
	rect = (SDL_Rect*)calloc(size, sizeof(SDL_Rect));
	origin = (SDL_Point*)calloc(size, sizeof(SDL_Point));
	
	if (rect == nullptr || origin == nullptr)
	{
		SDL_RWclose(fp);
		free(rect);
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
		origin[i].x = (int16_t)SDL_ReadBE16(fp);
		origin[i].y = (int16_t)SDL_ReadBE16(fp);
	}
	
	SDL_RWclose(fp);
	
	//Attach to linked list if given
	if (linkedList != nullptr)
	{
		list = linkedList;
		next = *linkedList;
		*linkedList = this;
	}
	
	LOG(("Success!\n"));
}

MAPPINGS::~MAPPINGS()
{
	//Free allocated data
	free(rect);
	free(origin);
}
