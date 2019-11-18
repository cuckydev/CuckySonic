#include <stdlib.h>
#include <string.h>
#include "CommonMacros.h"
#include "Filesystem.h"
#include "Mappings.h"
#include "Error.h"
#include "Log.h"

MAPPINGS::MAPPINGS(const char *path)
{
	LOG(("Loading mappings from %s... ", path));
	
	memset(this, 0, sizeof(MAPPINGS));
	
	//Open the file given
	source = DupePath(path);
	
	char *filepath = AllocPath(gBasePath, path, nullptr);
	BACKEND_FILE *fp = OpenFile(filepath, "rb");
	delete[] filepath;
	
	if (fp == nullptr)
	{
		Error(fail = GetFileError());
		return;
	}
	
	//Allocate our frames
	size = GetFileSize(fp) / (6 * 2);
	rect = new RECT[size];
	origin = new POINT[size];
	
	if (rect == nullptr || origin == nullptr)
	{
		CloseFile(fp);
		delete[] rect;
		delete[] origin;
		Error(fail = "Failed to allocate rect and origin arrays");
		return;
	}
	
	//Read from the file
	for (int i = 0; i < size; i++)
	{
		rect[i].x = ReadFile_BE16(fp);
		rect[i].y = ReadFile_BE16(fp);
		rect[i].w = ReadFile_BE16(fp);
		rect[i].h = ReadFile_BE16(fp);
		origin[i].x = (int16_t)ReadFile_BE16(fp);
		origin[i].y = (int16_t)ReadFile_BE16(fp);
	}
	
	CloseFile(fp);
	
	LOG(("Success!\n"));
}

MAPPINGS::~MAPPINGS()
{
	//Free allocated data
	delete[] rect;
	delete[] origin;
	delete[] source;
}
