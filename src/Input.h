#pragma once
#include <stdint.h>

#define CONTROLLERS 4
#define CONTROLLER_DEADZONE 0x200

#ifdef BACKEND_SDL2
	#include "SDL_gamecontroller.h"
	#include "SDL_events.h"

	struct BUTTONBIND
	{
		SDL_Scancode key;
		SDL_GameControllerButton button;
	};
	
	#define BACKEND_CONTROLLER SDL_GameController
#endif

struct CONTROLMASK
{
	bool start : 1;
	bool a : 1;
	bool b : 1;
	bool c : 1;
	bool right : 1;
	bool left : 1;
	bool down : 1;
	bool up : 1;
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
	
	//Our game controller
	BACKEND_CONTROLLER *controller;
	int16_t axisX;
	int16_t axisY;
};

extern CONTROLLER gController[CONTROLLERS];

bool InitializeInput();
void QuitInput();

void ClearControllerInput();
void UpdateInput();
