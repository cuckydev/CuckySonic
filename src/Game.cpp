#include "Game.h"
#include "Log.h"
#include "Error.h"
#include "GM.h"

#include "SDL_events.h"

//Debug bool
bool gDebugEnabled = false;

//Score, time, rings, and lives
#define SCORE_REWARD 50000
#define RINGS_REWARD 100

#define RINGS_CAP 999
#define LIVES_CAP 99

#define INITIAL_LIVES 3

unsigned int gScore = 0;
unsigned int gNextScoreReward = SCORE_REWARD;

unsigned int gTime = 0;

unsigned int gRings = 0;
unsigned int gNextRingReward = RINGS_REWARD;

unsigned int gLives = INITIAL_LIVES;

//Generic game functions
void AddToScore(unsigned int score)
{
	//Increase score
	gScore += score;
	
	//Check for extra life rewards
	if (gScore >= gNextScoreReward)
	{
		//Increase lives
		while (gScore >= gNextScoreReward)
		{
			gNextScoreReward += SCORE_REWARD;
			AddToLives(1);
		}
		
		//Play jingle
		MUSICSPEC jingleSpec = {"ExtraLifeJingle", 0, 1.0f};
		gLevel->PlayJingle(jingleSpec);
	}
}

void AddToRings(unsigned int rings)
{
	//Update ring reward (if we've lost a bunch of rings then lower it)
	if (gRings == 0)
		gNextRingReward = RINGS_REWARD;
	while (gRings + (RINGS_REWARD * 2) < gNextRingReward)
		gNextRingReward -= RINGS_REWARD;
	
	//Increase ring count and cap
	if (gRings >= RINGS_CAP - rings)
		gRings = RINGS_CAP;
	else
		gRings += rings;
	
	//Check for extra life rewards
	if (gRings >= gNextRingReward)
	{
		//Increase lives
		while (gRings >= gNextRingReward)
		{
			AddToLives(1);
			gNextRingReward += RINGS_REWARD;
		}
		
		//Play jingle
		MUSICSPEC jingleSpec = {"ExtraLifeJingle", 0, 1.0f};
		gLevel->PlayJingle(jingleSpec);
	}
	else
	{
		//Play ring sound
		PlaySound(SOUNDID_RING);
	}
}

void AddToLives(unsigned int lives)
{
	//Increase lives and cap
	if (gLives >= LIVES_CAP - lives)
		gLives = LIVES_CAP;
	else
		gLives += lives;
}

void InitializeScores()
{
	gTime = 0;
	gRings = 0;
	gNextRingReward = RINGS_REWARD;
}

//Game loop
GAMEMODE gGameMode;

bool EnterGameLoop()
{
	//Initialize game memory
	gGameMode = GAMEMODE_SPLASH; //Start at splash screen
	
	gScore = 0;
	gNextScoreReward = SCORE_REWARD;
	gTime = 0;
	gRings = 0;
	gNextRingReward = RINGS_REWARD;
	gLives = INITIAL_LIVES;
	
	//Run game code
	bool noExit = true;
	bool noError = true;
	
	while (noExit)
	{
		switch (gGameMode)
		{
			case GAMEMODE_SPLASH:
				noExit = GM_Splash(&noError);
				break;
			case GAMEMODE_TITLE:
				noExit = GM_Title(&noError);
				break;
			case GAMEMODE_DEMO:
			case GAMEMODE_GAME:
				noExit = GM_Game(&noError);
				break;
			default:
				noExit = false;
				break;
		}
	}
	
	//End game loop
	return noError;
}
