#include "Render.h"

void SOFTWAREBUFFER::Blit32(PALCOLOUR *backgroundColour, uint32_t *buffer, int pitch)
{
	//Clear to the given background colour
	if (backgroundColour != NULL)
	{
		uint32_t *clrBuffer = buffer;
		
		for (int i = 0; i < pitch * height; i++)
			*clrBuffer++ = backgroundColour->colour;
	}
	
	//Iterate through each layer
	for (int i = RENDERLAYERS - 1; i >= 0; i--)
	{
		//Iterate through each entry
		for (RENDERQUEUE *entry = queueEntry[i] - 1; entry >= queue[i]; entry--)
		{
			switch (entry->type)
			{
				case RENDERQUEUE_TEXTURE:
				{
					uint8_t *srcBuffer = entry->texture.texture->texture;
					uint32_t *dstBuffer = buffer + (entry->dest.x + entry->dest.y * pitch);
					
					//Get how to render the texture according to our x and y flipping
					const int finc = -(entry->texture.xFlip << 1) + 1;
					int fpitch;
					
					//Vertical flip
					if (entry->texture.yFlip)
					{
						//Start at bottom and move upwards
						srcBuffer += entry->texture.srcX + entry->texture.texture->width * (entry->texture.srcY + (entry->dest.h - 1));
						fpitch = -(entry->texture.texture->width + entry->dest.w);
					}
					else
					{
						//Move downwards
						srcBuffer += (entry->texture.srcX + entry->texture.srcY * entry->texture.texture->width);
						fpitch = entry->texture.texture->width - entry->dest.w;
					}
					
					//Horizontal flip
					if (entry->texture.xFlip)
					{
						//Start at right side
						srcBuffer += entry->dest.w - 1;
						fpitch += entry->dest.w * 2;
					}
					
					//Iterate through each pixel
					while (entry->dest.h-- > 0)
					{
						for (int x = 0; x < entry->dest.w; x++)
						{
							if (*srcBuffer)
								*dstBuffer = entry->texture.palette->colour[*srcBuffer].colour;
							srcBuffer += finc;
							dstBuffer++;
						}
						
						srcBuffer += fpitch;
						dstBuffer += pitch - entry->dest.w;
					}
					break;
				}
				case RENDERQUEUE_SOLID:
				{
					//Iterate through each pixel
					uint32_t *dstBuffer = buffer + (entry->dest.x + entry->dest.y * pitch);
					
					while (entry->dest.h-- > 0)
					{
						for (int x = 0; x < entry->dest.w; x++)
							*dstBuffer++ = entry->solid.colour->colour;
						dstBuffer += pitch - entry->dest.w;
					}
					break;
				}
			}
		}
		
		//Reset this layer to the beginning
		queueEntry[i] = queue[i];
	}
}
