#pragma once
#include <stdint.h>
#include "Player.h"

//Camera
class CAMERA
{
	public:
		int16_t xPos;
		int16_t yPos;
		int16_t xPan = 0;
		int16_t yPan = 0;
		int16_t lookPan = 0;
		int16_t lookTimer = 0;
		
		uint16_t shake = 0;
		
	public:
		CAMERA(PLAYER *trackPlayer);
		~CAMERA();
		void Track(PLAYER *trackPlayer);
};
