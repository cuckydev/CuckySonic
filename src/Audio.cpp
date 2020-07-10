#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "LinkedList.h"
#include "Audio.h"
#include "Filesystem.h"
#include "MathUtil.h"
#include "Log.h"
#include "Error.h"

//Playback constants
#define AUDIO_FREQUENCY 48000
#define AUDIO_SAMPLES	0x200
#define AUDIO_CHANNELS	2

//Sound definitions
SOUNDDEFINITION soundDefinition[SOUNDID_MAX] = {
	{0, nullptr, SOUNDID_NULL}, //SOUNDID_NULL
	{SOUNDCHANNEL_PSG0, "data/Audio/Sound/Jump.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,  "data/Audio/Sound/Roll.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_PSG1 | SOUNDCHANNEL_PSG2, "data/Audio/Sound/Skid.wav", SOUNDID_NULL},
	
	{0, nullptr, SOUNDID_NULL}, //SOUNDID_SPINDASH_REV
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev0.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev1.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev2.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev3.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev4.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev5.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev6.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev7.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev8.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev9.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev10.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/CDCharge.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/DropDash.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4 | SOUNDCHANNEL_PSG2,  "data/Audio/Sound/SpindashRelease.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/Hurt.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/SpikeHurt.wav", SOUNDID_NULL},
	
	{0,	"data/Audio/Sound/Ring.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,	nullptr, SOUNDID_RING}, //SOUNDID_RING_LEFT
	{SOUNDCHANNEL_FM4,	nullptr, SOUNDID_RING}, //SOUNDID_RING_RIGHT
	
	{SOUNDCHANNEL_FM3 | SOUNDCHANNEL_FM4,	"data/Audio/Sound/RingLoss.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4 | SOUNDCHANNEL_PSG2,	"data/Audio/Sound/Pop.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3 | SOUNDCHANNEL_FM4,	"data/Audio/Sound/GoalpostSpin.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,	"data/Audio/Sound/Spring.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM2 | SOUNDCHANNEL_FM3 | SOUNDCHANNEL_FM4 | SOUNDCHANNEL_PSG2, "data/Audio/Sound/Collapse.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4 | SOUNDCHANNEL_PSG2,	"data/Audio/Sound/WallSmash.wav", SOUNDID_NULL},
	
	{0,	nullptr, SOUNDID_NULL}, //SOUNDID_WATERFALL
	{SOUNDCHANNEL_FM3,	"data/Audio/Sound/Waterfall1.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,	"data/Audio/Sound/Waterfall2.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetBlueBarrier.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetFlameBarrier.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetLightningBarrier.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetAquaBarrier.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_PSG2,	"data/Audio/Sound/DoubleSpinAttack.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_PSG2,	"data/Audio/Sound/UseFlameBarrier.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/UseLightningBarrier.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/UseAquaBarrier.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4 | SOUNDCHANNEL_PSG1 | SOUNDCHANNEL_PSG2,	"data/Audio/Sound/SuperTransform.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_DAC,	"data/Audio/Sound/SplashJingle.wav", SOUNDID_NULL},
};

//Common audio functions
void PlaySound(SOUNDID id)
{
	
}

void StopSound(SOUNDID id)
{
	
}

void StopChannel(uint16_t channel)
{
	
}

//Load sound function
bool LoadAllSoundEffects()
{
	return false;
}

//Sub-system functions
bool InitializeAudio()
{
	LOG(("Initializing audio... "));
	LOG(("Success!\n"));
	return false;
}

void QuitAudio()
{
	LOG(("Ending audio... "));
	LOG(("Success!\n"));
	return;
}
