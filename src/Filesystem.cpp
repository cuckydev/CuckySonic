#include <string>
#include "Backend/Filesystem.h"
#include "Filesystem.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"

std::string gBasePath;
std::string gPrefPath;

//Sub-system functions
bool InitializePath()
{
	LOG(("Initializing paths... "));
	if (Backend_GetPaths(&gBasePath, &gPrefPath))
		return Error("Failed to initialize paths");
	LOG(("Success!\n"));
	return false;
}

void QuitPath()
{
	LOG(("Ending paths... "));
	//...
	LOG(("Success!\n"));
}
