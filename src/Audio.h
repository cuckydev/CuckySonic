#pragma once
#include <stdint.h>

#define AUDIO_FREQUENCY 44100
#define AUDIO_SAMPLES	0x200

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
	SOUNDID_SPINDASH_RELEASE,
	
	SOUNDID_HURT,
	SOUNDID_SPIKE_HURT,
	
	SOUNDID_RING,
	SOUNDID_RING_LEFT,
	SOUNDID_RING_RIGHT,
	
	SOUNDID_RING_LOSS,
	
	SOUNDID_POP,
	SOUNDID_GOALPOST_SPIN,
	
	SOUNDID_SPLASHJINGLE,
	SOUNDID_MAX,
};

//Definitions
enum SOUNDCHANNEL
{
	SOUNDCHANNEL_NULL,
	SOUNDCHANNEL_PSG0,
	SOUNDCHANNEL_PSG1,
	SOUNDCHANNEL_PSG2,
	SOUNDCHANNEL_PSG3,
	SOUNDCHANNEL_FM0,
	SOUNDCHANNEL_FM1,
	SOUNDCHANNEL_FM2,
	SOUNDCHANNEL_FM3,
	SOUNDCHANNEL_FM4,
	SOUNDCHANNEL_FM5,
	SOUNDCHANNEL_DAC,
};

struct SOUNDDEFINITION
{
	SOUNDCHANNEL channel;
	const char *path;
	SOUNDID parent;
};

//Sound class
class SOUND
{
	public:
		const char *fail;
		
		//Our actual buffer data
		float *buffer;
		int size;
		
		//Playback position and frequency
		bool playing;
		
		double sample;
		double frequency;
		double baseFrequency;
		
		float volume;
		float volumeL;
		float volumeR;
		
		SOUND *next;
		SOUND *parent;
		
	public:
		SOUND(const char *path);
		SOUND(SOUND *ourParent);
		~SOUND();
		
		void Play();
		void Stop();
		
		void SetVolume(float setVolume);
		void SetPan(float setVolumeL, float setVolumeR);
		void SetFrequency(double setFrequency);
		
		void Mix(float *stream, int samples);
};

//Music class (using stb_vorbis and miniaudio)
#define STB_VORBIS_HEADER_ONLY
#include "Audio_stb_vorbis.cpp"
#include "Audio_miniaudio.h"

class MUSIC
{
	public:
		//Failure
		const char *fail;
		
		//stb_vorbis decoder
		stb_vorbis *file;
		double frequency;
		int channels;
		
		//miniaudio resampler
		ma_pcm_converter resampler;
		
		//Music data
		const char *source;
		
		int64_t loopStart;
		
		bool playing;
		int internalPosition;
		
		float volume;
	
	public:
		MUSIC(const char *name);
		~MUSIC();
		
		void Play(int position);
		int Pause();
		void SetVolume(float setVolume);
		float GetVolume();
		
		void ResetProperties();
		
		void Loop();
		ma_uint32 ReadSamplesToBuffer(float *buffer, int samples);
		void Mix(float *stream, int samples);
};

ma_uint32 MusicReadSamples(ma_pcm_converter *dsp, void *buffer, ma_uint32 requestFrames, void *musicPointer);

//Music functions
extern const char *gCurrentMusic;

void PlayMusic(const char *name);
int PauseMusic();
void ResumeMusic(int position);
void SetMusicVolume(float volume);
float GetMusicVolume();

//Sound functions
void PlaySound(SOUNDID id);
void StopSound(SOUNDID id);

//Audio subsystem functions
bool InitializeAudio();
void QuitAudio();
