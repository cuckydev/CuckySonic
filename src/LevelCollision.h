#pragma once
#include <stdint.h>

enum COLLISIONLAYER
{
	COLLISIONLAYER_NORMAL_TOP,
	COLLISIONLAYER_NORMAL_LRB,
	COLLISIONLAYER_ALTERNATE_TOP,
	COLLISIONLAYER_ALTERNATE_LRB,
};

#define LAYER_IS_ALT(layer)	(layer == COLLISIONLAYER_ALTERNATE_TOP || layer == COLLISIONLAYER_ALTERNATE_LRB)
#define LAYER_IS_LRB(layer)	(layer == COLLISIONLAYER_NORMAL_LRB || layer == COLLISIONLAYER_ALTERNATE_LRB)

int16_t GetCollisionH(int16_t x, int16_t y, COLLISIONLAYER layer, bool flipped, uint8_t *angle);
int16_t GetCollisionV(int16_t x, int16_t y, COLLISIONLAYER layer, bool flipped, uint8_t *angle);
