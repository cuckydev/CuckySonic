#include "../Object.h"
#include "../Audio.h"
#include "../Game.h"
#include "../Level.h"
#include "../Render.h"

void ObjGHZWaterfallSound(OBJECT *object)
{
	//Play waterfall sound while on-screen every 64 frames
	int16_t xDiff = object->x.pos - gLevel->camera->x;
	int16_t yDiff = object->y.pos - gLevel->camera->y;
	
	if (xDiff >= 0 && xDiff < gRenderSpec.width && yDiff >= 0 && yDiff < gRenderSpec.height && (gLevel->frameCounter & 0x3F) == 0)
		PlaySound(SOUNDID_WATERFALL);
}
