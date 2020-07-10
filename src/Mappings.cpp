#include <stdlib.h>
#include <string.h>
#include "CommonMacros.h"
#include "Filesystem.h"
#include "Mappings.h"
#include "Error.h"
#include "Log.h"

MAPPINGS::MAPPINGS(std::string path)
{
	LOG(("Loading mappings from %s... ", path.c_str()));
	
	//Open the file given
	FS_FILE fp(gBasePath + (source = path), "rb");
	if (fp.fail)
	{
		Error(fail = fp.fail);
		return;
	}
	
	//Allocate our frames
	size = fp.GetSize() / (6 * 2);
	rect = new RECT[size];
	origin = new POINT[size];
	
	if (rect == nullptr || origin == nullptr)
	{
		Error(fail = "Failed to allocate rect and origin arrays");
		delete[] rect;
		delete[] origin;
		return;
	}
	
	//Read from the file
	for (size_t i = 0; i < size; i++)
	{
		rect[i].x = fp.ReadBE16();
		rect[i].y = fp.ReadBE16();
		rect[i].w = fp.ReadBE16();
		rect[i].h = fp.ReadBE16();
		origin[i].x = (int16_t)fp.ReadBE16();
		origin[i].y = (int16_t)fp.ReadBE16();
	}
	
	LOG(("Success!\n"));
}

MAPPINGS::~MAPPINGS()
{
	//Free allocated data
	delete[] rect;
	delete[] origin;
}
