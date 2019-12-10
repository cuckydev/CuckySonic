#include <string.h>
#include "Input.h"
#include "Filesystem.h"
#include "Log.h"
#include "Error.h"

CONTROLLER gController[CONTROLLERS];

//Default masks
/*
BUTTONBINDS defaultBinds[CONTROLLERS] = {																										//nintendo controllers?
	{{SDL_SCANCODE_RETURN, SDL_CONTROLLER_BUTTON_START}, {SDL_SCANCODE_A, SDL_CONTROLLER_BUTTON_A}, {SDL_SCANCODE_S, SDL_CONTROLLER_BUTTON_B}, {SDL_SCANCODE_D, SDL_CONTROLLER_BUTTON_X}, {SDL_SCANCODE_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {SDL_SCANCODE_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {SDL_SCANCODE_DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {SDL_SCANCODE_UP, SDL_CONTROLLER_BUTTON_DPAD_UP}},
	{{(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_START}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_A}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_B}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_X}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP}},
	{{(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_START}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_A}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_B}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_X}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP}},
	{{(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_START}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_A}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_B}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_X}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP}},
};
*/

//Subsystem code
bool InitializeInput()
{
	LOG(("Initializing input...\n"));
	LOG(("Success!\n"));
	return false;
}

void QuitInput()
{
	LOG(("Ending input... "));
	LOG(("Success!\n"));
}

//Function to clear all controller's input
void ClearControllerInput()
{
	for (int i = 0; i < CONTROLLERS; i++)
	{
		gController[i].held = {};
		gController[i].lastHeld = {};
		gController[i].press = {};
	}
}

//Update our pressed buttons, and last held
#define DO_PRESS_CHECK(press, held, last) press = (held ? !last : false);

void UpdateInput()
{
	for (int i = 0; i < CONTROLLERS; i++)
	{
		//Update our held
		gController[i].held = gController[i].nextHeld;
		
		//Set pressed buttons
		DO_PRESS_CHECK(gController[i].press.start,	gController[i].held.start,	gController[i].lastHeld.start);
		DO_PRESS_CHECK(gController[i].press.a,		gController[i].held.a,		gController[i].lastHeld.a);
		DO_PRESS_CHECK(gController[i].press.b,		gController[i].held.b,		gController[i].lastHeld.b);
		DO_PRESS_CHECK(gController[i].press.c,		gController[i].held.c,		gController[i].lastHeld.c);
		DO_PRESS_CHECK(gController[i].press.right,	gController[i].held.right,	gController[i].lastHeld.right);
		DO_PRESS_CHECK(gController[i].press.left,	gController[i].held.left,	gController[i].lastHeld.left);
		DO_PRESS_CHECK(gController[i].press.down,	gController[i].held.down,	gController[i].lastHeld.down);
		DO_PRESS_CHECK(gController[i].press.up,		gController[i].held.up,		gController[i].lastHeld.up);
		
		//Copy our last held for next update
		gController[i].lastHeld = gController[i].held;
	}
}
