#pragma once
#include <stdint.h>
#include "Player.h"

class CAMERA
{
	public:
		int16_t x;
		int16_t y;
		
	public:
		CAMERA(void *level, PLAYER *trackPlayer);
		~CAMERA();
		void Track(void *level, PLAYER *trackPlayer);
};
