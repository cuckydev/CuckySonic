#include "Input.h"
#include "Log.h"

CONTROLLER gController[CONTROLLERS];

//Default masks
BUTTONBINDS defaultBinds[CONTROLLERS] = {																										//nintendo controllers?
	{{SDL_SCANCODE_RETURN, SDL_CONTROLLER_BUTTON_START}, {SDL_SCANCODE_A, SDL_CONTROLLER_BUTTON_A}, {SDL_SCANCODE_S, SDL_CONTROLLER_BUTTON_B}, {SDL_SCANCODE_D, SDL_CONTROLLER_BUTTON_X}, {SDL_SCANCODE_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {SDL_SCANCODE_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {SDL_SCANCODE_DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {SDL_SCANCODE_UP, SDL_CONTROLLER_BUTTON_DPAD_UP}},
	{{(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_START}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_A}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_B}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_X}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP}},
	{{(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_START}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_A}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_B}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_X}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP}},
	{{(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_START}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_A}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_B}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_X}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_RIGHT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_LEFT}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_DOWN}, {(SDL_Scancode)0, SDL_CONTROLLER_BUTTON_DPAD_UP}},
};

//Subsystem code
bool InitializeInput()
{
	LOG(("Initializing input... "));
	
	//Initialize controllers
	memset(gController, 0, sizeof(gController));
	
	//TODO: attempt to load input bindings save
	
	//Use default input bindings
	for (int i = 0; i < CONTROLLERS; i++)
		gController[i].binds = defaultBinds[i];
	
	//TODO: Connect controllers
	
	LOG(("Success!\n"));
	return true;
}

void QuitInput()
{
	LOG(("Ending input... "));
	
	//TODO: Disconnect controllers
	
	LOG(("Success!\n"));
}

//Function to clear all controller's input
void ClearControllerInput()
{
	for (int i = 0; i < CONTROLLERS; i++)
	{
		memset(&gController[i].held, 0, sizeof(CONTROLLER::held));
		memset(&gController[i].lastHeld, 0, sizeof(CONTROLLER::lastHeld));
		memset(&gController[i].press, 0, sizeof(CONTROLLER::press));
	}
}

//Handle event
#define DO_BUTTON_EVENT(button, bind, set) if (event->key.keysym.scancode == bind) { button = set; continue; }

void HandleInputEvent(SDL_Event *event)
{
	switch (event->type)
	{
		case SDL_KEYDOWN: //Keyboard input events
		case SDL_KEYUP:
			for (int i = 0; i < CONTROLLERS; i++)
			{
				DO_BUTTON_EVENT(gController[i].nextHeld.start,	gController[i].binds.start.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(gController[i].nextHeld.a,		gController[i].binds.a.key,		(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(gController[i].nextHeld.b,		gController[i].binds.b.key,		(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(gController[i].nextHeld.c,		gController[i].binds.c.key,		(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(gController[i].nextHeld.right,	gController[i].binds.right.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(gController[i].nextHeld.left,	gController[i].binds.left.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(gController[i].nextHeld.down,	gController[i].binds.down.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(gController[i].nextHeld.up,		gController[i].binds.up.key,	(event->type == SDL_KEYDOWN));
			}
			break;
	}
}

//Update our pressed keys, and last held
#define DO_PRESS_CHECK(press, held, last) press = (held ? !last : false);

void UpdateInput()
{
	for (int i = 0; i < CONTROLLERS; i++)
	{
		//Update our held
		gController[i].held = gController[i].nextHeld;
		
		//Set pressed keys
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
