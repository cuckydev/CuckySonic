#include "Input.h"
#include "Path.h"
#include "Log.h"
#include "Error.h"
#include "MathUtil.h"

#define CONTROLLER_DEADZONE 0x200

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
	LOG(("Initializing input...\n"));
	
	//Initialize controllers
	memset(gController, 0, sizeof(gController));
	
	//TODO: attempt to load input bindings save
	
	//Use default input bindings
	for (int i = 0; i < CONTROLLERS; i++)
		gController[i].binds = defaultBinds[i];
	
	//Open gamecontrollerdb.txt (Gamepad mappings)
	GET_GLOBAL_PATH(gcdbPath, "data/gamecontrollerdb.txt");
	
	if (SDL_GameControllerAddMappingsFromFile(gcdbPath) < 0)
	{
		Error(SDL_GetError());
		return false;
	}
	else
	{
		LOG(("Successfully loaded gamepad mappings from gamecontrollerdb.txt\n"));
	}
	
	//Connect controllers
	for (int i = 0, v = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			//Open this controller and assign to a virtual controller
			SDL_GameController *controller = SDL_GameControllerOpen(i);
			
			if (controller != nullptr)
			{
				if (v >= CONTROLLERS)
				{
					//Surpassed virtual controller count
					LOG(("Failed to connect controller %s to controller %d\nReason: Ran out of virtual controller slots\n", SDL_GameControllerName(controller), v));
					SDL_GameControllerClose(controller);
					break;
				}
				else
				{
					//Successfully connected controller
					LOG(("Connected controller %s to controller %d\n", SDL_GameControllerName(controller), v));
					gController[v++].controller = controller;
				}
			}
			else
			{
				LOG(("Failed to connect controller %s\nReason: %s\n", SDL_GameControllerName(controller), SDL_GetError()));
				SDL_GameControllerClose(controller);
			}
		}
	}
	
	LOG(("Success!\n"));
	return true;
}

void QuitInput()
{
	LOG(("Ending input... "));
	
	//Disconnect controllers
	for (int i = 0; i < CONTROLLERS; i++)
	{
		if (gController[i].controller != nullptr)
			SDL_GameControllerClose(gController[i].controller);
	}
	
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
#define DO_BUTTON_EVENT(src, button, bind, set) if (src == bind) { button = set; continue; }

void HandleInputEvent(SDL_Event *event)
{
	switch (event->type)
	{
		case SDL_KEYDOWN: //Keyboard input events
		case SDL_KEYUP:
			//Check which key and virtual controller this maps to, and set it according to if it's a press or release
			for (int i = 0; i < CONTROLLERS; i++)
			{
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.start,	gController[i].binds.start.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.a,		gController[i].binds.a.key,		(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.b,		gController[i].binds.b.key,		(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.c,		gController[i].binds.c.key,		(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.right,	gController[i].binds.right.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.left,	gController[i].binds.left.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.down,	gController[i].binds.down.key,	(event->type == SDL_KEYDOWN));
				DO_BUTTON_EVENT(event->key.keysym.scancode,	gController[i].nextHeld.up,		gController[i].binds.up.key,	(event->type == SDL_KEYDOWN));
			}
			break;
		case SDL_CONTROLLERDEVICEADDED: //Controller (not joystick) connected
		{
			//Open the connected controller
			SDL_GameController *connectController = SDL_GameControllerOpen(event->cdevice.which);
			if (connectController == nullptr)
			{
				LOG(("Failed to connect controller %s\nReason: %s\n", SDL_GameControllerName(connectController), SDL_GetError()));
				break;
			}
			
			//Attach to the first vacant virtual controller
			for (int i = 0; ; i++)
			{
				if (i >= CONTROLLERS)
				{
					//If there's no more virtual controllers, stop trying to connect
					LOG(("Failed to connect controller %s\nReason: Ran out of virtual controller slots\n", SDL_GameControllerName(connectController)));
					SDL_GameControllerClose(connectController);
					break;
				}
				else
				{
					//Quit if this controller is already connected
					if (gController[i].controller == connectController)
						break;
					
					//Check if this virtual controller is vacant, and attach if so (then stop checking)
					if (gController[i].controller == nullptr)
					{
						gController[i].controller = connectController;
						LOG(("Connected controller %s to controller %d\n", SDL_GameControllerName(connectController), i));
						break;
					}
				}
			}
			break;
		}
		case SDL_CONTROLLERDEVICEREMOVED: //Controller (not joystick) disconnecting
		{
			//Identify the disconnected controller
			SDL_GameController *disconnectController = SDL_GameControllerFromInstanceID(event->cdevice.which);
			if (disconnectController == nullptr)
			{
				LOG(("Failed to identify disconnected controller\n"));
				break;
			}
			
			//Check all virtual controllers to see if this is connected to any, then disconnect them
			for (int i = 0; i < CONTROLLERS; i++)
			{
				if (gController[i].controller == disconnectController)
				{
					//This controller matches the given controller, disconnect
					LOG(("Disconnecting controller %s from controller %d\n", SDL_GameControllerName(gController[i].controller), i));
					SDL_GameControllerClose(gController[i].controller);
					gController[i].controller = nullptr;
					
					//Clear previous input
					gController[i].axisX = 0;
					gController[i].axisY = 0;
					memset(&gController[i].held, 0, sizeof(CONTROLLER::held));
					memset(&gController[i].lastHeld, 0, sizeof(CONTROLLER::lastHeld));
					memset(&gController[i].press, 0, sizeof(CONTROLLER::press));
					break;
				}
			}
			break;
		}
		case SDL_CONTROLLERBUTTONDOWN: //Controller button press / release
		case SDL_CONTROLLERBUTTONUP:
		{
			//Identify controller
			SDL_GameController *buttonController = SDL_GameControllerFromInstanceID(event->cbutton.which);
			if (buttonController == nullptr)
			{
				LOG(("Failed to identify controller\n"));
				break;
			}
			
			//Check which button and virtual controller this maps to, and set it according to if it's a press or release
			for (int i = 0; i < CONTROLLERS; i++)
			{
				if (gController[i].controller == buttonController)
				{
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.start,	gController[i].binds.start.button,	(event->type == SDL_CONTROLLERBUTTONDOWN));
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.a,		gController[i].binds.a.button,		(event->type == SDL_CONTROLLERBUTTONDOWN));
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.b,		gController[i].binds.b.button,		(event->type == SDL_CONTROLLERBUTTONDOWN));
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.c,		gController[i].binds.c.button,		(event->type == SDL_CONTROLLERBUTTONDOWN));
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.right,	gController[i].binds.right.button,	(event->type == SDL_CONTROLLERBUTTONDOWN));
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.left,	gController[i].binds.left.button,	(event->type == SDL_CONTROLLERBUTTONDOWN));
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.down,	gController[i].binds.down.button,	(event->type == SDL_CONTROLLERBUTTONDOWN));
					DO_BUTTON_EVENT(event->cbutton.button,	gController[i].nextHeld.up,		gController[i].binds.up.button,		(event->type == SDL_CONTROLLERBUTTONDOWN));
				}
			}
			break;
		}
		case SDL_CONTROLLERAXISMOTION: //Controller axis (analogue stick and some d-pads)
		{
			//Identify controller
			SDL_GameController *axisController = SDL_GameControllerFromInstanceID(event->caxis.which);
			if (axisController == nullptr)
			{
				LOG(("Failed to identify controller\n"));
				break;
			}
			
			//Check which virtual controller this maps to and set the axis position + directions from it
			for (int i = 0; i < CONTROLLERS; i++)
			{
				if (gController[i].controller == axisController)
				{
					if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
					{
						//Check how the new axis value will affect our held directions
						if (abs(gController[i].axisX) >= CONTROLLER_DEADZONE && abs(event->caxis.value) < CONTROLLER_DEADZONE)
						{
							//Moving the stick back into the dead-zone
							gController[i].nextHeld.left = false;
							gController[i].nextHeld.right = false;
						}
						else
						{
							//Moving the stick out of the dead-zone or switching sides
							if (event->caxis.value > 0)	//+ = Right
							{
								gController[i].nextHeld.right = true;
								gController[i].nextHeld.left = false;
							}
							if (event->caxis.value < 0)	//- = Left
							{
								gController[i].nextHeld.left = true;
								gController[i].nextHeld.right = false;
							}
						}
						
						//Remember our axis value for next update
						gController[i].axisX = event->caxis.value;
					}
					else if (event->caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
					{
						//Check how the new axis value will affect our held directions
						if (abs(gController[i].axisY) >= CONTROLLER_DEADZONE && abs(event->caxis.value) < CONTROLLER_DEADZONE)
						{
							//Moving the stick back into the dead-zone
							gController[i].nextHeld.up = false;
							gController[i].nextHeld.down = false;
						}
						else
						{
							//Moving the stick out of the dead-zone or switching sides
							if (event->caxis.value > 0)	//+ = Down
							{
								gController[i].nextHeld.down = true;
								gController[i].nextHeld.up = false;
							}
							if (event->caxis.value < 0)	//- = Up
							{
								gController[i].nextHeld.up = true;
								gController[i].nextHeld.down = false;
							}
						}
						
						//Remember our axis value for next update
						gController[i].axisY = event->caxis.value;
					}
				}
			}
			break;
		}
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
