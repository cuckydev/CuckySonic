#pragma once
#include "SDL_render.h"

class MAPPINGS
{
	public:
		int size;
		SDL_Rect *rect;
		SDL_Point *origin;
		
		const char *fail;
		
	public:
		MAPPINGS(const char *path);
		~MAPPINGS();
};
