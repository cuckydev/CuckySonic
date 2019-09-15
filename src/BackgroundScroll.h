#pragma once
#include <stdint.h>
#include "Render.h"

#define BACK_NULLVAL	0xFFFF

//All enums and structs
enum OFFSETMODE
{
	OFFSETMODE_NONE,		//No offset, just directly from assigned timer
	OFFSETMODE_STRIP,		//Offset by the strip's index
	OFFSETMODE_STRIP_Y,		//Offset by the strip's Y-position
	OFFSETMODE_GLOBAL_Y,	//Offset by the screen Y-position
	OFFSETMODE_LOCAL_Y,		//Offset by the Y-position in the strip
};

struct BACKOFFSET
{
	uint16_t length;
	int16_t *offset;	//Offset entries, 2 bytes each
};

struct BACKTIMER
{
	uint16_t frames, frameCounter;	//How many frames until update
	uint16_t increment;				//How much to increment the value each update
	uint16_t value;					//Current value
	uint16_t max;					//Maximum value, wraps around
};

struct BACKSCROLL
{
	int32_t scrollIncrement;	//How much to increment the below by
	int32_t value;				//Scroll value (16.16 fixed point)
};

struct BACKPARALLAX
{
	int8_t multiplier, divisor;	//Multiplier and divisor (value = cameraX * multiplier / divisor)
	int32_t value;				//Scroll value (16.16 fixed point)
};

struct BACKSTRIP
{
	//Strip height
	uint16_t stripHeight;
	
	//Offset and timer
	const BACKOFFSET *offset;
	const uint16_t *timer;
	OFFSETMODE offsetMode;
	
	//From and to scroll / parallax values
	const int32_t *fromScroll;
	const int32_t *fromParallax;
	const int32_t *toScroll;
	const int32_t *toParallax;
};

//Background scroller class
class BACKGROUNDSCROLL
{
	public:
		//Failure
		const char *fail;
		
		//Scrolling variables
		int8_t multiplierX, divisorX, addX;	//Used for scrolling entire background, Multiplier, divisor, and add (value = cameraX * multiplier / divisor + add)
		int8_t multiplierY, divisorY, addY;	//Used for scrolling entire background, Multiplier, divisor, and add (value = cameraY * multiplier / divisor + add)
		
		BACKOFFSET *offset;
		
		uint16_t timers;
		BACKTIMER *timer;
		
		uint16_t scrolls;
		BACKSCROLL *scroll;
		
		uint16_t parallaxScrolls;
		BACKPARALLAX *parallax;
		
		uint16_t strips;
		BACKSTRIP *strip;
		
		//Reference texture and the output scroll array
		const TEXTURE *referenceBackgroundTexture;
		uint16_t *scrollArray;
	
	public:
		BACKGROUNDSCROLL(const char *name, TEXTURE *backgroundTexture);
		~BACKGROUNDSCROLL();
		
		void GetScroll(int16_t cameraX, int16_t cameraY, int16_t *backX, int16_t *backY);
};
