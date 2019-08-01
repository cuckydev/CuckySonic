#include <string.h>

#include "SDL_filesystem.h"

#include "GameConstants.h"
#include "Log.h"
#include "Error.h"

char *gBasePath;
char *gPrefPath;

char *AllocPath(const char *string, const char *append)
{
	char *newString = (char*)malloc(strlen(string) + (append == NULL ? 1 : strlen(append)) + 1);
	strcpy(newString, string);
	if (append)
		strcat(newString, append);
	return newString;
}

bool InitializePath()
{
	LOG(("Initializing path sub-system... "));
	
	gBasePath = SDL_GetBasePath();
	gPrefPath = SDL_GetPrefPath(GAME_ORGANIZATION, GAME_TITLE);
	
	//Get base (executable's) path
	if (gBasePath == NULL)
	{
		gBasePath = AllocPath("./", NULL);
		Warn("Couldn't get a path to the executable from SDL, may cause undefined behaviour");
	}
	
	//Get pref (save data) path
	if (gPrefPath == NULL)
	{
		gPrefPath = gBasePath;
		Warn("Couldn't get a path to pref directory from SDL, executable's path will be used instead");
	}
	
	LOG(("Success!\n"));
	
	return true;
}

void QuitPath()
{
	LOG(("Ending path sub-system... "));
	
	SDL_free(gBasePath);
	SDL_free(gPrefPath);
	
	LOG(("Success!\n"));
}
