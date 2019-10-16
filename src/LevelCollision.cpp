#include <stdint.h>
#include "LevelCollision.h"
#include "Level.h"
#include "Game.h"
#include "Log.h"

//#define COLLISION_DEBUG

#ifdef COLLISION_DEBUG
	PALCOLOUR inner = {0xFF0000, 255, 0, 0, 255, 0, 0};
	PALCOLOUR outer = {0x00FF00, 0, 255, 0, 0, 255, 0};
#endif

//Get the layout tile at the given x,y coordinate
TILE *FindTile(int16_t x, int16_t y)
{
	if (x < 0 || x >= gLevel->layout.width * 16 || y < 0 || y >= gLevel->layout.height * 16)
		return nullptr;
	return &gLevel->layout.foreground[(y / 16) * gLevel->layout.width + (x / 16)];
}

#define TILE_ON_LAYER(alt, lrb, tile) (!(alt ? ((!lrb && !tile->altTop) || (lrb && !tile->altLRB)) : ((!lrb && !tile->norTop) || (lrb && !tile->norLRB))))

//Floor collision function, returns Y-difference from position given and the floor or ceiling level (non-flipped checks down for floors, flipped checks up for ceilings)
int16_t FindFloor2(int16_t x, int16_t y, COLLISIONLAYER layer, bool flipped, uint8_t *angle)
{
	#ifdef COLLISION_DEBUG
		RECT inQuad = {x - 1 - gLevel->camera->x, (flipped ? y ^ 0xF : y) - 1 - gLevel->camera->y, 2, 2};
		RECT ouQuad = {x - 2 - gLevel->camera->x, (flipped ? y ^ 0xF : y) - 2 - gLevel->camera->y, 4, 4};
		gSoftwareBuffer->DrawQuad(0, &inQuad, &inner);
		gSoftwareBuffer->DrawQuad(0, &ouQuad, &outer);
	#endif
	
	//Get our chunk tile
	TILE *tile = FindTile(x, y);
	
	//Flip our y-position if flipped
	if (tile != nullptr && tile->tile != 0 && TILE_ON_LAYER(LAYER_IS_ALT(layer), LAYER_IS_LRB(layer), tile))
	{
		TILEMAPPING *tileMap = &gLevel->tileMapping[tile->tile];
		COLLISIONTILE *collisionTile = &gLevel->collisionTile[LAYER_IS_ALT(layer) ? (tileMap->alternateColTile) : (tileMap->normalColTile)];
		
		if (collisionTile != gLevel->collisionTile)
		{
			//Get our angle
			if (angle != nullptr)
				*angle = collisionTile->angle;
			
			int16_t xPos = x;
			if (tile->xFlip) //Reverse if horizontally flipped
			{
				xPos = ~xPos;
				if (angle != nullptr)
					*angle = -*angle;
			}
			
			if (tile->yFlip) //Invert if vertically flipped
			{
				if (angle != nullptr)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			
			//Get our height in the heightmap
			int8_t height = collisionTile->normal[xPos & 0xF];
			
			if (tile->yFlip ^ flipped)
				height = -height;
			
			//Return surface position
			if (height > 0)
			{
				return 0xF - (height + (y & 0xF));
			}
			else if (height < 0)
			{
				int16_t distance = y & 0xF;
				if (height + distance < 0)
					return ~distance;
			}
		}
	}
	
	return 0xF - (y & 0xF);
}

int16_t FindFloor(int16_t x, int16_t y, COLLISIONLAYER layer, bool flipped, uint8_t *angle)
{
	#ifdef COLLISION_DEBUG
		RECT inQuad = {x - 1 - gLevel->camera->x, y - 1 - gLevel->camera->y, 2, 2};
		RECT ouQuad = {x - 2 - gLevel->camera->x, y - 2 - gLevel->camera->y, 4, 4};
		gSoftwareBuffer->DrawQuad(0, &inQuad, &inner);
		gSoftwareBuffer->DrawQuad(0, &ouQuad, &outer);
	#endif
	
	//Flip our y-position if flipped
	if (flipped)
		y ^= 0xF;
	
	//Get our chunk tile
	TILE *tile = FindTile(x, y);
	
	if (tile != nullptr && tile->tile != 0 && TILE_ON_LAYER(LAYER_IS_ALT(layer), LAYER_IS_LRB(layer), tile))
	{
		TILEMAPPING *tileMap = &gLevel->tileMapping[tile->tile];
		COLLISIONTILE *collisionTile = &gLevel->collisionTile[LAYER_IS_ALT(layer) ? (tileMap->alternateColTile) : (tileMap->normalColTile)];
		
		if (collisionTile != gLevel->collisionTile)
		{
			//Get our angle
			if (angle != nullptr)
				*angle = collisionTile->angle;
			
			int16_t xPos = x;
			if (tile->xFlip) //Reverse if horizontally flipped
			{
				xPos = ~xPos;
				if (angle != nullptr)
					*angle = -*angle;
			}
			
			if (tile->yFlip) //Invert if vertically flipped
			{
				if (angle != nullptr)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			
			//Get our height in the heightmap
			int8_t height = collisionTile->normal[xPos & 0xF];
			
			if (tile->yFlip ^ flipped)
				height = -height;
			
			//Either return this surface or check the tile above
			if (height > 0)
			{
				if (height != 0x10)
					return 0xF - (height + (y & 0xF));
				else
					return FindFloor2(x, y - (flipped ? -0x10 : 0x10), layer, flipped, angle) - 0x10;
			}
			else if (height < 0)
			{
				if (height + (y & 0xF) < 0)
					return FindFloor2(x, y - (flipped ? -0x10 : 0x10), layer, flipped, angle) - 0x10;
			}
		}
	}
	
	return FindFloor2(x, y + (flipped ? -0x10 : 0x10), layer, flipped, angle) + 0x10;
}

//Wall collision function, returns X-difference from position given and the wall's surface (non-flipped checks right, flipped checks left)
int16_t FindWall2(int16_t x, int16_t y, COLLISIONLAYER layer, bool flipped, uint8_t *angle)
{
	#ifdef COLLISION_DEBUG
		RECT inQuad = {(flipped ? x ^ 0xF : x) - 1 - gLevel->camera->x, y - 1 - gLevel->camera->y, 2, 2};
		RECT ouQuad = {(flipped ? x ^ 0xF : x) - 2 - gLevel->camera->x, y - 2 - gLevel->camera->y, 4, 4};
		gSoftwareBuffer->DrawQuad(0, &inQuad, &inner);
		gSoftwareBuffer->DrawQuad(0, &ouQuad, &outer);
	#endif
	
	//Get our chunk tile
	TILE *tile = FindTile(x, y);
	
	if (tile != nullptr && tile->tile != 0 && TILE_ON_LAYER(LAYER_IS_ALT(layer), LAYER_IS_LRB(layer), tile))
	{
		TILEMAPPING *tileMap = &gLevel->tileMapping[tile->tile];
		COLLISIONTILE *collisionTile = &gLevel->collisionTile[LAYER_IS_ALT(layer) ? (tileMap->alternateColTile) : (tileMap->normalColTile)];
		
		if (collisionTile != gLevel->collisionTile)
		{
			//Get our angle
			if (angle != nullptr)
				*angle = collisionTile->angle;
			
			int16_t yPos = y;
			if (tile->yFlip) //Invert if vertically flipped
			{
				yPos = ~yPos;
				if (angle != nullptr)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			
			if (tile->xFlip) //Reverse if horizontally flipped
			{
				if (angle != nullptr)
					*angle = -*angle;
			}
			
			//Get our height in the heightmap
			int8_t height = collisionTile->rotated[yPos & 0xF];
			
			if (tile->xFlip ^ flipped)
				height = -height;
			
			//Return surface position
			if (height > 0)
			{
				return 0xF - (height + (x & 0xF));
			}
			else if (height < 0)
			{
				int16_t distance = x & 0xF;
				if (height + distance < 0)
					return ~distance;
			}
		}
	}
	
	return 0xF - (x & 0xF);
}

int16_t FindWall(int16_t x, int16_t y, COLLISIONLAYER layer, bool flipped, uint8_t *angle)
{
	#ifdef COLLISION_DEBUG
		RECT inQuad = {x - 1 - gLevel->camera->x, y - 1 - gLevel->camera->y, 2, 2};
		RECT ouQuad = {x - 2 - gLevel->camera->x, y - 2 - gLevel->camera->y, 4, 4};
		gSoftwareBuffer->DrawQuad(0, &inQuad, &inner);
		gSoftwareBuffer->DrawQuad(0, &ouQuad, &outer);
	#endif
	
	//Flip our x-position if flipped
	if (flipped)
		x ^= 0xF;
	
	//Get our chunk tile
	TILE *tile = FindTile(x, y);
	
	if (tile != nullptr && tile->tile != 0 && TILE_ON_LAYER(LAYER_IS_ALT(layer), LAYER_IS_LRB(layer), tile))
	{
		TILEMAPPING *tileMap = &gLevel->tileMapping[tile->tile];
		COLLISIONTILE *collisionTile = &gLevel->collisionTile[LAYER_IS_ALT(layer) ? (tileMap->alternateColTile) : (tileMap->normalColTile)];
		
		if (collisionTile != gLevel->collisionTile)
		{
			//Get our angle
			if (angle != nullptr)
				*angle = collisionTile->angle;
			
			int16_t yPos = y;
			if (tile->yFlip) //Invert if vertically flipped
			{
				yPos = ~yPos;
				if (angle != nullptr)
					*angle = (-(*angle + 0x40)) - 0x40;
			}
			
			if (tile->xFlip) //Reverse if horizontally flipped
			{
				if (angle != nullptr)
					*angle = -*angle;
			}
			
			//Get our height in the heightmap
			int8_t height = collisionTile->rotated[yPos & 0xF];
			
			if (tile->xFlip ^ flipped)
				height = -height;
			
			//Either return this surface or check the tile above
			if (height > 0)
			{
				if (height != 0x10)
					return 0xF - (height + (x & 0xF));
				else
					return FindWall2(x - (flipped ? -0x10 : 0x10), y, layer, flipped, angle) - 0x10;
			}
			else if (height < 0)
			{
				if (height + (x & 0xF) < 0)
					return FindWall2(x - (flipped ? -0x10 : 0x10), y, layer, flipped, angle) - 0x10;
			}
		}
	}
	
	return FindWall2(x + (flipped ? -0x10 : 0x10), y, layer, flipped, angle) + 0x10;
}
