#pragma once
#ifdef BACKEND_SDL2
	#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		#define CPU_BIGENDIAN
	#else
		#define CPU_LILENDIAN
	#endif
#endif
