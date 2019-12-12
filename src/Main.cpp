#include "Log.h"
#include "Filesystem.h"
#include "Render.h"
#include "Audio.h"
#include "Input.h"
#include "Error.h"
#include "Game.h"

//Include backend core
#include "Backend/Core.h"
#ifdef BACKEND_SDL2
	#include "SDL.h" //Must be included with the entry point, as SDL hooks into the entry point
#endif

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	
	//Initialize game sub-systems and backend core, then enter game loop
	bool error = false;
	if ((error = (Backend_InitCore() || InitializePath() || InitializeRender() || InitializeAudio() || InitializeInput())) == false)
		error = EnterGameLoop();
	
	//End game sub-systems and backend core
	QuitInput();
	QuitAudio();
	QuitRender();
	QuitPath();
	Backend_QuitCore();
	
	//Exit program
	if (error)
		return -1;
	return 0;
}
