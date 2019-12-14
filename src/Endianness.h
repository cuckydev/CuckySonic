#pragma once
#include <stdint.h>

//A hacky way to detect endianness
static const uint8_t endDec[4] = {'A','B','C','D'};
#if ((uint32_t)*endDec == 0x41424344)
	#define ENDIAN_BIG
#else
	#define ENDIAN_LIL
#endif
