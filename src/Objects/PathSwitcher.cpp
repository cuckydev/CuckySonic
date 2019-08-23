#include <stdint.h>
#include "../Level.h"
#include "../Game.h"
#include "../Log.h"

const uint16_t switchRadius[] = {0x20, 0x40, 0x80, 0x100};

void ObjPathSwitcher(OBJECT *object)
{
	enum SCRATCH
	{
		SCRATCH_RADIUS = 0,	// 2 bytes
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
				int i = 0;
				for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
				{
					if (object->y.pos < player->y.pos)
						object->playerContact[i].extraBit = true;
					i++;
				}
			}
			else
			{
				//Are players already across the switcher?
				int i = 0;
				for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
				{
					if (object->x.pos < player->x.pos)
						object->playerContact[i].extraBit = true;
					i++;
				}
			}
//Fallthrough
		case 1:
			if (object->subtype & 0x04)
			{
				//Vertical path switcher
				int i = 0;
				for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
				{
					if (object->playerContact[i].extraBit == false)
					{
						//Check if we're crossing
						if (object->y.pos <= player->y.pos)
						{
							//Set the crossed flag
							object->playerContact[i].extraBit = true;
							
							//Get the bounds for our actual switching
							const int16_t left = object->x.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t right = object->x.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (player->x.pos >= left && player->x.pos < right)
							{
								if ((object->subtype & 0x80) == 0 || player->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x08)
										{
											player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
									
									player->highPriority = (object->subtype & 0x20) != 0;
								}
							}
						}
					}
					else
					{
						//Check if we're crossing back over
						if (object->y.pos > player->y.pos)
						{
							//Clear the crossed flag
							object->playerContact[i].extraBit = false;
							
							//Get the bounds for our actual switching
							const int16_t left = object->x.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t right = object->x.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (player->x.pos >= left && player->x.pos < right)
							{
								if ((object->subtype & 0x80) == 0 || player->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x10)
										{
											player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
									
									player->highPriority = (object->subtype & 0x40) != 0;
								}
							}
						}
					}
					
					i++;
				}
			}
			else
			{
				//Horizontal path switcher
				int i = 0;
				for (PLAYER *player = gLevel->playerList; player != NULL; player = player->next)
				{
					if (object->playerContact[i].extraBit == false)
					{
						//Check if we're crossing
						if (object->x.pos <= player->x.pos)
						{
							//Set the crossed flag
							object->playerContact[i].extraBit = true;
							
							//Get the bounds for our actual switching
							const int16_t top = object->y.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t bottom = object->y.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (player->y.pos >= top && player->y.pos < bottom)
							{
								if ((object->subtype & 0x80) == 0 || player->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x08)
										{
											player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
									
									player->highPriority = (object->subtype & 0x20) != 0;
								}
							}
						}
					}
					else
					{
						//Check if we're crossing back over
						if (object->x.pos > player->x.pos)
						{
							//Clear the crossed flag
							object->playerContact[i].extraBit = false;
							
							//Get the bounds for our actual switching
							const int16_t top = object->y.pos - object->scratchU16[SCRATCH_RADIUS];
							const int16_t bottom = object->y.pos + object->scratchU16[SCRATCH_RADIUS];
							
							if (player->y.pos >= top && player->y.pos < bottom)
							{
								if ((object->subtype & 0x80) == 0 || player->status.inAir == false)
								{
									if (!object->renderFlags.xFlip)
									{
										if (object->subtype & 0x10)
										{
											player->topSolidLayer = COLLISIONLAYER_ALTERNATE_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_ALTERNATE_LRB;
										}
										else
										{
											player->topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
											player->lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
										}
									}
									
									player->highPriority = (object->subtype & 0x40) != 0;
								}
							}
						}
					}
					
					i++;
				}
			}
			break;
	}
}
