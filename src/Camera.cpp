#include <string.h>

#include "Camera.h"
#include "Game.h"
#include "Level.h"
#include "MathUtil.h"
#include "Log.h"

//Define things
//#define CAMERA_CD_PAN
//#define CAMERA_ROLL_SHIFT_FIX

//Constants
#define CAMERA_HSCROLL_LEFT		-16
#define CAMERA_HSCROLL_SIZE		 16

#define CAMERA_VSCROLL_OFFSET	-16
#define CAMERA_VSCROLL_UP		 32
#define CAMERA_VSCROLL_DOWN		 32

#define LOOK_PANTIME	120
#define LOOK_PANSPEED	2
#define LOOK_PAN_UP		104
#define LOOK_PAN_DOWN	88

#define CD_PAN_LEFT		64
#define CD_PAN_RIGHT	64
#define CD_PAN_SCROLL	2

CAMERA::CAMERA(PLAYER *trackPlayer)
{
	//Clear memory
	memset(this, 0, sizeof(CAMERA));
	
	//Move to our given player
	x = trackPlayer->x.pos - (gRenderSpec.width / 2);
	y = trackPlayer->y.pos - (gRenderSpec.height / 2 + CAMERA_VSCROLL_OFFSET);
	
	//Keep inside level boundaries
	if (x < gLevel->leftBoundary)
		x = gLevel->leftBoundary;
	if (x + gRenderSpec.width > gLevel->rightBoundary)
		x = gLevel->rightBoundary - gRenderSpec.width;
	if (y < gLevel->topBoundary)
		y = gLevel->topBoundary;
	if (y + gRenderSpec.height > gLevel->bottomBoundary)
		y = gLevel->bottomBoundary - gRenderSpec.height;
	return;
}

CAMERA::~CAMERA()
{
	return;
}

void CAMERA::Track(PLAYER *trackPlayer)
{
	#ifdef CAMERA_CD_PAN
		//Handle CD panning
		if (trackPlayer->spindashing)
		{
			if (trackPlayer->status.xFlip)
				xPan = min(xPan + CD_PAN_SCROLL,  CD_PAN_LEFT);
			else
				xPan = max(xPan - CD_PAN_SCROLL, -CD_PAN_RIGHT);
		}
		else if (abs(trackPlayer->inertia) >= 0x600)
		{
			if (trackPlayer->inertia < 0)
				xPan = min(xPan + CD_PAN_SCROLL,  CD_PAN_LEFT);
			else
				xPan = max(xPan - CD_PAN_SCROLL, -CD_PAN_RIGHT);
		}
		else
		{
			//Pan back to the center
			if (xPan > 0)
				xPan = max(xPan - CD_PAN_SCROLL,   0);
			else if (xPan < 0)
				xPan = min(xPan + CD_PAN_SCROLL,   0);
		}
	#endif
	
	//Handle panning while player is looking up / down
	if (trackPlayer->status.inAir == false && trackPlayer->anim == PLAYERANIMATION_LOOKUP)
	{
		//Wait LOOK_PANTIME frames, then look LOOK_PAN_UP pixels
		if (lookTimer >= LOOK_PANTIME)
			lookPan = min(lookPan + LOOK_PANSPEED, LOOK_PAN_UP);
		else
			lookTimer++;
	}
	else if (trackPlayer->status.inAir == false && trackPlayer->anim == PLAYERANIMATION_DUCK)
	{
		//Wait LOOK_PANTIME frames, then look LOOK_PAN_DOWN pixels
		if (lookTimer >= LOOK_PANTIME)
			lookPan = max(lookPan - LOOK_PANSPEED, -LOOK_PAN_DOWN);
		else
			lookTimer++;
	}
	else
	{
		//Reset timer
		lookTimer = 0;
	}
	
	//Reset panning if the timer is below our look time
	if (lookTimer < LOOK_PANTIME)
	{
		if (lookPan < 0)
			lookPan = min(lookPan + LOOK_PANSPEED, 0);
		if (lookPan > 0)
			lookPan = max(lookPan - LOOK_PANSPEED, 0);
	}
	
	//Don't move if locked
	if (trackPlayer->cameraLock)
		return;
	
	//Get our height shifting (shorter players and/or rolling)
	int8_t shift = trackPlayer->defaultYRadius - trackPlayer->yRadius;
	int16_t xShift = 0, yShift = shift;
	
#ifdef CAMERA_ROLL_SHIFT_FIX
	GetSine(trackPlayer->angle + 0x40, &yShift, &xShift);
	xShift = (xShift * shift) / 0x100;
	yShift = (yShift * shift) / 0x100;
#endif
	
	//Scroll horizontally to the player
	int16_t trackX = trackPlayer->x.pos;
	
	if (trackPlayer->scrollDelay)
	{
		unsigned int lastScroll = trackPlayer->scrollDelay;
		if ((trackPlayer->scrollDelay -= 0x100) > lastScroll)
			trackPlayer->scrollDelay = 0;
		trackX = trackPlayer->posRecord[(trackPlayer->recordPos - ((trackPlayer->scrollDelay / 0x100) + 1)) % PLAYER_RECORD_LENGTH].x;
	}
	else
	{
		trackX += xShift;
	}
	
	int16_t hScrollOffset = trackX - x - xPan;
	
	if ((hScrollOffset -= (gRenderSpec.width / 2 + CAMERA_HSCROLL_LEFT)) < 0) //Scroll to the left
	{
		//Cap our scrolling to 16 pixels per frame
		if (hScrollOffset <= -16)
			hScrollOffset = -16;
		
		//Scroll and keep within level boundaries
		x += hScrollOffset;
		if (x < gLevel->leftBoundary)
			x = gLevel->leftBoundary;
	}
	else if ((hScrollOffset -= CAMERA_HSCROLL_SIZE) >= 0) //Scroll to the right
	{
		//Cap our scrolling to 16 pixels per frame
		if (hScrollOffset > 16)
			hScrollOffset = 16;
		
		//Scroll and keep within level boundaries
		x += hScrollOffset;
		if ((x + gRenderSpec.width) > gLevel->rightBoundary)
			x = gLevel->rightBoundary - gRenderSpec.width;
	}
	
	//Scroll vertically to the player
	int16_t vScrollOffset = trackPlayer->y.pos - y - (gRenderSpec.height / 2 + CAMERA_VSCROLL_OFFSET) - lookPan;
	
	if (trackPlayer->status.reverseGravity)
		vScrollOffset += yShift;
	else
		vScrollOffset -= yShift;
	
	//Handle our specific scrolling
	uint16_t scrollSpeed = 16;
	
	if (trackPlayer->status.inAir)
	{
		//Scrolling in mid-air
		if (vScrollOffset < -CAMERA_VSCROLL_UP)
			vScrollOffset += CAMERA_VSCROLL_UP;
		else if (vScrollOffset >= CAMERA_VSCROLL_DOWN)
			vScrollOffset -= CAMERA_VSCROLL_DOWN;
		else
			vScrollOffset = 0;
	}
	else
	{
		//Get our scroll speed
		if (lookPan)
			scrollSpeed = 2;
		else
			scrollSpeed = (abs(trackPlayer->inertia) >= 0x800) ? 16 : 6;
	}
	
	if (vScrollOffset < 0)
	{
		//Scroll upwards (cap to scrollSpeed)
		if (vScrollOffset <= -scrollSpeed)
			vScrollOffset = -scrollSpeed;
		y += vScrollOffset;
		
		//Keep within level boundaries
		if (y < gLevel->topBoundary)
			y = gLevel->topBoundary;
	}
	else if (vScrollOffset > 0)
	{
		//Scroll downwards (cap to scrollSpeed)
		if (vScrollOffset > scrollSpeed)
			vScrollOffset = scrollSpeed;
		y += vScrollOffset;
		
		//Keep within level boundaries
		if (y + gRenderSpec.height > gLevel->bottomBoundary)
			y = gLevel->bottomBoundary - gRenderSpec.height;
	}
}
