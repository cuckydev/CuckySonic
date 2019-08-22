#pragma once
#include <stdint.h>
#include "Player.h"

class CAMERA
{
	public:
		int16_t x;
		int16_t y;
		int16_t xPan;
		int16_t yPan;
		int16_t lookPan;
		int16_t lookTimer;
		
	public:
		CAMERA(PLAYER *trackPlayer);
		~CAMERA();
		void Track(PLAYER *trackPlayer);
};
