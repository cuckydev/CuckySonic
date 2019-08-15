#include "Camera.h"
#include "Level.h"
#include "GameConstants.h"

CAMERA::CAMERA(PLAYER *trackPlayer)
{
	x = trackPlayer->x.pos - (SCREEN_WIDTH / 2);
	y = trackPlayer->y.pos - (SCREEN_HEIGHT / 2);
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
	
	if (trackPlayer->status.inBall)
		trackY -= 5; //NOTE: This is incorrect for Tails, if you want a correct result, 5 should instead be (trackPlayer->defaultYRadius - trackPlayer->rollYRadius)
	
	//Scroll horizontally to our tracked position
	int rDiff = (trackX - x) - (SCREEN_WIDTH / 2);
	int lDiff = ((SCREEN_WIDTH / 2) - 16) - (trackX - x);
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
		vScroll = abs(trackPlayer->yVel) > 0x600 ? 16 : 6;
	}
	
	if (bDiff > 0)
		y += (bDiff > vScroll ? vScroll : bDiff);
	if (tDiff > 0)
		y -= (tDiff > vScroll ? vScroll : tDiff);
	
	//Keep inside of level boundaries
	if (x < gLevelLeftBoundary)
		x = gLevelLeftBoundary;
	if (y < gLevelTopBoundary)
		y = gLevelTopBoundary;
	if (x > gLevelRightBoundary - SCREEN_WIDTH)
		x = gLevelRightBoundary - SCREEN_WIDTH;
	if (y > gLevelBottomBoundary2 - SCREEN_HEIGHT)
		y = gLevelBottomBoundary2 - SCREEN_HEIGHT;
}
