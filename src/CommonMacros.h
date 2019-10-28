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
			int32_t	axis##PosLong = 0;	\
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
			int32_t	axis##PosLong = 0;	\
		};
#endif
