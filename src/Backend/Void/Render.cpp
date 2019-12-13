#include "../Render.h"

//Buffer and render output
bool Backend_GetOutputBuffer(void **buffer, int *pitch)
{
	 *buffer = nullptr;
	 *pitch = 0;
	return false;
}

bool Backend_OutputBuffer()
{
	return false;
}

//Core initialization and quitting
bool Backend_InitRender(RENDERSPEC renderSpec, BACKEND_RENDER_FORMAT *outRenderFormat)
{
	return false;
}

void Backend_QuitRender()
{
	return;
}
