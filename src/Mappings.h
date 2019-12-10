#pragma once
#include <string>
#include "Render.h"

class MAPPINGS
{
	public:
		//Failure
		const char *fail = nullptr;
		
		//Source path
		std::string source;
		
		//Mappings
		size_t size = 0;
		RECT *rect = nullptr;
		POINT *origin = nullptr;
		
	public:
		MAPPINGS(std::string path);
		~MAPPINGS();
};
