#pragma once
#include <stdint.h>
#include "Render.h"
#include "CommonMacros.h"
#include "Audio.h"

enum SPECIALSTAGE_RENDERLAYER
{
	SPECIALSTAGE_RENDERLAYER_PLAYERS,
	SPECIALSTAGE_RENDERLAYER_SPHERES,
	SPECIALSTAGE_RENDERLAYER_STAGE,
	SPECIALSTAGE_RENDERLAYER_BACKGROUND,
};

class SPECIALSTAGE
{
	public:
		//Failure
		const char *fail = nullptr;
		
		//Tile colours
		COLOUR tile1, tile2;
		
		//Loaded data
		TEXTURE *stageTexture = nullptr;
		TEXTURE *sphereTexture = nullptr;
		TEXTURE *backgroundTexture = nullptr;
		
		//Stage layout
		size_t width = 0, height = 0;
		uint8_t *layout = nullptr;
		
		//Player's state
		struct
		{
			//Position (Extended from 8.8 of the original game to 16.16, to support larger stages)
			POSDEF(x)
			POSDEF(y)
			
			POSDEF(prevX)
			POSDEF(prevY)
			
			//Movement direction and speed
			uint8_t direction;	//Player's facing direction
			int8_t turn;		//Player's turning speed (negative = left, positive = right, 0 = no turning)
			int16_t speed;		//Player's current speed (negative = moving backwards) (8.8)
			bool isForward;		//SCHG: Set if you're moving forward (maybe a bumper thing?)
			
			struct
			{
				bool inAir : 1;
				bool spring : 1;
			} jumping;
		} playerState;
		
		//Stage state
		unsigned int ringsLeft = 0;
		
		unsigned int animFrame = 0;
		unsigned int paletteFrame = 0;
		
		unsigned int rate = 0;
		unsigned int rateTimer = 0;
		
		int backX = 0;
		int backY = 0;
		
	public:
		SPECIALSTAGE(std::string name);
		~SPECIALSTAGE();
		
		void Update();
		
		void PalCycle();
		
		void Draw();
};
