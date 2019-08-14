#pragma once
extern char *gBasePath;
extern char *gPrefPath;

#define GET_GLOBAL_PATH(variable, path)			char variable[0x400]; \
												sprintf(variable, "%s%s", gBasePath, path)

#define GET_PREF_PATH(variable, path)			char variable[0x400]; \
												sprintf(variable, "%s%s", gPrefPath, path)
										
#define GET_APPEND_PATH(variable, onto, append)	char variable[0x400]; \
												sprintf(variable, "%s%s", onto, append);

bool InitializePath();
void QuitPath();
