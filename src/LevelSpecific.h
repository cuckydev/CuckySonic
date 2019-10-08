#pragma once
#include "Background.h"

typedef void (*PALETTECYCLEFUNCTION)();

void GHZ_PaletteCycle();
void EHZ_PaletteCycle();
void GHZ_Background(BACKGROUND *background, bool doScroll, int cameraX, int cameraY);
void EHZ_Background(BACKGROUND *background, bool doScroll, int cameraX, int cameraY);
