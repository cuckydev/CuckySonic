#pragma once
#include <stdint.h>
#include "Player.h"

class CAMERA
{
	public:
		int16_t x;
		int16_t y;
		
	public:
		CAMERA(PLAYER *trackPlayer);
		~CAMERA();
		void Track(PLAYER *trackPlayer);
};
