#pragma once
#include <stdint.h>

#define AUDIO_FREQUENCY 44100
#define AUDIO_SAMPLES	0x200

//Music ids
enum MUSICID
{
	MUSICID_NULL,
	MUSICID_TITLE,
	MUSICID_MENU,
	MUSICID_GHZ,
	MUSICID_EHZ,
	MUSICID_MAX,
};

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

struct MUSICDEFINITION
{
	const char *path;
	int loopLength;	//How many samples the looping part lasts
	int loopEnd; //When the looping part ends
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

//Music class (using stb_vorbis backend)
#define STB_VORBIS_HEADER_ONLY
#include "Audio_stb_vorbis.cpp"

class MUSIC
{
	public:
		//Failure
		const char *fail;
		
		//stb_vorbis decoder
		stb_vorbis *file;
		int channels;
		
		//Song data
		MUSICDEFINITION *definition;
		bool playing;
		double internalPosition;
		
		float volume;
	
	public:
		MUSIC(MUSICDEFINITION *ourDefinition);
		~MUSIC();
		
		void Play(int position);
		int Pause();
		void SetVolume(float setVolume);
		float GetVolume();
		
		void Loop();
		void Mix(float *stream, int samples);
};

//Music functions
void PlayMusic(MUSICID music);
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
