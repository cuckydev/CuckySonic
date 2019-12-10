#include <string>
#include "Filesystem.h"
#include "GameConstants.h"
#include "Log.h"
#include "Error.h"

std::string gBasePath = "./";
std::string gPrefPath = "./";

//Sub-system functions
bool InitializePath()
{
	LOG(("Initializing paths... "));
	LOG(("Success!\n"));
	return false;
}

void QuitPath()
{
	LOG(("Ending paths... "));
	LOG(("Success!\n"));
}
