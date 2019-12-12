#pragma once

//https://github.com/SDL-mirror/SDL/blob/master/include/SDL_endian.h
#ifndef SDL_BYTEORDER
	#ifdef __linux__
		//Linux - endian.h
		#include <endian.h>
		
		//__BYTE_ORDER = __BIG_ENDIAN or __LITTLE_ENDIAN
		#if __BYTE_ORDER == __BIG_ENDIAN
			#define ENDIAN BIG
		#else
			#define ENDIAN LIL
		#endif
	#elif defined(__OpenBSD__)
		//OpenBSD - endian.h
		#include <endian.h>
		
		//BYTE_ORDER = BIG_ENDIAN or LITTLE_ENDIAN
		#if __BYTE_ORDER == BIG_ENDIAN
			#define ENDIAN BIG
		#else
			#define ENDIAN LIL
		#endif
	#else
		//Other - based on preprocessor defines
		#if defined(__hppa__) || \
			defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
			(defined(__MIPS__) && defined(__MIPSEB__)) || \
			defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
			defined(__sparc__)
			#define ENDIAN BIG
		#else
			#define ENDIAN LIL
		#endif
	#endif
#endif
