#pragma once
#include <stdint.h>

//Backend specific file handles
#ifdef BACKEND_SDL2
	#include "SDL_rwops.h"
	#include "SDL_filesystem.h"
	#define BACKEND_FILE	SDL_RWops
#endif

//File functions
BACKEND_FILE *OpenFile(const char *path, const char *mode);
void CloseFile(BACKEND_FILE *file);
const char *GetFileError();
size_t GetFileSize(BACKEND_FILE *file);

size_t		ReadFile(BACKEND_FILE *file, void *ptr, size_t size, size_t maxnum);
uint8_t		ReadFile_Byte(BACKEND_FILE *file);
uint16_t	ReadFile_LE16(BACKEND_FILE *file);
uint16_t	ReadFile_BE16(BACKEND_FILE *file);
uint32_t	ReadFile_LE32(BACKEND_FILE *file);
uint32_t	ReadFile_BE32(BACKEND_FILE *file);
uint64_t	ReadFile_LE64(BACKEND_FILE *file);
uint64_t	ReadFile_BE64(BACKEND_FILE *file);

size_t WriteFile(BACKEND_FILE *file, void *ptr, size_t size, size_t num);
size_t WriteFile_Byte(BACKEND_FILE *file, uint8_t  value);
size_t WriteFile_LE16(BACKEND_FILE *file, uint16_t value);
size_t WriteFile_BE16(BACKEND_FILE *file, uint16_t value);
size_t WriteFile_LE32(BACKEND_FILE *file, uint32_t value);
size_t WriteFile_BE32(BACKEND_FILE *file, uint32_t value);
size_t WriteFile_LE64(BACKEND_FILE *file, uint64_t value);
size_t WriteFile_BE64(BACKEND_FILE *file, uint64_t value);

enum FILESEEK
{
	FILESEEK_SET,
	FILESEEK_CUR,
	FILESEEK_END,
};

size_t SeekFile(BACKEND_FILE *file, size_t whence, int offset);
size_t TellFile(BACKEND_FILE *file);

//Path sub-system
extern char *gBasePath;
extern char *gPrefPath;

char *AllocPath(const char *base, const char *name, const char *append);
bool InitializePath();
void QuitPath();
