#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"

const uint16_t switchRadius[] = {0x20, 0x40, 0x80, 0x100};

#define INDEX_TO_BIT(i)	(1 << (i & 0x7))

void ObjPathSwitcher(OBJECT *object)
{
	enum scratch
	{
		SCRATCH_RADIUS = 0,	// 2 bytes
		SCRATCH_PLAYER_ACROSS_0 = 2, // PLAYERS / 8 bytes
	};
	
	switch (object->routine)
	{
		case 0:
			//Advance to next routine
			object->routine++;
			
			//Initialize properties
			object->scratchU16[SCRATCH_RADIUS] = switchRadius[object->subtype & 0x03];
			
			if (object->subtype & 0x04)
			{
				//Are players already across the switcher?
				for (int i = 0; i < PLAYERS; i++)
				{
					if (object->y.pos < gLevel->player[i]->y.pos)
						object->scratchU8[SCRATCH_PLAYER_ACROSS_0 + (i / 8)] |= INDEX_TO_BIT(i);
				}
			}
			else
			{
				//Are players already across the switcher?
				for (int i = 0; i < PLAYERS; i++)
				{
					if (object->x.pos < gLevel->player[i]->x.pos)
						object->scratchU8[SCRATCH_PLAYER_ACROSS_0 + (i / 8)] |= INDEX_TO_BIT(i);
				}
			}
			
		//Fallthrough
		case 1:
			if (object->subtype & 0x04)
			{
				//Vertical path switcher
				for (int i = 0; i < PLAYERS; i++)
				{
					//Skip over if debug is being used
					if (gLevel->player[i]->debug)
						continue;
					
					//Get the flag to use
					uint8_t *flag = &object->scratchU8[SCRATCH_PLAYER_ACROSS_0 + (i / 8)];
					
					if ((*flag & INDEX_TO_BIT(i)) == 0)
					{
						//Check if we're crossing
						if (object->y.pos <= gLevel->player[i]->y.pos)
						{
							//Set the crossed flag
							*flag |= INDEX_TO_BIT(i);
							
							//Get the bounds for our actual switching
							const int16_t left = object->x.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t right = object->x.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (gLevel->player[i]->x.pos >= left && gLevel->player[i]->x.pos < right)
							{
								if ((object->subtype & 0x80) == 0 || gLevel->player[i]->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x08)
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
								}
							}
						}
					}
					else
					{
						//Check if we're crossing back over
						if (object->y.pos > gLevel->player[i]->y.pos)
						{
							//Clear the crossed flag
							*flag &= ~INDEX_TO_BIT(i);
							
							//Get the bounds for our actual switching
							const int16_t left = object->x.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t right = object->x.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (gLevel->player[i]->x.pos >= left && gLevel->player[i]->x.pos < right)
							{
								if ((object->subtype & 0x80) == 0 || gLevel->player[i]->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x10)
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				//Horizontal path switcher
				for (int i = 0; i < PLAYERS; i++)
				{
					//Skip over if debug is being used
					if (gLevel->player[i]->debug)
						continue;
					
					//Get the flag to use
					uint8_t *flag = &object->scratchU8[SCRATCH_PLAYER_ACROSS_0 + (i / 8)];
					
					if ((*flag & INDEX_TO_BIT(i)) == 0)
					{
						//Check if we're crossing
						if (object->x.pos <= gLevel->player[i]->x.pos)
						{
							//Set the crossed flag
							*flag |= INDEX_TO_BIT(i);
							
							//Get the bounds for our actual switching
							const int16_t top = object->y.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t bottom = object->y.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (gLevel->player[i]->y.pos >= top && gLevel->player[i]->y.pos < bottom)
							{
								if ((object->subtype & 0x80) == 0 || gLevel->player[i]->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x08)
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
								}
							}
						}
					}
					else
					{
						//Check if we're crossing back over
						if (object->x.pos > gLevel->player[i]->x.pos)
						{
							//Clear the crossed flag
							*flag &= ~INDEX_TO_BIT(i);
							
							//Get the bounds for our actual switching
							const int16_t top = object->y.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t bottom = object->y.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (gLevel->player[i]->y.pos >= top && gLevel->player[i]->y.pos < bottom)
							{
								if ((object->subtype & 0x80) == 0 || gLevel->player[i]->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x10)
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											gLevel->player[i]->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											gLevel->player[i]->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
								}
							}
						}
					}
				}
			}
			break;
	}
}
