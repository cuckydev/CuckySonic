#pragma once
#include <stdint.h>

#ifdef BACKEND_SDL2
	#include "SDL_audio.h"
	
	//Audio lock / unlock access
	extern SDL_AudioDeviceID gAudioDevice;
	#define AUDIO_LOCK		SDL_LockAudioDevice(gAudioDevice)
	#define AUDIO_UNLOCK	SDL_UnlockAudioDevice(gAudioDevice)
#endif

//Constant defines
#define AUDIO_FREQUENCY 48000
#define AUDIO_SAMPLES	0x200
#define AUDIO_CHANNELS	2

//Sound ids
enum SOUNDID
{
	SOUNDID_NULL,
	SOUNDID_JUMP,
	SOUNDID_ROLL,
	SOUNDID_SKID,
	
	SOUNDID_SPINDASH_REV,
	SOUNDID_SPINDASH_REV_0,
	SOUNDID_SPINDASH_REV_1,
	SOUNDID_SPINDASH_REV_2,
	SOUNDID_SPINDASH_REV_3,
	SOUNDID_SPINDASH_REV_4,
	SOUNDID_SPINDASH_REV_5,
	SOUNDID_SPINDASH_REV_6,
	SOUNDID_SPINDASH_REV_7,
	SOUNDID_SPINDASH_REV_8,
	SOUNDID_SPINDASH_REV_9,
	SOUNDID_SPINDASH_REV_10,
	
	SOUNDID_CD_CHARGE,
	SOUNDID_DROPDASH,
	SOUNDID_SPINDASH_RELEASE,
	
	SOUNDID_HURT,
	SOUNDID_SPIKE_HURT,
	
	SOUNDID_RING,
	SOUNDID_RING_LEFT,
	SOUNDID_RING_RIGHT,
	
	SOUNDID_RING_LOSS,
	
	SOUNDID_POP,
	SOUNDID_GOALPOST_SPIN,
	SOUNDID_SPRING,
	SOUNDID_WALL_SMASH,
	
	SOUNDID_WATERFALL,
	SOUNDID_WATERFALL_1,
	SOUNDID_WATERFALL_2,
	
	SOUNDID_GET_BLUE_SHIELD,
	SOUNDID_GET_FIRE_SHIELD,
	SOUNDID_GET_ELECTRIC_SHIELD,
	SOUNDID_GET_BUBBLE_SHIELD,
	
	SOUNDID_USE_INSTA_SHIELD,
	SOUNDID_USE_FIRE_SHIELD,
	SOUNDID_USE_ELECTRIC_SHIELD,
	SOUNDID_USE_BUBBLE_SHIELD,
	
	SOUNDID_SUPER_TRANSFORM,
	
	SOUNDID_SPLASHJINGLE,
	SOUNDID_MAX,
};

//Definitions
enum SOUNDCHANNEL
{
	SOUNDCHANNEL_PSG0	= 1 << 0,
	SOUNDCHANNEL_PSG1	= 1 << 1,
	SOUNDCHANNEL_PSG2	= 1 << 2,
	SOUNDCHANNEL_PSG3	= 1 << 3,
	SOUNDCHANNEL_FM0	= 1 << 4,
	SOUNDCHANNEL_FM1	= 1 << 5,
	SOUNDCHANNEL_FM2	= 1 << 6,
	SOUNDCHANNEL_FM3	= 1 << 7,
	SOUNDCHANNEL_FM4	= 1 << 8,
	SOUNDCHANNEL_FM5	= 1 << 9,
	SOUNDCHANNEL_DAC	= 1 << 10,
};

struct SOUNDDEFINITION
{
	uint16_t channel;
	const char *path;
	SOUNDID parent;
};

//Sound class
class SOUND
{
	public:
		//Failure
		const char *fail;
		
		//Our actual buffer data
		float *buffer;
		size_t size;
		
		//Current playback state
		bool playing;
		
		size_t sample;
		
		float volume;
		float volumeL;
		float volumeR;
		
		//Linked list and parent
		SOUND *next;
		SOUND *parent;
		
	public:
		SOUND(const char *path);
		SOUND(SOUND *ourParent);
		~SOUND();
		
		void Mix(float *stream, int frames);
};

//Music class (using stb_vorbis and miniaudio)
#define STB_VORBIS_HEADER_ONLY
#include "Audio_stb_vorbis.cpp"
#include "Audio_miniaudio.h"

ma_uint32 MusicReadSamples(ma_pcm_converter *dsp, void *buffer, ma_uint32 requestFrames, void *musicPointer);

class MUSIC
{
	public:
		//Failure
		const char *fail;
		
		//stb_vorbis decoder (music file) and other things about the music
		stb_vorbis *file;
		
		double frequency;
		unsigned int channels;
		int64_t loopStart;
		
		//miniaudio resampler
		ma_pcm_converter resampler;
		float *mixBuffer;
		
		//Music data
		const char *source;
		
		//Current playback state
		bool playing;
		float volume;
		
		//Linked list
		MUSIC *next;
	
	public:
		MUSIC(const char *name, int initialPosition, float initialVolume);
		~MUSIC();
		
		void PlayAtPosition(int setPosition);
		
		void Loop();
		ma_uint32 ReadSamplesToBuffer(float *buffer, int samples);
		void ReadAndMix(float *stream, int frames);
};

//Sound functions
void PlaySound(SOUNDID id);
void StopSound(SOUNDID id);
void StopChannel(uint16_t channel);

//Audio subsystem functions
extern bool gAudioYield;

void YieldAudio(bool yield);
bool InitializeAudio();
void QuitAudio();
