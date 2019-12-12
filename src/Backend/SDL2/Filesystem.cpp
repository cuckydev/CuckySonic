#include <string>
#include "SDL_filesystem.h"
#include "../../GameConstants.h"

bool Backend_GetPaths(std::string *basePath, std::string *prefPath)
{
	//Allow SDL to get our paths
	char *gotBasePath = SDL_GetBasePath();
	char *gotPrefPath = SDL_GetPrefPath(GAME_ORGANIZATION, GAME_TITLE);
	
	//Apply these paths to our output paths
	if (gotBasePath != nullptr && basePath != nullptr)
		*basePath = std::string(gotBasePath);
	else if (basePath != nullptr)
		*basePath = "./";
	
	if (gotPrefPath != nullptr && prefPath != nullptr)
		*prefPath = std::string(gotPrefPath);
	else if (prefPath != nullptr && basePath != nullptr)
		*prefPath = *basePath;
	return false;
}
