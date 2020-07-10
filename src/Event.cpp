#include "Backend/Event.h"
#include "Input.h"

bool HandleEvents()
{
	//Handle events on the backend
	bool exit = Backend_HandleEvents();
	UpdateInput();
	return exit;
}
