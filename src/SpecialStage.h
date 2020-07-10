#pragma once
#include <stdint.h>
#include "Render.h"
#include "CommonMacros.h"

//Enumerations
enum SPECIALSTAGE_RENDERLAYER
{
	SPECIALSTAGE_RENDERLAYER_HUD,
	SPECIALSTAGE_RENDERLAYER_PLAYERS,
	SPECIALSTAGE_RENDERLAYER_SPHERES,
	SPECIALSTAGE_RENDERLAYER_STAGE,
	SPECIALSTAGE_RENDERLAYER_BACKGROUND,
};

//Constants
#define SS_WIDTH	0x20
#define SS_HEIGHT	0x20

//Special stage class
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
		uint8_t layout[SS_WIDTH * SS_HEIGHT];
		
		//Player state
		struct
		{
			//Position
			FPDEF(x, uint8_t, pos, uint8_t, sub, uint16_t)
			FPDEF(y, uint8_t, pos, uint8_t, sub, uint16_t)
			
			FPDEF(lastX, uint8_t, pos, uint8_t, sub, uint16_t)
			FPDEF(lastY, uint8_t, pos, uint8_t, sub, uint16_t)
			
			//Movement
			uint8_t angle = 0;		//(00 = North, 40 = West, 80 = South, C0 = East)
			int8_t turn = 0;
			bool turnLock = false;	
			
			int16_t velocity = 0;
			
			bool bumperLock = false;
			bool advancing = false;
			bool started = false;
			bool isForward = false;
			
			uint8_t jumping = 0;
			
			uint8_t clearRoutine = 0;
			
			uint16_t interact = 0;
		} player;
		
		//Stage state
		unsigned int ringsLeft = 0;
		
		uint16_t animFrame = 0;
		uint8_t paletteFrame = 0;
		
		int16_t rate = 0;
		int16_t rateTimer = 0;
		
		//Background position
		int backX = 0, backY = 0;
		
	public:
		SPECIALSTAGE(std::string name);
		~SPECIALSTAGE();
		
		void SpeedupStage();
		void MovePlayer();
		void Update();
		
		void RotatePalette();
		void UpdateStageFrame();
		void UpdateBackgroundPosition();
		void Draw();
};
