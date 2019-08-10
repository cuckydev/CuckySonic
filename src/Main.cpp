#include "SDL.h"

#include "Error.h"
#include "Log.h"
#include "Path.h"
#include "Render.h"
#include "Input.h"
#include "Game.h"

int main(int argc, char *argv[])
{
	//Initialize SDL2
	LOG(("Initializing SDL2... "));
	
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		Error(SDL_GetError());
		return -1; //Nothing to clean up, we can just return our failure
	}
	
	LOG(("Success!\n"));
	
	//Initialize game sub-systems
	bool noError = true;
	
	if ((noError = (InitializePath() && InitializeRender() && InitializeInput())))
		noError = EnterGameLoop();
	
	//End game sub-systems
	QuitInput();
	QuitRender();
	QuitPath();
	
	//Quit SDL2
	LOG(("Ending SDL2... "));
	
	SDL_Quit();
	
	LOG(("Success!\n"));
	
	//Failed exit
	if (!noError)
		return -1;
	
	//Successful exit
	return 0;
}
