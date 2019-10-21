#include <stdlib.h>
#include <string.h>
#include <cmath>

#include "SDL_render.h"
#include "SDL_timer.h"

#include "../CommonMacros.h"
#include "../Log.h"
#include "../GameConstants.h"
#include "../Render.h"
#include "../Error.h"
#include "../Filesystem.h"

//Window and renderer
SDL_Window *window;
SDL_Renderer *renderer;
SOFTWAREBUFFER *gSoftwareBuffer;

SDL_PixelFormat *nativeFormat;

//Current render specifications
RENDERSPEC gRenderSpec = {398, 224, 2};

//VSync / framerate limiter
unsigned int vsyncMultiple = 0;
const long double framerateMilliseconds = (1000.0 / FRAMERATE);

//Texture class
TEXTURE::TEXTURE(std::deque<TEXTURE*> *linkedList, const char *path)
{
	LOG(("Loading texture from %s... ", path));
	memset(this, 0, sizeof(TEXTURE));
	
	//Load bitmap
	source = duplicateString(path);
	
	char *filepath = AllocPath(gBasePath, path, nullptr);
	SDL_Surface *bitmap = SDL_LoadBMP(filepath);
	delete[] filepath;
	
	if (bitmap == nullptr)
	{
		fail = SDL_GetError();
		return;
	}
	
	//Check if this format is valid
	if (bitmap->format->palette == nullptr || bitmap->format->BytesPerPixel > 1)
	{
		SDL_FreeSurface(bitmap);
		fail = "Bitmap is not an 8-bit indexed .bmp";
		return;
	}
	
	//Create our palette using the image's palette
	if ((loadedPalette = new PALETTE) == nullptr)
	{
		SDL_FreeSurface(bitmap);
		fail = "Failed to allocate palette for texture";
		return;
	}
	
	int i;
	for (i = 0; i < bitmap->format->palette->ncolors; i++)
		SetPaletteColour(&loadedPalette->colour[i], bitmap->format->palette->colors[i].r, bitmap->format->palette->colors[i].g, bitmap->format->palette->colors[i].b);
	for (; i < 0x100; i++)
	{
		if (bitmap->format->palette->ncolors)
			loadedPalette->colour[i] = loadedPalette->colour[0];
		else
			SetPaletteColour(&loadedPalette->colour[i], 0, 0, 0);
	}
	
	//Allocate texture data
	texture = new uint8_t[bitmap->pitch * bitmap->h];
	
	if (texture == nullptr)
	{
		SDL_FreeSurface(bitmap);
		delete loadedPalette;
		
		fail = "Failed to allocate texture buffer";
		return;
	}
	
	//Copy our data
	memcpy(texture, bitmap->pixels, bitmap->pitch * bitmap->h);
	width = bitmap->pitch;
	height = bitmap->h;
	
	//Free bitmap surface
	SDL_FreeSurface(bitmap);
	
	//Attach to linked list if given
	if (linkedList != nullptr)
		linkedList->push_back(this);
	
	LOG(("Success!\n"));
}

TEXTURE::TEXTURE(std::deque<TEXTURE*> *linkedList, uint8_t *data, int dWidth, int dHeight)
{
	LOG(("Loading texture from memory location %p dimensions %dx%d... ", (void*)data, dWidth, dHeight));
	memset(this, 0, sizeof(TEXTURE));
	
	//Allocate texture data
	texture = new uint8_t[dWidth * dHeight];
	
	if (texture == nullptr)
	{
		fail = "Failed to allocate texture buffer";
		return;
	}
	
	//Copy our data
	memcpy(texture, data, dWidth * dHeight);
	width = dWidth;
	height = dHeight;
	
	//Attach to linked list if given
	if (linkedList != nullptr)
		linkedList->push_back(this);
	
	LOG(("Success!\n"));
}

TEXTURE::~TEXTURE()
{
	//Check if any software buffers are referencing us and remove said entries
	for (int i = 0; i < RENDERLAYERS; i++)
	{
		for (RENDERQUEUE *entry = gSoftwareBuffer->queue[i]; entry != nullptr; entry = entry->next)
		{
			if (entry->type == RENDERQUEUE_TEXTURE && entry->texture.texture == this)
			{
				//Remove from linked list then delete entry
				for (RENDERQUEUE **dcEntry = &gSoftwareBuffer->queue[i]; *dcEntry != nullptr; dcEntry = &(*dcEntry)->next)
				{
					if (*dcEntry == entry)
					{
						*dcEntry = entry->next;
						break;
					}
				}
				
				delete entry;
			}
		}
	}
	//Free data
	if (texture != nullptr)
		delete[] texture;
	if (loadedPalette)
		delete loadedPalette;
	if (source)
		delete[] source;
}

//Full-colour texture class
TEXTURE_FULLCOLOUR::TEXTURE_FULLCOLOUR(std::deque<TEXTURE_FULLCOLOUR*> *linkedList, const char *path)
{
	LOG(("Loading full-colour texture from %s... ", path));
	memset(this, 0, sizeof(TEXTURE));
	
	//Load bitmap
	source = path;
	
	char *filepath = AllocPath(gBasePath, path, nullptr);
	SDL_Surface *bitmap = SDL_LoadBMP(filepath);
	delete[] filepath;
	
	if (bitmap == nullptr)
	{
		fail = SDL_GetError();
		return;
	}
	
	//Allocate texture data
	texture = new uint32_t[bitmap->pitch / bitmap->format->BytesPerPixel * bitmap->h];
	
	if (texture == nullptr)
	{
		SDL_FreeSurface(bitmap);
		fail = "Failed to allocate texture buffer";
		return;
	}
	
	//Convert to our native format
	width = bitmap->pitch / bitmap->format->BytesPerPixel;
	height = bitmap->h;
	
	for (int v = 0; v < width * height; v++)
	{
		uint32_t pixel;
		switch (bitmap->format->BytesPerPixel)
		{
			case 1:
				pixel = ((uint8_t*)bitmap->pixels)[v];
				break;
			case 2:
				pixel = ((uint16_t*)bitmap->pixels)[v];
				break;
			case 3:
				#if SDL_BYTEORDER == SDL_BIG_ENDIAN
					pixel = (((uint8_t*)bitmap->pixels)[v * bitmap->format->BytesPerPixel + 0] << 16) | (((uint8_t*)bitmap->pixels)[v * bitmap->format->BytesPerPixel + 1] << 8) | (((uint8_t*)bitmap->pixels)[v * bitmap->format->BytesPerPixel + 2]);
				#else
					pixel = (((uint8_t*)bitmap->pixels)[v * bitmap->format->BytesPerPixel + 0]) | (((uint8_t*)bitmap->pixels)[v * bitmap->format->BytesPerPixel + 1] << 8) | (((uint8_t*)bitmap->pixels)[v * bitmap->format->BytesPerPixel + 2] << 16);
				#endif
				break;
			case 4:
				pixel = ((uint32_t*)bitmap->pixels)[v];
				break;
		}
		
		uint8_t r, g, b;
		SDL_GetRGB(pixel, bitmap->format, &r, &g, &b);
		texture[v] = SDL_MapRGB(nativeFormat, r, g, b);
	}
	
	//Free bitmap surface
	SDL_FreeSurface(bitmap);
	
	//Attach to linked list if given
	if (linkedList != nullptr)
		linkedList->push_back(this);
	
	LOG(("Success!\n"));
}

TEXTURE_FULLCOLOUR::~TEXTURE_FULLCOLOUR()
{
	//Check if the software buffer is referencing us and remove said entries
	for (int i = 0; i < RENDERLAYERS; i++)
	{
		for (RENDERQUEUE *entry = gSoftwareBuffer->queue[i]; entry != nullptr; entry = entry->next)
		{
			if (entry->type == RENDERQUEUE_TEXTURE_FULLCOLOUR && entry->textureFullColour.texture == this)
			{
				//Remove from linked list then delete entry
				for (RENDERQUEUE **dcEntry = &gSoftwareBuffer->queue[i]; *dcEntry != nullptr; dcEntry = &(*dcEntry)->next)
				{
					if (*dcEntry == entry)
					{
						*dcEntry = entry->next;
						break;
					}
				}
				
				delete entry;
			}
		}
	}
	
	//Free texture data
	if (texture != nullptr)
		delete[] texture;
}

//Palette functions
void SetPaletteColour(PALCOLOUR *palColour, uint8_t r, uint8_t g, uint8_t b)
{
	//Set formatted colour
	palColour->colour = SDL_MapRGB(nativeFormat, r, g, b);
	
	//Set colours
	palColour->r = r;
	palColour->g = g;
	palColour->b = b;
	
	//Set original colours
	palColour->ogr = r;
	palColour->ogg = g;
	palColour->ogb = b;
}

void ModifyPaletteColour(PALCOLOUR *palColour, uint8_t r, uint8_t g, uint8_t b)
{
	//Set formatted colour
	palColour->colour = SDL_MapRGB(nativeFormat, r, g, b);
	
	//Set colours
	palColour->r = r;
	palColour->g = g;
	palColour->b = b;
}

void RegenPaletteColour(PALCOLOUR *palColour)
{
	//Set formatted colour
	palColour->colour = SDL_MapRGB(nativeFormat, palColour->r, palColour->g, palColour->b);
}

//Software buffer class
SOFTWAREBUFFER::SOFTWAREBUFFER(int bufWidth, int bufHeight)
{
	//Set our properties
	memset(this, 0, sizeof(SOFTWAREBUFFER));
	width = bufWidth;
	height = bufHeight;
	
	//Create the render texture
	if ((texture = SDL_CreateTexture(renderer, nativeFormat->format, SDL_TEXTUREACCESS_STREAMING, width, height)) == nullptr)
	{
		fail = SDL_GetError();
		return;
	}
}

SOFTWAREBUFFER::~SOFTWAREBUFFER()
{
	//Free our render texture
	SDL_DestroyTexture(texture);
}

//Drawing functions
void SOFTWAREBUFFER::DrawPoint(int layer, const POINT *point, PALCOLOUR *colour)
{
	//Check if this is in view bounds (if not, just return, no point in clogging the queue with stuff that will not be rendered)
	if (point->x < 0 || point->x >= width)
		return;
	if (point->y < 0 || point->y >= height)
		return;
	
	//Allocate our new queue entry
	RENDERQUEUE *newEntry = new RENDERQUEUE;
	newEntry->type = RENDERQUEUE_SOLID;
	newEntry->dest = {point->x, point->y, 1, 1};
	newEntry->solid.colour = colour;
	
	//Link to the queue
	newEntry->next = queue[layer];
	queue[layer] = newEntry;
}

void SOFTWAREBUFFER::DrawQuad(int layer, const RECT *quad, PALCOLOUR *colour)
{
	//Allocate our new queue entry
	RENDERQUEUE *newEntry = new RENDERQUEUE;
	newEntry->type = RENDERQUEUE_SOLID;
	newEntry->dest = *quad;
	
	//Clip top & left
	if (quad->x < 0)
	{
		newEntry->dest.x -= quad->x;
		newEntry->dest.w += quad->x;
	}
	
	if (quad->y < 0)
	{
		newEntry->dest.y -= quad->y;
		newEntry->dest.h += quad->y;
	}
	
	//Clip right and bottom
	if (newEntry->dest.x > (width - newEntry->dest.w))
		newEntry->dest.w -= newEntry->dest.x - (width - newEntry->dest.w);
	if (newEntry->dest.y > (height - newEntry->dest.h))
		newEntry->dest.h -= newEntry->dest.y - (height - newEntry->dest.h);
	
	//Quit if clipped off-screen
	if (newEntry->dest.w <= 0 || newEntry->dest.h <= 0)
		return;
	
	//Set colour reference
	newEntry->solid.colour = colour;
	
	//Link to the queue
	newEntry->next = queue[layer];
	queue[layer] = newEntry;
}

void SOFTWAREBUFFER::DrawTexture(TEXTURE *texture, PALETTE *palette, const RECT *src, int layer, int x, int y, bool xFlip, bool yFlip)
{
	//Get the source rect to use (nullptr = entire texture)
	RECT newSrc;
	if (src != nullptr)
		newSrc = *src;
	else
		newSrc = {0, 0, texture->width, texture->height};
	
	//Clip to the destination
	if (x < 0)
	{
		if (!xFlip)
			newSrc.x -= x;
		newSrc.w += x;
		x -= x;
	}
	
	int dx = x + newSrc.w - width;
	if (dx > 0)
	{
		if (xFlip)
			newSrc.x += dx;
		newSrc.w -= dx;
	}
	
	if (y < 0)
	{
		if (!yFlip)
			newSrc.y -= y;
		newSrc.h += y;
		y -= y;
	}
	
	int dy = y + newSrc.h - height;
	if (dy > 0)
	{
		if (yFlip)
			newSrc.y += dy;
		newSrc.h -= dy;
	}
	
	//Allocate our new queue entry
	RENDERQUEUE *newEntry = new RENDERQUEUE;
	newEntry->type = RENDERQUEUE_TEXTURE;
	newEntry->dest = {x, y, newSrc.w, newSrc.h};
	newEntry->texture.srcX = newSrc.x;
	newEntry->texture.srcY = newSrc.y;
	
	//Set palette and texture references
	newEntry->texture.palette = palette;
	newEntry->texture.texture = texture;
	
	//Flipping
	newEntry->texture.xFlip = xFlip;
	newEntry->texture.yFlip = yFlip;
	
	//Link to the queue
	newEntry->next = queue[layer];
	queue[layer] = newEntry;
}

void SOFTWAREBUFFER::DrawTexture(TEXTURE_FULLCOLOUR *texture, const RECT *src, int layer, int x, int y, bool xFlip, bool yFlip)
{
	//Get the source rect to use (nullptr = entire texture)
	RECT newSrc;
	if (src != nullptr)
		newSrc = *src;
	else
		newSrc = {0, 0, texture->width, texture->height};
	
	//Clip to the destination
	if (x < 0)
	{
		if (!xFlip)
			newSrc.x -= x;
		newSrc.w += x;
		x -= x;
	}
	
	int dx = x + newSrc.w - width;
	if (dx > 0)
	{
		if (xFlip)
			newSrc.x += dx;
		newSrc.w -= dx;
	}
	
	if (y < 0)
	{
		if (!yFlip)
			newSrc.y -= y;
		newSrc.h += y;
		y -= y;
	}
	
	int dy = y + newSrc.h - height;
	if (dy > 0)
	{
		if (yFlip)
			newSrc.y += dy;
		newSrc.h -= dy;
	}
	
	//Allocate our new queue entry
	RENDERQUEUE *newEntry = new RENDERQUEUE;
	newEntry->type = RENDERQUEUE_TEXTURE_FULLCOLOUR;
	newEntry->dest = {x, y, newSrc.w, newSrc.h};
	newEntry->textureFullColour.srcX = newSrc.x;
	newEntry->textureFullColour.srcY = newSrc.y;
	newEntry->textureFullColour.texture = texture;
	
	//Flipping
	newEntry->textureFullColour.xFlip = xFlip;
	newEntry->textureFullColour.yFlip = yFlip;
	
	//Link to the queue
	newEntry->next = queue[layer];
	queue[layer] = newEntry;
}

//Primary render function
bool SOFTWAREBUFFER::RenderToScreen(PALCOLOUR *backgroundColour)
{
	//Lock texture
	void *writeBuffer;
	int writePitch;
	if (SDL_LockTexture(texture, nullptr, &writeBuffer, &writePitch) < 0)
		return Error(SDL_GetError());
	
	//Render to our buffer
	const uint8_t bpp = nativeFormat->BytesPerPixel;
	
	switch (bpp)
	{
		case 1:
			Blit8 (backgroundColour,  (uint8_t*)writeBuffer, writePitch / 1);
			break;
		case 2:
			Blit16(backgroundColour, (uint16_t*)writeBuffer, writePitch / 2);
			break;
		case 4:
			Blit32(backgroundColour, (uint32_t*)writeBuffer, writePitch / 4);
			break;
		default:
			return Error("Unsupported BPP");
	}
	
	//Clear all layers
	for (int i = 0; i < RENDERLAYERS; i++)
	{
		//Unload all entries
		for (RENDERQUEUE *entry = queue[i]; entry != nullptr;)
		{
			RENDERQUEUE *next = entry->next;
			delete entry;
			entry = next;
		}
		
		//Reset queue
		queue[i] = nullptr;
	}
	
	//Unlock
	SDL_UnlockTexture(texture);
	
	//Copy to renderer (this is also where the scaling happens, incidentally)
	if (SDL_RenderCopy(renderer, texture, nullptr, nullptr) < 0)
		return Error(SDL_GetError());
	
	//Present renderer then wait for next frame (either use VSync if applicable, or just wait)
	if (vsyncMultiple != 0)
	{
		//Present renderer X amount of times, to call VSync that many times (hack-ish)
		for (unsigned int iteration = 0; iteration < vsyncMultiple; iteration++)
			SDL_RenderPresent(renderer);
	}
	else
	{
		//Present renderer, then wait for next frame
		SDL_RenderPresent(renderer);
		
		//Framerate limiter
		static long double timePrev;
		const uint32_t timeNow = SDL_GetTicks();
		const long double timeNext = timePrev + framerateMilliseconds;
		
		if (timeNow >= timePrev + 100)
		{
			timePrev = (long double)timeNow;
		}
		else
		{
			if (timeNow < timeNext)
				SDL_Delay(timeNext - timeNow);
			timePrev += framerateMilliseconds;
		}
	}
	
	return true;
}

//Sub-system
bool RefreshRenderer()
{
	//Destroy old renderer and software buffer
	if (gSoftwareBuffer)
		delete gSoftwareBuffer;
	if (renderer != nullptr)
		SDL_DestroyRenderer(renderer);
	
	//Create the renderer based off of our VSync settings
	if ((renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | (vsyncMultiple > 0 ? SDL_RENDERER_PRESENTVSYNC : 0))) == nullptr)
		return Error(SDL_GetError());
	
	//Create our software buffer
	gSoftwareBuffer = new SOFTWAREBUFFER(gRenderSpec.width, gRenderSpec.height);
	if (gSoftwareBuffer->fail)
		return Error(gSoftwareBuffer->fail);
	return true;
}

bool RenderCheckVSync()
{
	//Use display mode to detect VSync
	SDL_DisplayMode mode;
	if (SDL_GetWindowDisplayMode(window, &mode) < 0)
		return Error(SDL_GetError());
	
	long double refreshIntegral;
	long double refreshFractional = std::modf((long double)mode.refresh_rate / FRAMERATE, &refreshIntegral);
	
	if (refreshIntegral >= 1.0 && refreshFractional == 0.0)
		vsyncMultiple = (unsigned int)refreshIntegral;
	return true;
}

bool RefreshWindow()
{
	//Destroy old window
	if (window != nullptr)
		SDL_DestroyWindow(window);
	
	//Create our window
	if ((window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, gRenderSpec.width * gRenderSpec.scale, gRenderSpec.height * gRenderSpec.scale, 0)) == nullptr)
		return Error(SDL_GetError());
	
	//Check our vsync
	if (!RenderCheckVSync())
		return false;
	
	//Free old window pixel format and get new one
	if (nativeFormat != nullptr)
		SDL_FreeFormat(nativeFormat);
	if ((nativeFormat = SDL_AllocFormat(SDL_GetWindowPixelFormat(window))) == nullptr)
		return Error(SDL_GetError());
	
	//Regenerate our renderer and software buffer
	return RefreshRenderer();
}

bool InitializeRender()
{
	LOG(("Initializing renderer... "));
	
	//Create window, renderer, and software buffer
	if (!RefreshWindow())
		return false;
	
	LOG(("Success!\n"));
	return true;
}

void QuitRender()
{
	LOG(("Ending renderer... "));
	
	//Destroy window, renderer, and software buffer
	if (gSoftwareBuffer)
		delete gSoftwareBuffer;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_FreeFormat(nativeFormat);
	
	LOG(("Success!\n"));
}
