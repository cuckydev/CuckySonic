#pragma once
#include <stdint.h>
#include <deque>

#ifdef BACKEND_SDL2
	#include "SDL_render.h"
	#include "SDL_pixels.h"
	#define BACKEND_TEXTURE		SDL_Texture
	#define BACKEND_PIXELFORMAT	SDL_PixelFormat
#endif

//Rect and point structures
struct RECT { int x, y, w, h; };
struct POINT { int x, y; };
	
//Palette line
struct PALCOLOUR
{
	uint32_t colour;		//The natively formatted colour
	uint8_t r, g, b;		//The original RGB colours
	uint8_t ogr, ogg, ogb;	//For palette effects such as fading
};

struct PALETTE
{
	PALCOLOUR colour[0x100];
};

void SetPaletteColour(PALCOLOUR *palColour, uint8_t r, uint8_t g, uint8_t b);
void ModifyPaletteColour(PALCOLOUR *palColour, uint8_t r, uint8_t g, uint8_t b);
void RegenPaletteColour(PALCOLOUR *palColour);

//Texture classes
class TEXTURE
{
	public:
		//Status
		const char *fail;
		
		//Source file (if applicable)
		const char *source;
		
		//Texture data
		uint8_t *texture;
		int width;
		int height;
		
		//Loaded palette
		PALETTE *loadedPalette;
		
	public:
		TEXTURE(const char *path);
		TEXTURE(uint8_t *data, int dWidth, int dHeight);
		~TEXTURE();
};

//Render queue structure
#define RENDERLAYERS 0x100

enum RENDERQUEUE_TYPE
{
	RENDERQUEUE_TEXTURE,
	RENDERQUEUE_SOLID,
};

struct RENDERQUEUE
{
	//Shared
	RENDERQUEUE_TYPE type;
	RECT dest;
	
	//Our union for different types
	union
	{
		struct
		{
			int srcX, srcY;
			PALETTE *palette;
			TEXTURE *texture;
			bool xFlip, yFlip;
		} texture;
		struct
		{
			PALCOLOUR *colour;
		} solid;
	};
	
	//Linked list
	RENDERQUEUE *next;
};

//Software framebuffer class
class SOFTWAREBUFFER
{
	public:
		//Failure
		const char *fail;
		
		//Render queue
		RENDERQUEUE *queue[RENDERLAYERS];
		
		//Dimensions and pixel drawn buffer
		int width;
		int height;
		
	private:
		BACKEND_TEXTURE *texture;
		
	public:
		SOFTWAREBUFFER(int bufWidth, int bufHeight);
		~SOFTWAREBUFFER();
		
		void DrawPoint(int layer, const POINT *point, PALCOLOUR *colour);
		void DrawQuad(int layer, const RECT *quad, PALCOLOUR *colour);
		void DrawTexture(TEXTURE *texture, PALETTE *palette, const RECT *src, int layer, int x, int y, bool xFlip, bool yFlip);
		
		bool RenderToScreen(PALCOLOUR *backgroundColour);
		
		//Blit function
		template <typename T> void Blit(PALCOLOUR *backgroundColour, T *buffer, int pitch)
		{
			//Clear to the given background colour
			if (backgroundColour != nullptr)
			{
				T *clrBuffer = buffer;
				
				for (int i = 0; i < pitch * height; i++)
					*clrBuffer++ = backgroundColour->colour;
			}
			
			//Iterate through each layer
			for (int i = RENDERLAYERS - 1; i >= 0; i--)
			{
				//Iterate through each entry
				for (RENDERQUEUE *entry = queue[i]; entry != nullptr; entry = entry->next)
				{
					switch (entry->type)
					{
						case RENDERQUEUE_TEXTURE:
						{
							uint8_t *srcBuffer = entry->texture.texture->texture;
							T *dstBuffer = buffer + (entry->dest.x + entry->dest.y * pitch);
							
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
							T *dstBuffer = buffer + (entry->dest.x + entry->dest.y * pitch);
							
							while (entry->dest.h-- > 0)
							{
								for (int x = 0; x < entry->dest.w; x++)
									*dstBuffer++ = entry->solid.colour->colour;
								dstBuffer += pitch - entry->dest.w;
							}
							break;
						}
						default:
						{
							break;
						}
					}
				}
			}
		}

};

//Current render specifications
struct RENDERSPEC
{
	int width, height, scale;
};

//Globals
extern SOFTWAREBUFFER *gSoftwareBuffer;
extern RENDERSPEC gRenderSpec;

bool InitializeRender();
void QuitRender();
