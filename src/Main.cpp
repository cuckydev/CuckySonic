#include "Filesystem.h"
#include "Render.h"
#include "Audio.h"
#include "Input.h"
#include "Game.h"
#include "Error.h"
#include "Log.h"

//Backend usage
#ifdef BACKEND_SDL2
	#include "SDL.h"
	
	#define BACKEND_INIT	SDL_Init(SDL_INIT_EVERYTHING) < 0
	#define BACKEND_ERROR	SDL_GetError()
	#define BACKEND_QUIT	SDL_Quit()
#endif

int main(int argc, char *argv[])
{
	//Print our given arguments (just for debug stuff)
	LOG(("arguments %d\n", argc));
	for (int i = 0; i < argc; i++)
		LOG(("    %s\n", argv[i]));
	
	//Initialize our backend
	LOG(("Initializing backend... "));
	
	if (BACKEND_INIT)
	{
		Error(BACKEND_ERROR);
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
	
	//Quit backend
	LOG(("Ending backend... "));
	BACKEND_QUIT;
	LOG(("Success!\n"));
	
	//Failed exit
	if (!noError)
		return -1;
	
	//Successful exit
	return 0;
}
