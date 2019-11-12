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
		const char *fail;
		
		//Tile colours
		PALCOLOUR tile1, tile2;
		
		//Loaded data
		TEXTURE *stageTexture;
		TEXTURE *sphereTexture;
		TEXTURE *backgroundTexture;
		
		uint8_t *perspectiveMap;
		
		//Music
		MUSIC *music;
		
		//Stage layout
		uint16_t width, height;
		uint8_t *layout;
		
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
			int8_t turn;		//Player's turning speed (-4 = left, 4 = right, 0 = no turning)
			int16_t speed;		//Player's current speed (negative = moving backwards) (8.8)
			bool isForward;		//SCHG: Set if you're moving forward (maybe a bumper thing?)
			
			struct
			{
				bool inAir : 1;
				bool spring : 1;
			} jumping;
		} playerState;
		
		//Stage state
		uint16_t spheresLeft;
		
		uint16_t animFrame;
		uint16_t paletteFrame;
		
		uint16_t rate;
		uint16_t rateTimer;
		
		int backX;
		int backY;
		
	public:
		SPECIALSTAGE(const char *name);
		~SPECIALSTAGE();
		
		void Update();
		
		void PalCycle();
		
		void Draw();
};
