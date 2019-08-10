#pragma once
#include <stdint.h>

#define abs(x) (x < 0 ? -x : x)

void GetSine(uint8_t angle, int16_t *sin, int16_t *cos);
