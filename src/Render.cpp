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
TEXTURE::TEXTURE(const char *path)
{
	fail = NULL;
	
	//Load bitmap
	GET_GLOBAL_PATH(filepath, path);
	
	LOG(("Loading texture from %s...\n", filepath));
	SDL_Surface *bitmap = SDL_LoadBMP(filepath);
	if (!bitmap)
	{
		fail = SDL_GetError();
		return;
	}
	
	//Check if this format is valid
	if (bitmap->format->palette == NULL || bitmap->format->BytesPerPixel > 1)
	{
		fail = "Bitmap is not an 8-bit indexed .bmp";
		return;
	}
	
	//Allocate texture data
	texture = (uint8_t*)malloc(bitmap->w * bitmap->h);
	
	//Copy our data
	memcpy(texture, bitmap->pixels, bitmap->w * bitmap->h);
	width = bitmap->w;
	height = bitmap->h;
	
	//Free bitmap surface
	SDL_FreeSurface(bitmap);
}

TEXTURE::~TEXTURE()
{
	//Free data
	free(texture);
}

void TEXTURE::Draw(PALETTE *palette, SDL_Rect *src, int x, int y)
{
	/*
	//Get soft buffer properties
	const void* buffer = gSoftwareBuffer->buffer;
	const uint8_t bpp = gSoftwareBuffer->format->BytesPerPixel;
	const int width = gSoftwareBuffer->width;
	const int height = gSoftwareBuffer->height;
	
	//Clip our rect
	SDL_Rect renderRect;
	renderRect.x = (x < 0) ? (src->x - x) : src->x;
	renderRect.y = (y < 0) ? (src->y - y) : src->y;
	renderRect.w = ((x + src->w) >= width) ? src->w - ((x + src->w) - width) : src->w;
	renderRect.h = ((y + src->h) >= height) ? src->h - ((y + src->h) - height) : src->h;
	
	const int right = renderRect.x + renderRect.w;
	const int bottom = renderRect.y + renderRect.h;
	
	//Render to the software buffer (TODO: do some optimization)
	for (int fx = renderRect.x; fx < right; fx++)
	{
		for (int fy = renderRect.y; fy < bottom; fy++)
		{
			int dx = x + (fx - src->x);
			int dy = y + (fy - src->y);
			
			const uint8_t index = texture[fx + fy * width];
			if (!index)
				continue;
			
			SET_BUFFER_PIXEL(buffer, bpp, dx + dy * width, palette->colour[index].colour);
		}
	}
	
	return true;
	*/
	
	//Check if this is in view bounds (if not, just return, no point in clogging the queue with stuff that will not be rendered)
	if (x <= -src->w || x >= gSoftwareBuffer->width)
		return;
	if (y <= -src->h || y >= gSoftwareBuffer->height)
		return;
	
	//Set the member of the render queue
	gSoftwareBuffer->queueEntry->type = RENDERQUEUE_TEXTURE;
	gSoftwareBuffer->queueEntry->dest = {x, y, src->w, src->h};
	gSoftwareBuffer->queueEntry->texture.srcX = src->x;
	gSoftwareBuffer->queueEntry->texture.srcY = src->y;
	gSoftwareBuffer->queueEntry->texture.palette = palette;
	gSoftwareBuffer->queueEntry->texture.texture = this;
	
	//Push forward in queue
	gSoftwareBuffer->queueEntry++;
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

//Software buffer class
SOFTWAREBUFFER::SOFTWAREBUFFER(uint32_t bufFormat, int bufWidth, int bufHeight)
{
	//Set our properties
	fail = NULL;
	
	format = SDL_AllocFormat(bufFormat);
	width = bufWidth;
	height = bufHeight;
	
	//Set our render queue position
	queueEntry = queue;
	
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

inline uint32_t SOFTWAREBUFFER::RGB(uint8_t r, uint8_t g, uint8_t b)
{
	return SDL_MapRGB(format, r, g, b);
}

#define SBRTSD(macro)	\
	for (int fx = 0; fx < width; fx++)	\
		for (int fy = 0; fy < height; fy++)	\
			macro(writeBuffer, bpp, fx + fy * width, backgroundColour->colour);	\
		\
	for (RENDERQUEUE *entry = queue; entry != queueEntry; entry++)	\
	{	\
		switch (entry->type)	\
		{	\
			case RENDERQUEUE_TEXTURE:	\
				renderRect.x = (entry->dest.x < 0) ? (entry->texture.srcX - entry->dest.x) : entry->texture.srcX;	\
				renderRect.y = (entry->dest.y < 0) ? (entry->texture.srcY - entry->dest.y) : entry->texture.srcY;	\
				renderRect.w = ((entry->dest.x + entry->dest.w) >= width) ? entry->dest.w - ((entry->dest.x + entry->dest.w) - width) : entry->dest.w;	\
				renderRect.h = ((entry->dest.y + entry->dest.h) >= height) ? entry->dest.h - ((entry->dest.y + entry->dest.h) - height) : entry->dest.h;	\
					\
				for (int fx = renderRect.x; fx < (renderRect.x + renderRect.w); fx++)	\
				{	\
					for (int fy = renderRect.y; fy < (renderRect.y + renderRect.h); fy++)	\
					{	\
						int dx = entry->dest.x + (fx - entry->texture.srcX);	\
						int dy = entry->dest.y + (fy - entry->texture.srcY);	\
							\
						const uint8_t index = entry->texture.texture->texture[fx + fy * entry->texture.texture->width];	\
						if (!index)	\
							continue;	\
							\
						macro(writeBuffer, bpp, dx + dy * width, entry->texture.palette->colour[index].colour);	\
					}	\
				}	\
				break;	\
			default:	\
				break;	\
		}	\
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
	SDL_Rect renderRect;
	
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
			SBRTSD(SET_BUFFER_PIXEL4);
			break;
		default:
			break;
	}
	
	//Reset our render queue
	queueEntry = queue;
	
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
	
	return false;
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
