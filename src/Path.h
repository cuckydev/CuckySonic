#pragma once
extern char *gBasePath;
extern char *gPrefPath;

#define GET_GLOBAL_PATH(variable, path)	char variable[0x200]; \
										sprintf(variable, "%s%s", gBasePath, path)
										
#define GET_PREF_PATH(variable, path)	char variable[0x200]; \
										sprintf(variable, "%s%s", gPrefPath, path)

bool InitializePath();
void QuitPath();
