#pragma once
#include "Render.h"

#define FADE_TIME 22

bool FadeInFromBlack(PALCOLOUR *palColour);
bool FadeOutToBlack(PALCOLOUR *palColour);
bool FadeInFromWhite(PALCOLOUR *palColour);
bool FadeOutToWhite(PALCOLOUR *palColour);

bool PaletteFadeInFromBlack(PALETTE *palette);
bool PaletteFadeOutToBlack(PALETTE *palette);
bool PaletteFadeInFromWhite(PALETTE *palette);
bool PaletteFadeOutToWhite(PALETTE *palette);

void FillPaletteBlack(PALETTE *palette);
void FillPaletteWhite(PALETTE *palette);
