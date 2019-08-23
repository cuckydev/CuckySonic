#pragma once
#include "SDL_gamecontroller.h"
#include "SDL_events.h"

#define CONTROLLERS 4

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

struct BUTTONBIND
{
	SDL_Scancode key;
	SDL_GameControllerButton button;
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
	SDL_GameController *controller;
};

extern CONTROLLER gController[CONTROLLERS];

bool InitializeInput();
void QuitInput();

void ClearControllerInput();
void HandleInputEvent(SDL_Event *event);
void UpdateInput();
