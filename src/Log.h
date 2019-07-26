#pragma once
#include <stdio.h>

#ifdef DEBUG
	#define LOG(tuple)	printf( tuple )
#else
	#define LOG(tuple)	;
#endif
