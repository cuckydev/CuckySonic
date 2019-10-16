#pragma once
#include "Render.h"

class MAPPINGS
{
	public:
		//Failure
		const char *fail;
		
		//Source path
		const char *source;
		
		//Mappings
		int size;
		RECT *rect;
		POINT *origin;
		
		//Linked list
		MAPPINGS **list;
		MAPPINGS *next;
		
	public:
		MAPPINGS(MAPPINGS **linkedList, const char *path);
		~MAPPINGS();
};
