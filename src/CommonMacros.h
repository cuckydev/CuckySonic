#pragma once
#include "Endianness.h"

#ifdef CPU_BIGENDIAN
	#define POSDEF(axis)	\
		union	\
		{	\
			struct	\
			{	\
				int16_t pos;	\
				uint16_t sub;	\
			} axis;	\
			int32_t	axis##PosLong;	\
		};
#else
	#define POSDEF(axis)	\
		union	\
		{	\
			struct	\
			{	\
				uint16_t sub;	\
				int16_t pos;	\
			} axis;	\
			int32_t	axis##PosLong;	\
		};
#endif

#define DESTROY_LINKEDLIST_CONTENTS(linkedList)	for (signed int dllci = (signed int)linkedList.size() - 1; dllci >= 0; dllci--)	\
													delete linkedList[dllci];	\
												linkedList.clear();	\
												linkedList.shrink_to_fit();

inline char *duplicateString(const char *str)
{
	size_t length = strlen(str) + 1;
	char *retVal = new char[length];
	return static_cast<char *>(memcpy(retVal, str, length));
}
