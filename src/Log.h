#pragma once
#include <stdio.h>

//Log function, stub when non-debug build
#ifdef DEBUG
	#define LOG(tuple)	printf tuple
#else
	#define LOG(tuple)				//Valid?
#endif
