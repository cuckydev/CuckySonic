#include "Camera.h"
#include "Game.h"
#include "Level.h"
#include "MathUtil.h"
#include "Log.h"

//Bug-fixes
//#define FIX_CAMERA_YSHIFT    //This makes the camera shifting for when rolling less bad

//Optional differences
//#define CAMERA_CD_PAN //NOTE: pretty crappy since it only really works in CD because it has a lot of crappy flat ground and stuff

//Constants
#define CAMERA_GROUND_YMARGIN_BOTTOM 0
#define CAMERA_GROUND_YMARGIN_OFFSET 16

#define CAMERA_AIR_YMARGIN_BOTTOM 64
#define CAMERA_AIR_YMARGIN_OFFSET 48

#define CAMERA_XMARGIN_RIGHT 16
#define CAMERA_XMARGIN_OFFSET 16

#define LOOK_PANTIME 120

#define approach(var, to, am)	if (var < to)	\
								{	\
									var += am;	\
									if (var > to)	\
										var = to;	\
								}	\
								if (var > to)	\
								{	\
									var -= am;	\
									if (var < to)	\
										var = to;	\
								}

CAMERA::CAMERA(PLAYER *trackPlayer)
{
	//Clear memory
	memset(this, 0, sizeof(CAMERA));
	
	//Attach to our given player
	x = trackPlayer->x.pos - (SCREEN_WIDTH / 2);
	y = trackPlayer->y.pos - (SCREEN_HEIGHT / 2 - 16);
	Track(trackPlayer);
	return;
}

CAMERA::~CAMERA()
{
	//Nothing to free
	return;
}

void CAMERA::Track(PLAYER *trackPlayer)
{
	//Don't move if locked
	if (trackPlayer->cameraLock)
		return;
	
	//Get our track positions
	int trackX = trackPlayer->x.pos, trackY = trackPlayer->y.pos;
	
	if (trackPlayer->scrollDelay)
	{
		trackPlayer->scrollDelay -= 0x100;
		trackX = trackPlayer->posRecord[(trackPlayer->recordPos - ((trackPlayer->scrollDelay / 0x100) + 1)) % PLAYER_RECORD_LENGTH].x;
	}
	
	//Camera panning
	#ifdef CAMERA_CD_PAN
		int16_t speed = trackPlayer->inertia;
		
		if (!trackPlayer->scrollDelay) //Don't increase if scroll is delayed
		{
			if (abs(speed) >= 0x600 || trackPlayer->spindashing)
			{
				//Pan towards our moving direction
				approach(xPan, ((trackPlayer->spindashing ? trackPlayer->status.xFlip : speed < 0) ? -64 : 64), 2);
			}
			else
			{
				//Unpan
				approach(xPan, 0, 2);
			}
		}
		
		//Pan our track position
		trackX += xPan;
	#endif
	
	#ifndef FIX_CAMERA_YSHIFT
		//Shift downwards when rolling
		if (trackPlayer->status.inBall)
			trackY -= (trackPlayer->defaultYRadius - trackPlayer->rollYRadius);
	#else
		//Shift downwards when rolling
		uint8_t difference = trackPlayer->defaultYRadius - trackPlayer->yRadius;
	
		int16_t sin, cos;
		GetSine(trackPlayer->angle - 0x40, &sin, &cos);
		
		trackX += cos * difference / 0x100;
		trackY += sin * difference / 0x100;
	#endif
	
	//Handle looking up and down
	if (trackPlayer->anim == PLAYERANIMATION_LOOKUP || trackPlayer->anim == PLAYERANIMATION_DUCK)
	{
		if (lookTimer < LOOK_PANTIME)
		{
			//Increase timer and unpan
			lookTimer++;
			approach(lookPan, 0, 2);
		}
		else
		{
			//Pan to our intended direction
			approach(lookPan, (trackPlayer->anim == PLAYERANIMATION_LOOKUP ? -104 : 88), 2);
		}
	}
	else
	{
		//Unpan
		lookTimer = 0;
		approach(lookPan, 0, 2);
	}
	
	trackY += lookPan;
	
	//Scroll horizontally to our tracked position
	int rDiff = (trackX - x) - (SCREEN_WIDTH / 2 + CAMERA_XMARGIN_RIGHT - CAMERA_XMARGIN_OFFSET);
	int lDiff = (SCREEN_WIDTH / 2 - CAMERA_XMARGIN_OFFSET) - (trackX - x);
	
	if (rDiff > 0)
		x += (rDiff > 16 ? 16 : rDiff);
	if (lDiff > 0)
		x -= (lDiff > 16 ? 16 : lDiff);
	
	//Scroll vertically to our tracked position
	int tDiff, bDiff, vScroll;
	
	if (trackPlayer->status.inAir)
	{
		tDiff = (SCREEN_HEIGHT / 2 - CAMERA_AIR_YMARGIN_OFFSET) - (trackY - y);
		bDiff = (trackY - y) - (SCREEN_HEIGHT / 2 + CAMERA_AIR_YMARGIN_BOTTOM - CAMERA_AIR_YMARGIN_OFFSET);
		vScroll = 16;
	}
	else
	{
		tDiff = (SCREEN_HEIGHT / 2 - CAMERA_GROUND_YMARGIN_OFFSET) - (trackY - y);
		bDiff = (trackY - y) - (SCREEN_HEIGHT / 2 + CAMERA_GROUND_YMARGIN_BOTTOM - CAMERA_GROUND_YMARGIN_OFFSET);
		vScroll = abs(trackPlayer->inertia) >= 0x800 ? 16 : 6;
	}
	
	if (bDiff > 0)
		y += (bDiff > vScroll ? vScroll : bDiff);
	if (tDiff > 0)
		y -= (tDiff > vScroll ? vScroll : tDiff);
	
	//Keep inside of level boundaries
	if (x < gLevel->leftBoundary)
		x = gLevel->leftBoundary;
	if (y < gLevel->topBoundary)
		y = gLevel->topBoundary;
	if (x > gLevel->rightBoundary - SCREEN_WIDTH)
		x = gLevel->rightBoundary - SCREEN_WIDTH;
	if (y > gLevel->bottomBoundary - SCREEN_HEIGHT)
		y = gLevel->bottomBoundary - SCREEN_HEIGHT;
}
