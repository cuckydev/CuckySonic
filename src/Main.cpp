#include "SDL.h"

#include "Render.h"
#include "Error.h"
#include "Log.h"

int main(int argc, char *argv[])
{
	//Initialize SDL2
	LOG("Initializing SDL2\n");
	
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		Error(SDL_GetError());
		return -1; //Nothing to clean up, we can just return our failure
	}
	
	LOG("Success\n");
	
	//Initialize game sub-systems
	bool noError = true;
	
	if ((noError = InitializeRender()))
	{
		//TODO: Run game
		
	}
	
	//End game sub-systems
	QuitRender();
	
	//Quit SDL2
	LOG("Ending SDL2\n");
	
	SDL_Quit();
	
	LOG("Success\n");
	
	//Failed exit
	if (!noError)
		return -1;
	
	//Successful exit
	return 0;
}
