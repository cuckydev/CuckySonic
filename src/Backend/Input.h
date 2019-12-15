#pragma once
#include <stddef.h>
#include <stdint.h>
#include "../Input.h"

bool Backend_IsKeyDown(INPUTBINDKEY key);
bool Backend_IsButtonDown(size_t index, INPUTBINDBUTTON button);
void Backend_GetAnalogueStick(size_t index, int16_t *x, int16_t *y);
void Backend_UpdateInputState();
