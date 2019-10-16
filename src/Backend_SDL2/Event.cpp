#include "SDL_events.h"
#include "../Input.h"
#include "../Audio.h"
#include "../Log.h"

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

bool HandleEvents()
{
	bool noExit = true;
	bool focusYield = false;
	
	while (SDL_PollEvent(nullptr) || (focusYield && noExit))
	{
		//Wait for next event (instantaneous if focus gained, just polling then stopping when done)
		SDL_Event event;
		SDL_WaitEvent(&event);
		
		switch (event.type)
		{
			case SDL_QUIT:	//Window/game is closed
				noExit = false;
				break;
			
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
					case SDL_WINDOWEVENT_FOCUS_GAINED: //Window focused
						//Unyield game
						focusYield = false;
						YieldAudio(false);
						break;
					case SDL_WINDOWEVENT_FOCUS_LOST: //Window unfocused
						//Yield game until refocused
						focusYield = true;
						YieldAudio(true);
						ClearControllerInput();
						break;
					default:
						break;
				}
				break;
		}
		
		//Check this event for input
		HandleInputEvent(&event);
	}
	
	//Update our input (such as pressed keys, or analogue stick input)
	UpdateInput();
	
	return noExit;
}
