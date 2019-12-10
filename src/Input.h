#pragma once
#include <stdint.h>

//Constants
#define CONTROLLERS	4

//TEMP
#define BUTTONBIND int

struct CONTROLMASK
{
	bool start = false;
	bool a = false;
	bool b = false;
	bool c = false;
	bool right = false;
	bool left = false;
	bool down = false;
	bool up = false;
};

struct BUTTONBINDS
{
	BUTTONBIND start;
	BUTTONBIND a;
	BUTTONBIND b;
	BUTTONBIND c;
	BUTTONBIND right;
	BUTTONBIND left;
	BUTTONBIND down;
	BUTTONBIND up;
};

struct CONTROLLER
{
	//Current control masks
	CONTROLMASK held;
	CONTROLMASK nextHeld;
	
	CONTROLMASK lastHeld;
	CONTROLMASK press;
	
	//Button binds
	BUTTONBINDS binds;
};

extern CONTROLLER gController[CONTROLLERS];

bool InitializeInput();
void QuitInput();

void ClearControllerInput();
void UpdateInput();
