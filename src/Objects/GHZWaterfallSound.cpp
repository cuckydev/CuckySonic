#include "../Game.h"
#include "../Audio.h"

void ObjGHZWaterfallSound(OBJECT *object)
{
	//Play waterfall sound while on-screen every 64 frames
	if ((gLevel->frameCounter & 0x3F) == 0)
		PlaySound(SOUNDID_WATERFALL);
	object->UnloadOffscreen(object->x.pos);
}
