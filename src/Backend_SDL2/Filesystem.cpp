#include <stdlib.h>
#include <string.h>

#include "../Filesystem.h"
#include "../GameConstants.h"
#include "../Log.h"
#include "../Error.h"

char *gBasePath;
char *gPrefPath;

//File handle functions
BACKEND_FILE *OpenFile(const char *path, const char *mode) { return SDL_RWFromFile(path, mode); }
void CloseFile(BACKEND_FILE *file) { SDL_RWclose(file); }
const char *GetFileError() { return SDL_GetError(); }
size_t GetFileSize(BACKEND_FILE *file) { return SDL_RWsize(file); }

size_t		ReadFile(BACKEND_FILE *file, void *ptr, size_t size, size_t maxnum) { return SDL_RWread(file, ptr, size, maxnum); }
uint8_t		ReadFile_Byte(BACKEND_FILE *file) { return SDL_ReadU8(file); }
uint16_t	ReadFile_LE16(BACKEND_FILE *file) { return SDL_ReadLE16(file); }
uint16_t	ReadFile_BE16(BACKEND_FILE *file) { return SDL_ReadBE16(file); }
uint32_t	ReadFile_LE32(BACKEND_FILE *file) { return SDL_ReadLE32(file); }
uint32_t	ReadFile_BE32(BACKEND_FILE *file) { return SDL_ReadBE32(file); }
uint64_t	ReadFile_LE64(BACKEND_FILE *file) { return SDL_ReadLE64(file); }
uint64_t	ReadFile_BE64(BACKEND_FILE *file) { return SDL_ReadBE64(file); }

size_t		WriteFile(BACKEND_FILE *file, void *ptr, size_t size, size_t num) { return SDL_RWwrite(file, ptr, size, num); }
size_t		WriteFile_Byte(BACKEND_FILE *file, uint8_t  value)	{ return SDL_WriteU8(file, value); }
size_t		WriteFile_LE16(BACKEND_FILE *file, uint16_t  value)	{ return SDL_WriteLE16(file, value); }
size_t		WriteFile_BE16(BACKEND_FILE *file, uint16_t  value)	{ return SDL_WriteBE16(file, value); }
size_t		WriteFile_LE32(BACKEND_FILE *file, uint32_t  value)	{ return SDL_WriteLE32(file, value); }
size_t		WriteFile_BE32(BACKEND_FILE *file, uint32_t  value)	{ return SDL_WriteBE32(file, value); }
size_t		WriteFile_LE64(BACKEND_FILE *file, uint64_t  value)	{ return SDL_WriteLE64(file, value); }
size_t		WriteFile_BE64(BACKEND_FILE *file, uint64_t  value)	{ return SDL_WriteBE64(file, value); }

size_t SeekFile(BACKEND_FILE *file, size_t whence, int offset)
{
	switch (offset)
	{
		case FILESEEK_SET:
			return SDL_RWseek(file, whence, RW_SEEK_SET);
		case FILESEEK_CUR:
			return SDL_RWseek(file, whence, RW_SEEK_CUR);
		case FILESEEK_END:
			return SDL_RWseek(file, whence, RW_SEEK_END);
	}
	
	return -1;
}

size_t TellFile(BACKEND_FILE *file) { return SDL_RWtell(file); }

//Core file path stuff
char *DupePath(const char *path)
{
	size_t length = strlen(path) + 1;
	char *retVal = new char[length];
	return (char*)memcpy(retVal, path, length);
}

char *AllocPath(const char *base, const char *name, const char *append)
{
	char *newString = new char[
								strlen(base) //base (if not null)
								+ (name == nullptr ? 0 : strlen(name)) //name (if not null)
								+ (append == nullptr ? 0 : strlen(append)) //append (if not null)
								+ 1
							];
	strcpy(newString, base);
	if (name != nullptr)
		strcat(newString, name);
	if (append != nullptr)
		strcat(newString, append);
	return newString;
}

bool InitializePath()
{
	LOG(("Initializing paths... "));
	char *basePath = SDL_GetBasePath();
	char *prefPath = SDL_GetPrefPath(GAME_ORGANIZATION, GAME_TITLE);
	
	//Get base (executable's) path
	if (basePath == nullptr)
	{
		//Use "./" as a placeholder
		gBasePath = AllocPath("./", nullptr, nullptr);
		Warn("Couldn't get a path to the executable from SDL, may cause undefined behaviour");
	}
	else
	{
		//Copy basePath
		gBasePath = AllocPath(basePath, nullptr, nullptr);
		SDL_free(basePath);
	}
	
	//Get pref (save data) path
	if (prefPath == nullptr)
	{
		//Copy gBasePath as a placeholder
		gPrefPath = AllocPath(gBasePath, nullptr, nullptr);
		Warn("Couldn't get a path to pref directory from SDL, executable's path will be used instead");
	}
	else
	{
		//Copy prefPath
		gPrefPath = AllocPath(prefPath, nullptr, nullptr);
		SDL_free(prefPath);
	}
	
	LOG(("Success!\n"));
	return true;
}

void QuitPath()
{
	LOG(("Ending paths... "));
	delete[] gBasePath;
	delete[] gPrefPath;
	LOG(("Success!\n"));
}
