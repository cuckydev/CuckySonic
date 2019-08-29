#pragma once
#include "SDL_render.h"

class MAPPINGS
{
	public:
		//Failure
		const char *fail;
		
		//Source path
		const char *source;
		
		//Mappings
		int size;
		SDL_Rect *rect;
		SDL_Point *origin;
		
		//Linked list
		MAPPINGS **list;
		MAPPINGS *next;
		
	public:
		MAPPINGS(MAPPINGS **linkedList, const char *path);
		~MAPPINGS();
};
