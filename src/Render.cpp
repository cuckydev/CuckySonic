#include "SDL_render.h"
#include "SDL_timer.h"

#include "Render.h"
#include "Error.h"
#include "Log.h"
#include "Path.h"
#include "GameConstants.h"

//Window
SDL_Window *gWindow;
SDL_Renderer *gRenderer;
SOFTWAREBUFFER *gSoftwareBuffer;

//VSync / framerate limiter
unsigned int vsyncMultiple = 0;
const long double framerateMilliseconds = (1000.0 / FRAMERATE);

//Texture class
TEXTURE::TEXTURE(TEXTURE **linkedList, const char *path)
{
	LOG(("Loading texture from %s... ", path));
	memset(this, 0, sizeof(TEXTURE));
	
	//Load bitmap
	source = path;
	
	GET_GLOBAL_PATH(filepath, path);
	
	SDL_Surface *bitmap = SDL_LoadBMP(filepath);
	if (bitmap == NULL)
	{
		fail = SDL_GetError();
		return;
	}
	
	//Check if this format is valid
	if (bitmap->format->palette == NULL || bitmap->format->BytesPerPixel > 1)
	{
		SDL_FreeSurface(bitmap);
		fail = "Bitmap is not an 8-bit indexed .bmp";
		return;
	}
	
	//Create our palette using the image's palette
	if ((loadedPalette = new PALETTE) == NULL)
	{
		SDL_FreeSurface(bitmap);
		fail = "Failed to allocate palette for texture";
		return;
	}
	
	for (int i = 0; i < 0x100; i++)
	{
		if (bitmap->format->palette->ncolors)
			SetPaletteColour(&loadedPalette->colour[i], bitmap->format->palette->colors[i].r, bitmap->format->palette->colors[i].g, bitmap->format->palette->colors[i].b);
		else
			SetPaletteColour(&loadedPalette->colour[i], 0, 0, 0);
	}
	
	//Allocate texture data
	texture = (uint8_t*)malloc(bitmap->w * bitmap->h);
	textureXFlip = (uint8_t*)malloc(bitmap->w * bitmap->h);
	textureYFlip = (uint8_t*)malloc(bitmap->w * bitmap->h);
	textureXYFlip = (uint8_t*)malloc(bitmap->w * bitmap->h);
	
	if (!(texture && textureXFlip && textureYFlip && textureXYFlip))
	{
		SDL_FreeSurface(bitmap);
		
		free(texture); //This frees any buffers that successfully allocated
		free(textureXFlip);
		free(textureYFlip);
		free(textureXYFlip);
		
		fail = "Failed to allocate texture buffer";
		return;
	}
	
	//Copy our data
	for (int x = 0; x < bitmap->w; x++)
	{
		for (int y = 0; y < bitmap->h; y++)
		{
			texture[x + y * bitmap->w] = ((uint8_t*)bitmap->pixels)[x + y * bitmap->pitch];
			textureXFlip[x + y * bitmap->w] = ((uint8_t*)bitmap->pixels)[(bitmap->w - x - 1) + y * bitmap->pitch];
			textureYFlip[x + y * bitmap->w] = ((uint8_t*)bitmap->pixels)[x + (bitmap->h - y - 1) * bitmap->pitch];
			textureXYFlip[x + y * bitmap->w] = ((uint8_t*)bitmap->pixels)[(bitmap->w - x - 1) + (bitmap->h - y - 1) * bitmap->pitch];
		}
	}
	
	width = bitmap->w;
	height = bitmap->h;
	
	//Free bitmap surface
	SDL_FreeSurface(bitmap);
	
	//Attach to linked list if given
	if (linkedList != NULL)
	{
		list = linkedList;
		next = *linkedList;
		*linkedList = this;
	}
	
	LOG(("Success!\n"));
}

TEXTURE::TEXTURE(TEXTURE **linkedList, uint8_t *data, int dWidth, int dHeight)
{
	LOG(("Loading texture from memory location %p dimensions %dx%d... ", (void*)data, dWidth, dHeight));
	memset(this, 0, sizeof(TEXTURE));
	
	//Allocate texture data
	texture = (uint8_t*)malloc(dWidth * dHeight);
	textureXFlip = (uint8_t*)malloc(dWidth * dHeight);
	textureYFlip = (uint8_t*)malloc(dWidth * dHeight);
	textureXYFlip = (uint8_t*)malloc(dWidth * dHeight);
	
	if (!(texture && textureXFlip && textureYFlip && textureXYFlip))
	{
		free(texture); //This frees any buffers that successfully allocated
		free(textureXFlip);
		free(textureYFlip);
		free(textureXYFlip);
		
		fail = "Failed to allocate texture buffer";
		return;
	}
	
	//Copy our data
	for (int x = 0; x < dWidth; x++)
	{
		for (int y = 0; y < dHeight; y++)
		{
			texture[x + y * dWidth] = data[x + y * dWidth];
			textureXFlip[x + y * dWidth] = data[(dWidth - x - 1) + y * dWidth];
			textureYFlip[x + y * dWidth] = data[x + (dHeight - y - 1) * dWidth];
			textureXYFlip[x + y * dWidth] = data[(dWidth - x - 1) + (dHeight - y - 1) * dWidth];
		}
	}
	
	width = dWidth;
	height = dHeight;
	
	//Attach to linked list if given
	if (linkedList != NULL)
	{
		list = linkedList;
		next = *linkedList;
		*linkedList = this;
	}
	
	LOG(("Success!\n"));
}

TEXTURE::~TEXTURE()
{
	//Free data
	free(texture);
	free(textureXFlip);
	free(textureYFlip);
	free(textureXYFlip);
	
	if (loadedPalette)
		delete loadedPalette;
	
	//Remove from linked list if given
	if (list != NULL)
	{
		for (TEXTURE **texture = list; *texture != NULL; texture = &(*texture)->next)
		{
			if (*texture == this)
			{
				*texture = next;
				break;
			}
		}
	}
}

void TEXTURE::Draw(int layer, PALETTE *palette, SDL_Rect *src, int x, int y, bool xFlip, bool yFlip)
{
	//Check if this is in view bounds (if not, just return, no point in clogging the queue with stuff that will not be rendered)
	if (x <= -src->w || x >= gSoftwareBuffer->width)
		return;
	if (y <= -src->h || y >= gSoftwareBuffer->height)
		return;
	
	//Set the member of the render queue
	gSoftwareBuffer->queueEntry[layer]->type = RENDERQUEUE_TEXTURE;
	gSoftwareBuffer->queueEntry[layer]->dest = {x, y, src->w, src->h};
	gSoftwareBuffer->queueEntry[layer]->texture.srcX = src->x;
	gSoftwareBuffer->queueEntry[layer]->texture.srcY = src->y;
	
	//Clip top & left
	int clipL = 0, clipT = 0;
	
	if (x < 0)
	{
		clipL = -x;
		gSoftwareBuffer->queueEntry[layer]->dest.x += clipL;
		gSoftwareBuffer->queueEntry[layer]->dest.w -= clipL;
		gSoftwareBuffer->queueEntry[layer]->texture.srcX += clipL;
	}
	
	if (y < 0)
	{
		clipT = -y;
		gSoftwareBuffer->queueEntry[layer]->dest.y += clipT;
		gSoftwareBuffer->queueEntry[layer]->dest.h -= clipT;
		gSoftwareBuffer->queueEntry[layer]->texture.srcY += clipT;
	}
	
	//Clip right and bottom
	int clipR = 0, clipB = 0;
	
	if (gSoftwareBuffer->queueEntry[layer]->dest.x > (gSoftwareBuffer->width - gSoftwareBuffer->queueEntry[layer]->dest.w))
	{
		clipR = gSoftwareBuffer->queueEntry[layer]->dest.x - (gSoftwareBuffer->width - gSoftwareBuffer->queueEntry[layer]->dest.w);
		gSoftwareBuffer->queueEntry[layer]->dest.w -= clipR;
	}
	
	if (gSoftwareBuffer->queueEntry[layer]->dest.y > (gSoftwareBuffer->height - gSoftwareBuffer->queueEntry[layer]->dest.h))
	{
		clipB = gSoftwareBuffer->queueEntry[layer]->dest.y - (gSoftwareBuffer->height - gSoftwareBuffer->queueEntry[layer]->dest.h);
		gSoftwareBuffer->queueEntry[layer]->dest.h -= clipB;
	}
	
	//Set palette and texture references
	gSoftwareBuffer->queueEntry[layer]->texture.palette = palette;
	gSoftwareBuffer->queueEntry[layer]->texture.texture = this;
	
	//Flipping
	if (xFlip || yFlip)
	{
		if (xFlip && yFlip)
		{
			gSoftwareBuffer->queueEntry[layer]->texture.srcBuffer = textureXYFlip;
			gSoftwareBuffer->queueEntry[layer]->texture.srcX = width - (gSoftwareBuffer->queueEntry[layer]->texture.srcX + gSoftwareBuffer->queueEntry[layer]->dest.w) - (clipR - clipL);
			gSoftwareBuffer->queueEntry[layer]->texture.srcY = height - (gSoftwareBuffer->queueEntry[layer]->texture.srcY + gSoftwareBuffer->queueEntry[layer]->dest.h) - (clipB - clipT);
		}
		else if (xFlip)
		{
			gSoftwareBuffer->queueEntry[layer]->texture.srcBuffer = textureXFlip;
			gSoftwareBuffer->queueEntry[layer]->texture.srcX = width - (gSoftwareBuffer->queueEntry[layer]->texture.srcX + gSoftwareBuffer->queueEntry[layer]->dest.w) - (clipR - clipL);
		}
		else if (yFlip)
		{
			gSoftwareBuffer->queueEntry[layer]->texture.srcBuffer = textureYFlip;
			gSoftwareBuffer->queueEntry[layer]->texture.srcY = height - (gSoftwareBuffer->queueEntry[layer]->texture.srcY + gSoftwareBuffer->queueEntry[layer]->dest.h) - (clipB - clipT);
		}
	}
	else
	{
		gSoftwareBuffer->queueEntry[layer]->texture.srcBuffer = texture;
	}
	
	//Push forward in queue
	gSoftwareBuffer->queueEntry[layer]++;
}

//Palette functions
void SetPaletteColour(PALCOLOUR *palColour, uint8_t r, uint8_t g, uint8_t b)
{
	//Set formatted colour
	palColour->colour = gSoftwareBuffer->RGB(r, g, b);
	
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
	palColour->colour = gSoftwareBuffer->RGB(r, g, b);
	
	//Set colours
	palColour->r = r;
	palColour->g = g;
	palColour->b = b;
}

void RegenPaletteColour(PALCOLOUR *palColour)
{
	//Set formatted colour
	palColour->colour = gSoftwareBuffer->RGB(palColour->r, palColour->g, palColour->b);
}

//Software buffer class
SOFTWAREBUFFER::SOFTWAREBUFFER(uint32_t bufFormat, int bufWidth, int bufHeight)
{
	//Set our properties
	fail = NULL;
	
	format = SDL_AllocFormat(bufFormat);
	width = bufWidth;
	height = bufHeight;
	
	//Set our render queue position
	for (int i = 0; i < RENDERLAYERS; i++)
		queueEntry[i] = queue[i];
	
	//Allocate our framebuffer stuff
	if ((texture = SDL_CreateTexture(gRenderer, format->format, SDL_TEXTUREACCESS_STREAMING, width, height)) == NULL)
	{
		fail = SDL_GetError();
		return;
	}
}

SOFTWAREBUFFER::~SOFTWAREBUFFER()
{
	//Free our framebuffer stuff
	SDL_DestroyTexture(texture);
	SDL_FreeFormat(format);
}

//Colour format function
inline uint32_t SOFTWAREBUFFER::RGB(uint8_t r, uint8_t g, uint8_t b)
{
	return SDL_MapRGB(format, r, g, b);
}

//Drawing functions (no-class)
void SOFTWAREBUFFER::DrawPoint(int layer, int x, int y, PALCOLOUR *colour)
{
	//Check if this is in view bounds (if not, just return, no point in clogging the queue with stuff that will not be rendered)
	if (x < 0 || x >= width)
		return;
	if (y < 0 || y >= height)
		return;
	
	//Set the member of the render queue
	queueEntry[layer]->type = RENDERQUEUE_SOLID;
	queueEntry[layer]->dest = {x, y, 1, 1};
	
	//Set colour reference
	queueEntry[layer]->solid.colour = colour;
	
	//Push forward in queue
	queueEntry[layer]++;
}

void SOFTWAREBUFFER::DrawQuad(int layer, SDL_Rect *quad, PALCOLOUR *colour)
{
	//Check if this is in view bounds (if not, just return, no point in clogging the queue with stuff that will not be rendered)
	if (quad->x <= -quad->w || quad->x >= width)
		return;
	if (quad->y <= -quad->h || quad->y >= height)
		return;
	
	//Set the member of the render queue
	queueEntry[layer]->type = RENDERQUEUE_SOLID;
	queueEntry[layer]->dest = *quad;
	
	//Clip top & left
	if (quad->x < 0)
	{
		queueEntry[layer]->dest.x -= quad->x;
		queueEntry[layer]->dest.w += quad->x;
	}
	
	if (quad->y < 0)
	{
		queueEntry[layer]->dest.y -= quad->y;
		queueEntry[layer]->dest.h += quad->y;
	}
	
	//Clip right and bottom
	if (queueEntry[layer]->dest.x > (width - queueEntry[layer]->dest.w))
		queueEntry[layer]->dest.w -= queueEntry[layer]->dest.x - (width - queueEntry[layer]->dest.w);
	if (queueEntry[layer]->dest.y > (height - queueEntry[layer]->dest.h))
		queueEntry[layer]->dest.h -= queueEntry[layer]->dest.y - (height - queueEntry[layer]->dest.h);
	
	//Set colour reference
	queueEntry[layer]->solid.colour = colour;
	
	//Push forward in queue
	queueEntry[layer]++;
}

//Render the software buffer to the screen
#define SBRTSD(macro)	\
	if (backgroundColour != NULL)	\
	{	\
		for (int px = 0; px < width * height; px++)	\
			macro(writeBuffer, bpp, px, backgroundColour->colour);	\
	}	\
		\
	for (int i = 0; i < RENDERLAYERS; i++)	\
	{	\
		for (RENDERQUEUE *entry = queue[i]; entry != queueEntry[i]; entry++)	\
		{	\
			switch (entry->type)	\
			{	\
				case RENDERQUEUE_TEXTURE:	\
					fpx = entry->texture.srcX + entry->texture.srcY * entry->texture.texture->width;	\
					dpx = entry->dest.x + entry->dest.y * width;	\
						\
					for (int fy = entry->texture.srcY; fy < entry->texture.srcY + entry->dest.h; fy++)	\
					{	\
						for (int fx = entry->texture.srcX; fx < entry->texture.srcX + entry->dest.w; fx++)	\
						{	\
							const uint8_t index = entry->texture.srcBuffer[fpx++];	\
							if (index)	\
								macro(writeBuffer, bpp, dpx, entry->texture.palette->colour[index].colour);	\
							dpx++;	\
						}	\
						fpx += entry->texture.texture->width - entry->dest.w;	\
						dpx += width - entry->dest.w;	\
					}	\
					break;	\
				case RENDERQUEUE_SOLID:	\
					dpx = entry->dest.x + entry->dest.y * width;	\
						\
					for (int fy = entry->dest.y; fy < entry->dest.y + entry->dest.h; fy++)	\
					{	\
						for (int fx = entry->dest.x; fx < entry->dest.x + entry->dest.w; fx++)	\
						{	\
							macro(writeBuffer, bpp, dpx, entry->solid.colour->colour);	\
							dpx++;	\
						}	\
						dpx += width - entry->dest.w;	\
					}	\
					break;	\
				default:	\
					break;	\
			}	\
		}	\
			\
		queueEntry[i] = queue[i];	\
	}

bool SOFTWAREBUFFER::RenderToScreen(PALCOLOUR *backgroundColour)
{
	//Lock texture
	void *writeBuffer;
	int writePitch;
	if (SDL_LockTexture(texture, NULL, &writeBuffer, &writePitch) < 0)
		return Error(SDL_GetError());
	
	//Render to our buffer
	const uint8_t bpp = format->BytesPerPixel;
	int fpx, dpx;
	
	switch (bpp)
	{
		case 1:
			SBRTSD(SET_BUFFER_PIXEL1);
			break;
		case 2:
			SBRTSD(SET_BUFFER_PIXEL2);
			break;
		case 3:
			SBRTSD(SET_BUFFER_PIXEL3);
			break;
		case 4:
			//SBRTSD(SET_BUFFER_PIXEL4);
			if (backgroundColour != NULL)	\
	{	\
		for (int px = 0; px < width * height; px++)	\
			SET_BUFFER_PIXEL4(writeBuffer, bpp, px, backgroundColour->colour);	\
	}	\
		\
	for (int i = 0; i < RENDERLAYERS; i++)	\
	{	\
		for (RENDERQUEUE *entry = queue[i]; entry != queueEntry[i]; entry++)	\
		{	\
			switch (entry->type)	\
			{	\
				case RENDERQUEUE_TEXTURE:	\
					fpx = entry->texture.srcX + entry->texture.srcY * entry->texture.texture->width;	\
					dpx = entry->dest.x + entry->dest.y * width;	\
						\
					for (int fy = entry->texture.srcY; fy < entry->texture.srcY + entry->dest.h; fy++)	\
					{	\
						for (int fx = entry->texture.srcX; fx < entry->texture.srcX + entry->dest.w; fx++)	\
						{	\
							const uint8_t index = entry->texture.srcBuffer[fpx++];	\
							if (index)	\
								SET_BUFFER_PIXEL4(writeBuffer, bpp, dpx, entry->texture.palette->colour[index].colour);	\
							dpx++;	\
						}	\
						fpx += entry->texture.texture->width - entry->dest.w;	\
						dpx += width - entry->dest.w;	\
					}	\
					break;	\
				case RENDERQUEUE_SOLID:	\
					dpx = entry->dest.x + entry->dest.y * width;	\
						\
					for (int fy = entry->dest.y; fy < entry->dest.y + entry->dest.h; fy++)	\
					{	\
						for (int fx = entry->dest.x; fx < entry->dest.x + entry->dest.w; fx++)	\
						{	\
							SET_BUFFER_PIXEL4(writeBuffer, bpp, dpx, entry->solid.colour->colour);	\
							dpx++;	\
						}	\
						dpx += width - entry->dest.w;	\
					}	\
					break;	\
				default:	\
					break;	\
			}	\
		}	\
			\
		queueEntry[i] = queue[i];	\
	}
			break;
		default:
			break;
	}
	
	//Unlock
	SDL_UnlockTexture(texture);
	
	//Copy to renderer (this is also where the scaling happens, incidentally)
	if (SDL_RenderCopy(gRenderer, texture, NULL, NULL) < 0)
		return Error(SDL_GetError());
	
	//Present renderer then wait for next frame (either use VSync if applicable, or just wait)
	if (vsyncMultiple != 0)
	{
		//Present gRenderer X amount of times, to call VSync that many times (hack-ish)
		for (unsigned int iteration = 0; iteration < vsyncMultiple; iteration++)
			SDL_RenderPresent(gRenderer);
	}
	else
	{
		//Present gRenderer, then wait for next frame
		SDL_RenderPresent(gRenderer);
		
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
bool InitializeRender()
{
	LOG(("Initializing renderer... "));
	
	//Create our window
	if ((gWindow = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCREEN_SCALE, SCREEN_HEIGHT * SCREEN_SCALE, 0)) == NULL)
		return Error(SDL_GetError());
	
	//Use display mode to detect VSync
	SDL_DisplayMode mode;
	if (SDL_GetWindowDisplayMode(gWindow, &mode) < 0)
		Error(SDL_GetError());
	
	long double refreshIntegral;
	long double refreshFractional = std::modf((long double)mode.refresh_rate / FRAMERATE, &refreshIntegral);
	
	if (refreshIntegral >= 1.0 && refreshFractional == 0.0)
		vsyncMultiple = (unsigned int)refreshIntegral;
	
	//Create the renderer based off of our VSync settings
	if ((gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | (vsyncMultiple > 0 ? SDL_RENDERER_PRESENTVSYNC : 0))) == NULL)
		return Error(SDL_GetError());
	
	//Create our software buffer
	gSoftwareBuffer = new SOFTWAREBUFFER(SDL_GetWindowPixelFormat(gWindow), SCREEN_WIDTH, SCREEN_HEIGHT);
	if (gSoftwareBuffer->fail)
		return Error(gSoftwareBuffer->fail);
	
	LOG(("Success!\n"));
	return true;
}

void QuitRender()
{
	LOG(("Ending renderer... "));
	
	//Destroy window, renderer, and software framebuffer
	if (gSoftwareBuffer)
		delete gSoftwareBuffer;
	
	SDL_DestroyRenderer(gRenderer);	//no-op
	SDL_DestroyWindow(gWindow);
	
	LOG(("Success!\n"));
}
