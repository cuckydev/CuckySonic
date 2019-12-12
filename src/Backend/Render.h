#pragma once
#include "../Render.h"

//Backend render format
struct BACKEND_RENDER_FORMAT
{
	PIXELFORMAT pixelFormat; //Output pixel format
};

//Render functions
bool Backend_GetOutputBuffer(void **buffer, int *pitch);
bool Backend_OutputBuffer();

bool Backend_InitRender(RENDERSPEC renderSpec, BACKEND_RENDER_FORMAT *outRenderFormat);
void Backend_QuitRender();
