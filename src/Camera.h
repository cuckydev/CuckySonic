#pragma once
#include <stdint.h>
#include "Player.h"

class CAMERA
{
	public:
		int16_t x = 0;
		int16_t y = 0;
		
	public:
		CAMERA(PLAYER *trackPlayer);
		~CAMERA();
		void Track(PLAYER *trackPlayer);
};
