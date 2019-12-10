#pragma once
#include <string>
#include <stdint.h>
#include "LinkedList.h"

//Rect and point structures
struct RECT { int x, y, w, h; };
struct POINT { int x, y; };
	
//Palette and colours
class PIXELFORMAT
{
	private:
		uint8_t bitsPerPixel, bytesPerPixel;	//Sizes of a pixel
		uint32_t rMask, gMask, bMask, aMask;	//Mask of the colour in the pixel
		uint8_t rLoss, gLoss, bLoss, aLoss;		//Bitshift to mask size
		uint8_t rShift, gShift, bShift, aShift;	//Bitshift into mask position
};

class COLOUR
{
	public:
		uint32_t colour;	//The natively formatted colour
		uint8_t r, g, b;	//Modified RGB colours
		uint8_t mr, mg, mb;	//The original RGB colours
		
	public:
		//Constructors
		COLOUR() { return; } //Undefined constructor (to have colour set later)
		
		COLOUR(const COLOUR &copyColour) //Copies the given colour
		{
			//Copy our given colour
			colour = copyColour.colour;
			r = copyColour.r; g = copyColour.g; b = copyColour.b;
			mr = copyColour.mr; mg = copyColour.mg; mb = copyColour.mb;
		}
		
		COLOUR(const uint8_t setR, const uint8_t setG, const uint8_t setB) //Sets according to the given RGB
		{
			//Set our RGB accordingly and get our native colour
			SetColour(true, true, true, setR, setG, setB);
		}
		
		//Destructor (blank)
		~COLOUR() { return; };
		
		//Colour modification
		inline void SetColour(const bool setMod, const bool setOrig, const bool doRegen, const uint8_t setR, const uint8_t setG, const uint8_t setB)	//Completely changes the colour
		{
			//Set our colours, then get our actual pixel colour
			if (setMod)		{ r = setR; g = setG; b = setB; }
			if (setOrig)	{ mr = setR; mg = setG; mb = setB; }
			if (doRegen)	{ Regen(setR, setG, setB); }
		}
		
		inline void Regen(const uint8_t setR, const uint8_t setG, const uint8_t setB)
		{
			//Get our colour using the native format
			colour = (setR << 16) | (setG << 8) | setB;
		}
};

class PALETTE
{
	public:
		//Colour array
		size_t colours;				//How many colours in the array
		COLOUR *colour = nullptr;	//The actual colours
		
	public:
		//Constructors
		PALETTE(const size_t setColours) //Allocated undefined array of setColours length
		{
			//Allocate array
			colours = setColours;
			colour = new COLOUR[colours = setColours]{};
		}
		
		PALETTE(const size_t setColours, const COLOUR setColour) //Creates setColours colours with the value of setColour
		{
			//Free old colour array (if exists)
			if (colour != nullptr)
				delete[] colour;
			
			//Create our array, filled with setColour
			colour = new COLOUR[colours = setColours]{setColour};
		}
		
		PALETTE(const size_t setColours, const COLOUR *setColour) //Use the given array
		{
			//Free old colour array (if exists)
			if (colour != nullptr)
				delete[] colour;
			
			//Copy given array
			colours = setColours;
			colour = new COLOUR[colours = setColours]{};
			for (size_t i = 0; i < colours; i++)
				colour[i] = COLOUR(setColour[i]);
		}
		
		//Destructor
		~PALETTE()
		{
			//Free colour array
			delete[] colour;
		}
};

//Texture class
class TEXTURE
{
	public:
		//Status
		const char *fail = nullptr;
		
		//Source file (if applicable)
		std::string source;
		
		//Texture data
		uint8_t *texture = nullptr;
		int width;
		int height;
		
		//Loaded palette
		PALETTE *loadedPalette;
		
	public:
		TEXTURE(std::string path);
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
			const PALETTE *palette;
			const TEXTURE *texture;
			bool xFlip, yFlip;
		} texture;
		struct
		{
			const COLOUR *colour;
		} solid;
	};
};

//Software framebuffer class
class SOFTWAREBUFFER
{
	public:
		//Failure
		const char *fail = nullptr;
		
		//Render queue
		LINKEDLIST<RENDERQUEUE> queue[RENDERLAYERS];
		
		//Dimensions of buffer
		int width;
		int height;
		
	public:
		SOFTWAREBUFFER(int bufWidth, int bufHeight);
		~SOFTWAREBUFFER();
		
		void DrawPoint(const int layer, const POINT *point, const COLOUR *colour);
		void DrawQuad(const int layer, const RECT *quad, const COLOUR *colour);
		void DrawTexture(TEXTURE *texture, PALETTE *palette, const RECT *src, const int layer, const int x, const int y, const bool xFlip, const bool yFlip);
		
		bool RenderToScreen(const COLOUR *backgroundColour);
		
		//Blit function
		template <typename T> inline void BlitQueue(const COLOUR *backgroundColour, const T *buffer, const int pitch)
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
				for (size_t i = 0; i < queue[i].size(); i++)
				{
					RENDERQUEUE *entry = &queue[i];
					
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
extern RENDERSPEC gRenderSpec;
extern SOFTWAREBUFFER *gSoftwareBuffer;

//Sub-system functions
bool InitializeRender();
void QuitRender();
