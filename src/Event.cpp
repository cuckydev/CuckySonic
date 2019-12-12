#include "Backend/Event.h"

bool HandleEvents()
{
	//Handle events on the backend
	bool exit = Backend_HandleEvents();
	
	//TODO - Update input
	return exit;
}
