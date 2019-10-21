#pragma once
#include <deque>
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
		
	public:
		MAPPINGS(std::deque<MAPPINGS*> *linkedList, const char *path);
		~MAPPINGS();
};
