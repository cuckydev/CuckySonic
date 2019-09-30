#include "SDL_rwops.h"
#include "BackgroundScroll.h"
#include "Path.h"
#include "Error.h"

BACKGROUNDSCROLL::BACKGROUNDSCROLL(const char *name, TEXTURE *backgroundTexture)
{
	//Clear memory
	memset(this, 0, sizeof(BACKGROUNDSCROLL));
	
	//Open the .bsc file
	GET_GLOBAL_PATH(path, name);
	
	SDL_RWops *fp = SDL_RWFromFile(path, "rb");
	if (fp == nullptr)
	{
		fail = SDL_GetError();
		return;
	}
	
	//Background scroll variables
	multiplierX = (int8_t)SDL_ReadU8(fp);
	divisorX = (int8_t)SDL_ReadU8(fp);
	addX = (int8_t)SDL_ReadU8(fp);
	
	multiplierY = (int8_t)SDL_ReadU8(fp);
	divisorY = (int8_t)SDL_ReadU8(fp);
	addY = (int8_t)SDL_ReadU8(fp);
	
	//Read offset arrays
	const uint16_t offsets = SDL_ReadBE16(fp);
	offset = (BACKOFFSET*)malloc(offsets * sizeof(BACKOFFSET));
	
	for (uint16_t i = 0; i < offsets; i++)
	{
		//Read all entries
		offset[i].length = SDL_ReadBE16(fp);
		offset[i].offset = (int16_t*)malloc(offset[i].length * sizeof(int16_t));
		
		for (uint16_t v = 0; v < offset[i].length; v++)
			offset[i].offset[v] = (int16_t)SDL_ReadBE16(fp);
	}
	
	//Read timers
	timers = SDL_ReadBE16(fp);
	timer = (BACKTIMER*)malloc(timers * sizeof(BACKTIMER));
	
	for (uint16_t i = 0; i < timers; i++)
	{
		//Read timer data
		timer[i].frames = SDL_ReadBE16(fp);
		timer[i].increment = SDL_ReadBE16(fp);
		timer[i].value = SDL_ReadBE16(fp);
		timer[i].max = SDL_ReadBE16(fp);
	}
	
	//Read constant scrolls
	scrolls = SDL_ReadBE16(fp);
	scroll = (BACKSCROLL*)malloc(scrolls * sizeof(BACKSCROLL));
	
	for (uint16_t i = 0; i < scrolls; i++)
	{
		//Read scroll data
		scroll[i].scrollIncrement = (int32_t)SDL_ReadBE32(fp);
		scroll[i].value = (int32_t)SDL_ReadBE32(fp);
	}
	
	//Read parallax scrolls
	parallaxScrolls = SDL_ReadBE16(fp);
	parallax = (BACKPARALLAX*)malloc(parallaxScrolls * sizeof(BACKPARALLAX));
	
	for (uint16_t i = 0; i < parallaxScrolls; i++)
	{
		//Read multiplier and divisor
		parallax[i].multiplier = (int8_t)SDL_ReadU8(fp);
		parallax[i].divisor = (int8_t)SDL_ReadU8(fp);
	}
	
	//Read strips
	strips = SDL_ReadBE16(fp);
	strip = (BACKSTRIP*)malloc(strips * sizeof(BACKSTRIP));
	
	for (uint16_t i = 0; i < strips; i++)
	{
		//Read strip data
		strip[i].stripHeight = SDL_ReadBE16(fp);
		
		//Offset array index
		const uint16_t offsetIndex = SDL_ReadBE16(fp);
		strip[i].offset = (offsetIndex == BACK_NULLVAL) ? nullptr : &offset[offsetIndex];
		
		//Timer index
		const uint16_t timerIndex = SDL_ReadBE16(fp);
		strip[i].timer = (timerIndex == BACK_NULLVAL) ? nullptr : &timer[timerIndex].value;
		
		//Offset array reference mode
		strip[i].offsetMode = (OFFSETMODE)SDL_ReadU8(fp);
		
		//Scroll from...
		const uint16_t fromScrollIndex = SDL_ReadBE16(fp);
		const uint16_t fromParallaxIndex = SDL_ReadBE16(fp);
		strip[i].fromScroll = (fromScrollIndex == BACK_NULLVAL) ? nullptr : &scroll[fromScrollIndex].value;
		strip[i].fromParallax = (fromParallaxIndex == BACK_NULLVAL) ? nullptr : &parallax[fromParallaxIndex].value;
		
		//Scroll to...
		const uint16_t toScrollIndex = SDL_ReadBE16(fp);
		const uint16_t toParallaxIndex = SDL_ReadBE16(fp);
		strip[i].toScroll = (toScrollIndex == BACK_NULLVAL) ? nullptr : &scroll[toScrollIndex].value;
		strip[i].toParallax = (toParallaxIndex == BACK_NULLVAL) ? nullptr : &parallax[toParallaxIndex].value;
	}
	
	//We're done reading the file
	SDL_RWclose(fp);
	
	//Assign to our reference texture
	referenceBackgroundTexture = backgroundTexture;
	scrollArray = (uint16_t*)malloc(referenceBackgroundTexture->height * sizeof(uint16_t));
}

BACKGROUNDSCROLL::~BACKGROUNDSCROLL()
{
	//Free everything
	free(offset);
	free(timer);
	free(scroll);
	free(parallax);
	free(strip);
	free(scrollArray);
}

void BACKGROUNDSCROLL::GetScroll(bool doScroll, int16_t cameraX, int16_t cameraY, int16_t *backX, int16_t *backY)
{
	//Update our parallax and background scrolling to the camera position
	if (backX != nullptr)
		*backX = cameraX * multiplierX / divisorX + addX;
	if (backY != nullptr)
		*backY = cameraY * multiplierY / divisorY + addY;
	
	for (uint16_t i = 0; i < parallaxScrolls; i++)
		parallax[i].value = (cameraX << 16) * parallax[i].multiplier / parallax[i].divisor;
	
	//Handle our scrolling with strips
	uint16_t lines = 0;
	uint16_t *line = scrollArray;
	
	for (uint16_t i = 0; i < strips; i++)
	{
		//Get how many lines to render (0xFFFF goes to the bottom of the texture)
		uint16_t height = strip[i].stripHeight;
		if (height == BACK_NULLVAL)
			height = referenceBackgroundTexture->height - lines;
		
		//Get our scroll positions
		int64_t from = 0, to = 0;
		
		if (strip[i].fromScroll != nullptr)
			from += *strip[i].fromScroll;
		if (strip[i].fromParallax != nullptr)
			from += *strip[i].fromParallax;
		
		if (strip[i].toScroll != nullptr)
			to += *strip[i].toScroll;
		if (strip[i].toParallax != nullptr)
			to += *strip[i].toParallax;
		
		for (uint16_t v = 0; v < height; v++)
		{
			//Set this line's position
			*line++ = (from + ((to - from) * v / height)) >> 16;
			lines++;
		}
	}
	
	//Handle constant scrolling
	for (uint16_t i = 0; i < scrolls && doScroll; i++)
		scroll[i].value = (scroll[i].value + scroll[i].scrollIncrement) % (referenceBackgroundTexture->width << 16);
	
	//Handle timers
	for (uint16_t i = 0; i < timers; i++)
	{
		if (timer[i].frameCounter++ >= timer[i].frames)
			timer[i].value = (timer[i].value + timer[i].increment) % timer[i].max;
	}
}
