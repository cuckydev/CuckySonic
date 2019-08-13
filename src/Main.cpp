#include "SDL.h"
#include "Error.h"
#include "Log.h"
#include "Path.h"
#include "Render.h"
#include "Audio.h"
#include "Input.h"
#include "Game.h"

int main(int argc, char *argv[])
{
	//Print our given arguments (just for debug stuff)
	LOG(("arguments %d\n", argc));
	for (int i = 0; i < argc; i++)
		LOG(("    %s\n", argv[i]));
	
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
	
	if ((noError = (InitializePath() && InitializeRender() && InitializeAudio() && InitializeInput())))
		noError = EnterGameLoop();
	
	//End game sub-systems
	QuitInput();
	QuitAudio();
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
