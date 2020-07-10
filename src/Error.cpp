#include <string.h>
#include <stdio.h>
#include <time.h>

bool Error(const char *error)
{
	//Print to the console
	printf("Error: %s\n", error);
	
	//Open
	FILE *fp = fopen("error.log", "a");
	
	//Write the time
	char timeString[0x200];
	time_t timeValue = time(nullptr);
	tm dateTime = *localtime(&timeValue);
	
	sprintf(timeString, "%d-%d-%d %d:%d:%d: ", dateTime.tm_year + 1900, dateTime.tm_mon + 1, dateTime.tm_mday, dateTime.tm_hour, dateTime.tm_min, dateTime.tm_sec);
	fwrite(timeString, 1, strlen(timeString), fp);
	
	//Write the error
	fwrite(error, 1, strlen(error), fp);
	fwrite("\r\n", 1, 2, fp);
	
	//Close
	fclose(fp);
	return true;
}

bool Warn(const char *warning)
{
	//Print to the console
	printf("Warning: %s\n", warning);
	return true;
}
