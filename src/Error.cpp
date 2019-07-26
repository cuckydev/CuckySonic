#include "SDL_messagebox.h"
#include <stdlib.h>
#include <time.h>

bool Error(const char *error)
{
	//Print to the console
	printf("Error: %s\n", error);
	
	//Define message box
	const SDL_MessageBoxButtonData msgButtons[] = {
		{SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Log"},
		{SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Quit"},
	};
	
	const SDL_MessageBoxData msgData = {SDL_MESSAGEBOX_ERROR, NULL, "Error", error, SDL_arraysize(msgButtons), msgButtons, NULL};
	
	//Show us our error
	int button;
    if (SDL_ShowMessageBox(&msgData, &button) < 0)
		button = 0; //Default to "Log" if errored
	
	//Handle the button we pressed
	if (button == 0)
	{
		//Open
		SDL_RWops *fp = SDL_RWFromFile("error.log", "a");
		
		//Write the time
		char timeString[0x200];
		time_t timeValue = time(NULL);
		tm dateTime = *localtime(&timeValue);
		
		sprintf(timeString, "%d-%d-%d %d:%d:%d: ", dateTime.tm_year + 1900, dateTime.tm_mon + 1, dateTime.tm_mday, dateTime.tm_hour, dateTime.tm_min, dateTime.tm_sec);
		SDL_RWwrite(fp, timeString, 1, strlen(timeString));
		
		//Write the error
		SDL_RWwrite(fp, error, 1, strlen(error));
		SDL_RWwrite(fp, "\r\n", 1, 2);
		
		//Close
		SDL_RWclose(fp);
	}
	
	return true; //Return true so that this function can be directly returned into a function, so it's known to have failed
}
