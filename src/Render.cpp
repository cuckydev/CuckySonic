#include "Backend/Render.h"
#include "Render.h"
#include "GameConstants.h"
#include "Log.h"
#include "Error.h"
#include "Filesystem.h"

//Render specification
RENDERSPEC gRenderSpec = {426, 240, 2, 60.001, false, false};

SOFTWAREBUFFER *gSoftwareBuffer;

//Render format
PIXELFORMAT gPixelFormat;

//Bitmap constants / enumerations
enum BMPCMP
{
	BMPCMP_RGB,
	BMPCMP_RLE8,
	BMPCMP_RLE4,
	BMPCMP_BITFIELDS,
};

//Texture class
TEXTURE::TEXTURE(std::string path)
{
	LOG(("Loading texture from %s... ", path.c_str()));
	
	//Open our given file file
	FS_FILE fp(gBasePath + (source = path), "rb");
	if (fp.fail)
	{
		Error(fail = fp.fail);
		return;
	}
	
	//Read file as .BMP - Header
	uint16_t signature = fp.ReadBE16();
		fp.ReadLE32(); //UNUSED - filesize
		fp.ReadLE32(); //RESERVED - 4 bytes of reserved space
	uint32_t pixelDataPointer = fp.ReadLE32();
	
	//Validate header
	if (signature != 0x424D)
	{
		Error(fail = "Not a .bmp (invalid header)");
		return;
	}
	
	//Read file as .BMP  - Info Header
	uint32_t infHeaderSize = fp.ReadLE32();
	BMPCMP bitmapCompression;
	uint16_t bitmapBPP;
	uint32_t bitmapColours;
	
	if (infHeaderSize == 12) //Deprecated BMP format - BITMAPCOREHEADER
	{
		//Unsupported (no indexed support, at all)
		Error(fail = "Unsupported .bmp type");
		return;
	}
	else if (infHeaderSize >= 40) //BITMAPINFOHEADER
	{
		width = (int32_t)fp.ReadLE32();
		height = (int32_t)fp.ReadLE32();
			fp.ReadLE16(); //UNUSED - Planes
		bitmapBPP = fp.ReadLE16();
		bitmapCompression = (BMPCMP)fp.ReadLE32();
			fp.ReadLE32(); //UNUSED - Size of the image in bytes.
			fp.ReadLE32(); //UNUSED - Something with DPI?
			fp.ReadLE32(); //UNUSED - Something with DPI?
		bitmapColours = fp.ReadLE32();
			fp.ReadLE32(); //UNUSED - "Important" colours
	}
	else
	{
		//Invalid / unknown info header size?
		Error(fail = "Invalid info header size");
		return;
	}
	
	//Handle width and height stuff
	if (width <= 0 || height == 0)
	{
		Error(fail = "Invalid dimensions");
		return;
	}
	
	//Get if our bitmap is top to down or not
	bool bitmapIsTopDown;
	if (height < 0)
	{
		//Negative - bitmap is top to down
		bitmapIsTopDown = true;
		height = -height;
	}
	else
	{
		//Positive - bitmap is not top to down
		bitmapIsTopDown = false;
	}
	
	//Skip unused header data
	fp.Seek(pixelDataPointer - (bitmapColours * 4), SEEK_SET);
	
	//Pad our width to bmp data's pitch
	if (width & 0x3)
		width += (4 - (width & 0x3));
	
	//Verify that image is indexed and uncompressed
	if (bitmapCompression == BMPCMP_RGB && bitmapColours > 0)
	{
		//Read our palette
		loadedPalette = new PALETTE(bitmapColours);
		for (uint32_t i = 0; i < bitmapColours; i++)
		{
			//Read our colours
			uint8_t r, g, b;
			b = fp.ReadU8();	//Colours are read as BGR
			g = fp.ReadU8();
			r = fp.ReadU8();
				fp.ReadU8();	//RESERVED - ???
			
			//Copy to the loaded palette
			loadedPalette->colour[i].SetColour(true, true, true, r, g, b);
		}
		
		//Allocate and read texture data
		texture = new uint8_t[width * height];
		uint8_t *txPnt = texture + (bitmapIsTopDown ? 0 : (height - 1) * width);
		
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width;)
			{
				uint8_t src = fp.ReadU8();
				switch (bitmapBPP)
				{
					case 1:
						*txPnt++ = (src & 0x80) >> 7;
						*txPnt++ = (src & 0x40) >> 6;
						*txPnt++ = (src & 0x20) >> 5;
						*txPnt++ = (src & 0x10) >> 4;
						*txPnt++ = (src & 0x08) >> 3;
						*txPnt++ = (src & 0x04) >> 2;
						*txPnt++ = (src & 0x02) >> 1;
						*txPnt++ = (src & 0x01) >> 0;
						x += 8;
						break;
					case 4:
						*txPnt++ = (src & 0xF0) >> 4;
						*txPnt++ = (src & 0x0F) >> 0;
						x += 2;
						break;
					case 8:
						*txPnt++ = src;
						x++;
						break;
					default:
						Error(fail = "Invalid bit depth");
						return;
				}
			}
			
			if (!bitmapIsTopDown)
				txPnt -= width * 2;
		}
	}
	else
	{
		Error(fail = "Bitmap is not indexed and uncompressed");
		return;
	}
	
	LOG(("Success!\n"));
}

TEXTURE::~TEXTURE()
{
	//Unload texture data
	delete[] texture;
}

//Software buffer class
SOFTWAREBUFFER::SOFTWAREBUFFER(const int bufWidth, const int bufHeight)
{
	//Set our dimensions
	width = bufWidth;
	height = bufHeight;
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
	newEntry.solid.colour = colour;
	
	//Link to queue
	queue[layer].link_front(newEntry);
}

void SOFTWAREBUFFER::DrawQuad(const int layer, const RECT *quad, const COLOUR *colour)
{
	//Don't draw bad quads
	if (quad->w <= 0 || quad->h <= 0)
		return;
	
	//Setup our queue entry
	RENDERQUEUE newEntry;
	newEntry.dest = *quad;
	
	//Clip top and left
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
	const int right = (width - newEntry.dest.w);
	const int bottom = (height - newEntry.dest.h);
	if (newEntry.dest.x > right)
		newEntry.dest.w -= newEntry.dest.x - right;
	if (newEntry.dest.y > bottom)
		newEntry.dest.h -= newEntry.dest.y - bottom;
	
	//Quit if clipped off-screen
	if (newEntry.dest.w <= 0 || newEntry.dest.h <= 0)
		return;
	
	//Finish setting up entry and link to queue
	newEntry.type = RENDERQUEUE_SOLID;
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
	
	//Don't draw bad quads
	if (newSrc.w <= 0 || newSrc.h <= 0)
		return;
	
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
	newEntry.texture.srcX = newSrc.x;
	newEntry.texture.srcY = newSrc.y;
	newEntry.texture.palette = palette;
	newEntry.texture.texture = texture;
	newEntry.texture.xFlip = xFlip;
	newEntry.texture.yFlip = yFlip;
	
	//Link to queue
	queue[layer].link_front(newEntry);
}

//Primary render function
bool SOFTWAREBUFFER::RenderToScreen(const COLOUR *backgroundColour)
{
	//Get our buffer to render to
	void *outBuffer;
	int outPitch;
	if (Backend_GetOutputBuffer(&outBuffer, &outPitch))
		return true;
	
	if (outBuffer != nullptr)
	{
		//Render to our buffer
		switch (gPixelFormat.bytesPerPixel)
		{
			case 1:
				BlitQueue<uint8_t>(backgroundColour,  (uint8_t*)outBuffer, outPitch / 1);
				break;
			case 2:
				BlitQueue<uint16_t>(backgroundColour, (uint16_t*)outBuffer, outPitch / 2);
				break;
		#ifdef uint24_t //If the compiler supports 24-bit integers, then I mean, I guess
			case 3:
				BlitQueue<uint24_t>(backgroundColour, (uint24_t*)outBuffer, outPitch / 3);
				break;
		#endif
			case 4:
				BlitQueue<uint32_t>(backgroundColour, (uint32_t*)outBuffer, outPitch / 4);
				break;
			default:
				return Error("Unsupported BPP");
		}
	}
	
	//Clear all layers
	for (size_t i = 0; i < RENDERLAYERS; i++)
		queue[i].clear();
	
	//Render buffer to output
	if (Backend_OutputBuffer())
		return true;
	return false;
}

//Sub-system functions
bool InitializeRender()
{
	LOG(("Initializing renderer... "));
	
	//Initialize backend rendering
	BACKEND_RENDER_FORMAT backendRenderFormat;
	if (Backend_InitRender(gRenderSpec, &backendRenderFormat))
		return true;
	
	//Set our format globals
	gPixelFormat = backendRenderFormat.pixelFormat;
	
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
