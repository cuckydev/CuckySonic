#include "Log.h"
#include "Filesystem.h"
#include "Render.h"
#include "Audio.h"
#include "Input.h"
#include "Error.h"
#include "Game.h"

//Include backend cores
#include "Backend/Core.h"
#ifdef BACKEND_SDL2
	#include "SDL.h" //Must be included with the entry point, as SDL hooks into the entry point
#endif

//Misc. includes
#ifdef SWITCH
	#include <switch.h>
#endif

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	
	#ifdef ENABLE_NXLINK
		//Enable NXLink for Switch debugging
		socketInitializeDefault();
		nxlinkStdio();
	#endif
	
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
	
	#ifdef ENABLE_NXLINK
		//End NXLink
		socketExit();
	#endif
	
	//Exit program
	if (error)
		return -1;
	return 0;
}
