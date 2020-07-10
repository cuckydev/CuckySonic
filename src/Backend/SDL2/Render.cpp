#include "SDL_render.h"
#include "SDL_timer.h"
#include "../Render.h"
#include "../../GameConstants.h"

//Window and renderer
SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *outputTexture;

//Vsync and framerate
long double framerateMilliseconds;
unsigned int vsyncMultiple;

//Buffer and render output
bool Backend_GetOutputBuffer(void **buffer, int *pitch)
{
	//Lock texture
	if (SDL_LockTexture(outputTexture, nullptr, buffer, pitch) < 0)
		return true;
	return false;
}

bool Backend_OutputBuffer()
{
	//Unlock texture and draw to window
	SDL_UnlockTexture(outputTexture);
	if (SDL_RenderCopy(renderer, outputTexture, nullptr, nullptr) < 0)
		return true;
	
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
	return false;
}

//Core initialization and quitting
bool Backend_InitRender(RENDERSPEC renderSpec, BACKEND_RENDER_FORMAT *outRenderFormat)
{
	//Copy render specification's framerate
	framerateMilliseconds = (1000.0 / renderSpec.framerate);
	
	//Create window
	if ((window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, renderSpec.width * renderSpec.scale, renderSpec.height * renderSpec.scale, 0)) == nullptr)
		return true;
	
	//Use display mode to detect VSync
	SDL_DisplayMode mode;
	if (SDL_GetWindowDisplayMode(window, &mode) < 0)
		return true;
	
	long double refreshIntegral;
	long double refreshFractional = std::modf((long double)mode.refresh_rate / renderSpec.framerate, &refreshIntegral);
	vsyncMultiple = (unsigned int)refreshIntegral;
	
	//Check if vsync should be enabled
	if ((renderSpec.forceVsync && !renderSpec.forceVsyncValue) || !((renderSpec.forceVsync && renderSpec.forceVsyncValue) || (refreshIntegral >= 1.0 && refreshFractional == 0.0)))
		vsyncMultiple = 0;
	
	//Create renderer
	if ((renderer = SDL_CreateRenderer(window, -1, vsyncMultiple ? SDL_RENDERER_PRESENTVSYNC : 0)) == nullptr)
		return true;
	
	//Setup output render format
	uint32_t windowFormat = SDL_GetWindowPixelFormat(window);
	
	if (outRenderFormat != nullptr)
	{
		//Allocate our window pixel format
		SDL_PixelFormat *winFormat = SDL_AllocFormat(windowFormat);
		if (winFormat == nullptr)
			return true;
		
		//Convert to output pixel format
		outRenderFormat->pixelFormat.bitsPerPixel =		winFormat->BitsPerPixel;
		outRenderFormat->pixelFormat.bytesPerPixel =	winFormat->BytesPerPixel;
		outRenderFormat->pixelFormat.rMask =			winFormat->Rmask;
		outRenderFormat->pixelFormat.gMask =			winFormat->Gmask;
		outRenderFormat->pixelFormat.bMask =			winFormat->Bmask;
		outRenderFormat->pixelFormat.aMask =			winFormat->Amask;
		outRenderFormat->pixelFormat.rLoss =			winFormat->Rloss;
		outRenderFormat->pixelFormat.gLoss =			winFormat->Gloss;
		outRenderFormat->pixelFormat.bLoss =			winFormat->Bloss;
		outRenderFormat->pixelFormat.aLoss =			winFormat->Aloss;
		outRenderFormat->pixelFormat.rShift =			winFormat->Rshift;
		outRenderFormat->pixelFormat.gShift =			winFormat->Gshift;
		outRenderFormat->pixelFormat.bShift =			winFormat->Bshift;
		outRenderFormat->pixelFormat.aShift =			winFormat->Ashift;
		
		//Free allocated pixel format
		SDL_FreeFormat(winFormat);
	}
	
	//Create our output texture at the given width, height, and window format
	if ((outputTexture = SDL_CreateTexture(renderer, windowFormat, SDL_TEXTUREACCESS_STREAMING, renderSpec.width, renderSpec.height)) == nullptr)
		return true;
	return false;
}

void Backend_QuitRender()
{
	//Destroy window and renderer
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
}
