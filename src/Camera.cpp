#include "Camera.h"
#include "Game.h"
#include "Level.h"
#include "Log.h"

//#define CAMERA_CENTERED
//#define CAMERA_CD_PAN
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
	#if defined(CAMERA_CD_PAN)
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
	
	//Shift downwards when rolling
	if (trackPlayer->status.inBall)
		trackY -= (trackPlayer->defaultYRadius - trackPlayer->rollYRadius);
	
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
	#ifdef CAMERA_CENTERED
		int rDiff = (trackX - x) - (SCREEN_WIDTH / 2);
		int lDiff = (SCREEN_WIDTH / 2) - (trackX - x);
	#else
		int rDiff = (trackX - x) - (SCREEN_WIDTH / 2);
		int lDiff = ((SCREEN_WIDTH / 2) - 16) - (trackX - x);
	#endif
	
	if (rDiff > 0)
		x += (rDiff > 16 ? 16 : rDiff);
	if (lDiff > 0)
		x -= (lDiff > 16 ? 16 : lDiff);
	
	//Scroll vertically to our tracked position
	int tDiff, bDiff, vScroll;
	
	if (trackPlayer->status.inAir)
	{
		tDiff = ((SCREEN_HEIGHT / 2) - 48) - (trackY - y);
		bDiff = (trackY - y) - ((SCREEN_HEIGHT / 2) + 16);
		vScroll = 16;
	}
	else
	{
		tDiff = ((SCREEN_HEIGHT / 2) - 16) - (trackY - y);
		bDiff = (trackY - y) - ((SCREEN_HEIGHT / 2) - 16);
		vScroll = abs(trackPlayer->inertia) >= 0x800 ? 16 : 6;
	}
	
	if (bDiff > 0)
		y += (bDiff > vScroll ? vScroll : bDiff);
	if (tDiff > 0)
		y -= (tDiff > vScroll ? vScroll : tDiff);
	
	//Keep inside of level boundaries
	if (x < gLevel->leftBoundary2)
		x = gLevel->leftBoundary2;
	if (y < gLevel->topBoundary2)
		y = gLevel->topBoundary2;
	if (x > gLevel->rightBoundary2 - SCREEN_WIDTH)
		x = gLevel->rightBoundary2 - SCREEN_WIDTH;
	if (y > gLevel->bottomBoundary2 - SCREEN_HEIGHT)
		y = gLevel->bottomBoundary2 - SCREEN_HEIGHT;
}
