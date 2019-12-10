#include "GameConstants.h"
#include "Render.h"
#include "Log.h"
#include "Error.h"
#include "Filesystem.h"

//Render state and software buffer
RENDERSPEC gRenderSpec = {398, 224, 2};
SOFTWAREBUFFER *gSoftwareBuffer;

//VSync / framerate limiter
unsigned int vsyncMultiple = 0;
const long double framerateMilliseconds = (1000.0 / FRAMERATE);

//Texture class
TEXTURE::TEXTURE(std::string path)
{
	LOG(("Loading texture from %s... ", path.c_str()));
	loadedPalette = new PALETTE(0);
	LOG(("Success!\n"));
}

TEXTURE::~TEXTURE()
{
	return;
}

//Software buffer class
SOFTWAREBUFFER::SOFTWAREBUFFER(const int bufWidth, const int bufHeight)
{
	//Set our dimensions
	width = bufWidth;
	height = bufHeight;
}

SOFTWAREBUFFER::~SOFTWAREBUFFER()
{
	return;
}

//Drawing functions
void SOFTWAREBUFFER::DrawPoint(const int layer, const POINT *point, const COLOUR *colour)
{
	//Check if this is in view bounds (if not, just return, no point in clogging the queue with stuff that will not be rendered)
	if (point->x < 0 || point->x >= width)
		return;
	if (point->y < 0 || point->y >= height)
		return;
	
	//Setup our queue entry
	RENDERQUEUE newEntry;
	newEntry.type = RENDERQUEUE_SOLID;
	newEntry.dest = {point->x, point->y, 1, 1};
	
	//Set colour reference and link to queue
	newEntry.solid.colour = colour;
	queue[layer].link_front(newEntry);
}

void SOFTWAREBUFFER::DrawQuad(const int layer, const RECT *quad, const COLOUR *colour)
{
	//Setup our queue entry
	RENDERQUEUE newEntry;
	newEntry.type = RENDERQUEUE_SOLID;
	newEntry.dest = *quad;
	
	//Clip top & left
	if (quad->x < 0)
	{
		newEntry.dest.x -= quad->x;
		newEntry.dest.w += quad->x;
	}
	
	if (quad->y < 0)
	{
		newEntry.dest.y -= quad->y;
		newEntry.dest.h += quad->y;
	}
	
	//Clip right and bottom
	if (newEntry.dest.x > (width - newEntry.dest.w))
		newEntry.dest.w -= newEntry.dest.x - (width - newEntry.dest.w);
	if (newEntry.dest.y > (height - newEntry.dest.h))
		newEntry.dest.h -= newEntry.dest.y - (height - newEntry.dest.h);
	
	//Quit if clipped off-screen
	if (newEntry.dest.w <= 0 || newEntry.dest.h <= 0)
		return;
	
	//Set colour reference and link to queue
	newEntry.solid.colour = colour;
	queue[layer].link_front(newEntry);
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
	
	//Setup our queue entry
	RENDERQUEUE newEntry;
	newEntry.type = RENDERQUEUE_TEXTURE;
	newEntry.dest = {x, y, newSrc.w, newSrc.h};
	
	//Setup texture data
	newEntry.texture.srcX = newSrc.x;
	newEntry.texture.srcY = newSrc.y;
	newEntry.texture.palette = palette;
	newEntry.texture.texture = texture;
	newEntry.texture.xFlip = xFlip;
	newEntry.texture.yFlip = yFlip;
	
	//Link to the queue
	queue[layer].link_front(newEntry);
}

//Primary render function
bool SOFTWAREBUFFER::RenderToScreen(const COLOUR *backgroundColour)
{
	//Clear all layers
	for (size_t i = 0; i < RENDERLAYERS; i++)
		queue[i].clear();
	return false;
}

//Sub-system functions
bool InitializeRender()
{
	LOG(("Initializing renderer... "));
	
	//Create our software buffer
	gSoftwareBuffer = new SOFTWAREBUFFER(gRenderSpec.width, gRenderSpec.height);
	if (gSoftwareBuffer->fail)
		return Error(gSoftwareBuffer->fail);
	
	LOG(("Success!\n"));
	return false;
}

void QuitRender()
{
	LOG(("Ending renderer... "));
	
	//Destroy software buffer
	if (gSoftwareBuffer)
		delete gSoftwareBuffer;
	
	LOG(("Success!\n"));
}
