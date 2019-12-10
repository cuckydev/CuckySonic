#include "Log.h"
#include "Filesystem.h"
#include "Render.h"
#include "Audio.h"
#include "Input.h"
#include "Error.h"
#include "Game.h"

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	
	//Initialize game sub-systems, then enter game loop
	bool error = false;
	if ((error = (InitializePath() || InitializeRender() || InitializeAudio() || InitializeInput())) == false)
		error = EnterGameLoop();
	
	//End game sub-systems
	QuitInput();
	QuitAudio();
	QuitRender();
	QuitPath();
	
	//Exit program
	if (error)
		return -1;
	return 0;
}
