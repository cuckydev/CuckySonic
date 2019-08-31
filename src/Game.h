#pragma once
#include "Level.h"

enum GAMEMODE
{
	GAMEMODE_SPLASH,		//"Sega" screen, but can be used for general splash
	GAMEMODE_TITLE,			//Title screen
	GAMEMODE_DEMO,			//Identical to game, but uses demo inputs
	GAMEMODE_GAME,			//Gameplay
	GAMEMODE_SPECIALSTAGE,	//Special stage
	GAMEMODE_CONTINUE,		//Continue screen
	GAMEMODE_ENDING,		//Ending sequence
	GAMEMODE_CREDITS,		//Credits sequence
};

//Debug enabled boolean
extern bool gDebugEnabled;

//Score, time, rings, and lives
extern unsigned int gScore;
extern unsigned int gTime;
extern unsigned int gRings;
extern unsigned int gLives;

//Gamemode and level state
extern GAMEMODE gGameMode;
extern LEVEL *gLevel;

extern int gGameLoadLevel;
extern int gGameLoadCharacter;

//Generic game functions
void AddToScore(unsigned int score);
void CollectRing();

//Game loop function
bool EnterGameLoop();
