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
	
	//Our game controller
	BACKEND_CONTROLLER *controller;
	int16_t axisX = 0;
	int16_t axisY = 0;
};

extern CONTROLLER gController[CONTROLLERS];

bool InitializeInput();
void QuitInput();

void ClearControllerInput();
void UpdateInput();
