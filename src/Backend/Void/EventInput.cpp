#include "../Input.h"

bool Backend_IsKeyDown(INPUTBINDKEY key)
{
	return false;
}

bool Backend_IsButtonDown(size_t index, INPUTBINDBUTTON button)
{
	return false;
}

void Backend_GetAnalogueStick(size_t index, int16_t *x, int16_t *y)
{
	return;
}

void Backend_UpdateInputState()
{
	return;
}

bool Backend_HandleEvents()
{
	return false;
}
