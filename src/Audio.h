#pragma once
#include <stdint.h>

#define AUDIO_FREQUENCY 44100
#define AUDIO_SAMPLES	0x200

//Music ids
enum MUSICID
{
	MUSICID_GHZ,
	MUSICID_MAX,
};

//Sound ids
enum SOUNDID
{
	SOUNDID_JUMP,
	SOUNDID_ROLL,
	SOUNDID_SKID,
	SOUNDID_SPINDASH_REV,
	SOUNDID_SPINDASH_RELEASE,
	SOUNDID_DEATH,
	SOUNDID_MAX,
};

//Definitions
enum SOUNDCHANNEL
{
	SOUNDCHANNEL_PSG1,
	SOUNDCHANNEL_PSG2,
	SOUNDCHANNEL_PSG3,
	SOUNDCHANNEL_PSG4,
	SOUNDCHANNEL_FM1,
	SOUNDCHANNEL_FM2,
	SOUNDCHANNEL_FM3,
	SOUNDCHANNEL_FM4,
	SOUNDCHANNEL_FM5,
	SOUNDCHANNEL_FM6,
};

struct MUSICDEFINITION
{
	const char *path;
	size_t loopSample;	//Frames for the loop point, in-case the song has an intro, this can be 0
};

struct SOUNDDEFINITION
{
	SOUNDCHANNEL channel;
	const char *path;
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
		
		float volume;
		float volumeL;
		float volumeR;
		
		SOUND *next;
		
	public:
		SOUND(const char *path);
		~SOUND();
		
		void Play();
		void Stop();
		void Mix(float *stream, int samples);
};

//Audio functions
void PlaySound(SOUNDID id);

bool InitializeAudio();
void QuitAudio();
