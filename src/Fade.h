#pragma once
#include "Render.h"

//[[Single colour operations]]
//Fading in/out black
bool FadeInFromBlack(COLOUR *colour);
bool FadeOutToBlack(COLOUR *colour);
//Fading in/out white
bool FadeInFromWhite(COLOUR *colour);
bool FadeOutToWhite(COLOUR *colour);

//[[Full palette operations]]
//Fading in/out black
bool PaletteFadeInFromBlack(PALETTE *palette);
bool PaletteFadeOutToBlack(PALETTE *palette);
//Fading in/out white
bool PaletteFadeInFromWhite(PALETTE *palette);
bool PaletteFadeOutToWhite(PALETTE *palette);

void FillPaletteBlack(PALETTE *palette);
void FillPaletteWhite(PALETTE *palette);
