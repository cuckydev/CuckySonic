#include <stdio.h>

bool Error(const char *error)
{
	//Print to the console
	printf("Error: %s\n", error);
	return true;
}

bool Warn(const char *warning)
{
	//Print to the console
	printf("Warning: %s\n", warning);
	return true;
}
