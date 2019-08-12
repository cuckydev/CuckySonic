#include "Camera.h"
#include "Level.h"
#include "GameConstants.h"

CAMERA::CAMERA(PLAYER *trackPlayer)
{
	Track(trackPlayer);
	return;
}

CAMERA::~CAMERA()
{
	return;
}

void CAMERA::Track(PLAYER *trackPlayer)
{
	//Get our track positions
	int trackX = trackPlayer->x.pos, trackY = trackPlayer->y.pos;
	
	if (trackPlayer->scrollDelay)
	{
		trackPlayer->scrollDelay -= 0x100;
		trackX = trackPlayer->posRecord[(trackPlayer->recordPos - ((trackPlayer->scrollDelay / 0x100) + 1)) % PLAYER_RECORD_LENGTH].x;
	}
	
	if (trackPlayer->status.inBall)
		trackY -= 5; //NOTE: This is incorrect for Tails, if you want a correct result, 5 should instead be (trackPlayer->defaultYRadius - trackPlayer->rollYRadius)
	
	//Move to our track position (temporary)
	x = trackX - SCREEN_WIDTH / 2;
	y = trackY - SCREEN_HEIGHT / 2;
	
	//Keep inside of level boundaries
	if (x < gLevelLeftBoundary)
		x = gLevelLeftBoundary;
	if (y < gLevelTopBoundary)
		y = gLevelTopBoundary;
	if (x > gLevelRightBoundary - SCREEN_WIDTH)
		x = gLevelRightBoundary - SCREEN_WIDTH;
	if (y > gLevelBottomBoundary - SCREEN_HEIGHT)
		y = gLevelBottomBoundary - SCREEN_HEIGHT;
}
