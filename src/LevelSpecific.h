#pragma once
#include "Level.h"

void GHZ_PaletteCycle(LEVEL *lvl);
void EHZ_PaletteCycle(LEVEL *lvl);

void GHZ_BackgroundScroll(bool updateScroll, uint16_t *array, int16_t *cameraX, int16_t *cameraY);
void EHZ_BackgroundScroll(bool updateScroll, uint16_t *array, int16_t *cameraX, int16_t *cameraY);
