#include "../Object.h"
#include "../Audio.h"
#include "../Game.h"
#include "../Level.h"
#include "../Render.h"

void ObjGHZWaterfallSound(OBJECT *object)
{
	//Play waterfall sound while on-screen every 64 frames
	if ((gLevel->frameCounter & 0x3F) == 0)
		PlaySound(SOUNDID_WATERFALL);
	object->UnloadOffscreen(object->x.pos);
}
