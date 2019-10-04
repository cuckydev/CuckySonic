#include "SDL_audio.h"
#include "SDL_timer.h"

#include "Audio.h"
#include "Log.h"
#include "Error.h"
#include "Path.h"

#include "MathUtil.h"

//The current audio device
SDL_AudioDeviceID gAudioDevice;

//Sound and music lists (for mixing and remembering what soundList/songs exist)
SOUND *soundList = nullptr;
MUSIC *musicList = nullptr;

//Sound effects
SOUND *sounds[SOUNDID_MAX];
SOUNDDEFINITION soundDefinition[SOUNDID_MAX] = {
	{SOUNDCHANNEL_NULL, nullptr, SOUNDID_NULL}, //SOUNDID_NULL
	{SOUNDCHANNEL_PSG0, "data/Audio/Sound/Jump.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,  "data/Audio/Sound/Roll.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_PSG1, "data/Audio/Sound/Skid.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_NULL, nullptr, SOUNDID_NULL}, //SOUNDID_SPINDASH_REV
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
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRelease.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/Hurt.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/SpikeHurt.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_NULL,	"data/Audio/Sound/Ring.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	nullptr, SOUNDID_RING}, //SOUNDID_RING_LEFT
	{SOUNDCHANNEL_FM3,	nullptr, SOUNDID_RING}, //SOUNDID_RING_RIGHT
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/RingLoss.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/Pop.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,	"data/Audio/Sound/GoalpostSpin.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,	"data/Audio/Sound/Spring.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetBlueShield.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetFireShield.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetElectricShield.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/GetBubbleShield.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_PSG2,	"data/Audio/Sound/UseInstaShield.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_PSG2,	"data/Audio/Sound/UseFireShield.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/UseElectricShield.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/UseBubbleShield.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_DAC,	"data/Audio/Sound/SplashJingle.wav", SOUNDID_NULL},
};

//Sound class
SOUND::SOUND(const char *path)
{
	//Clear class memory
	LOG(("Creating a sound from %s... ", path));
	memset(this, 0, sizeof(SOUND));
	
	//Read the file given
	GET_GLOBAL_PATH(filepath, path);
	
	SDL_AudioSpec wavSpec;
	uint8_t *wavBuffer;
	uint32_t wavLength;
	
	SDL_AudioSpec *audioSpec = SDL_LoadWAV(filepath, &wavSpec, &wavBuffer, &wavLength);
	
	if (audioSpec == nullptr)
	{
		Error(fail = SDL_GetError());
		SDL_UnlockAudioDevice(gAudioDevice);
		return;
	}
	
	//Build our audio CVT
	SDL_AudioCVT wavCVT;
	if (SDL_BuildAudioCVT(&wavCVT, wavSpec.format, wavSpec.channels, wavSpec.freq, AUDIO_F32, 2, wavSpec.freq) < 0)
	{
		Error(fail = SDL_GetError());
		SDL_FreeWAV(wavBuffer);
		SDL_UnlockAudioDevice(gAudioDevice);
		return;
	}
	
	//Set up our conversion
	if ((wavCVT.buf = (uint8_t*)malloc(wavLength * wavCVT.len_mult)) == nullptr)
	{
		Error(fail = "Failed to allocate our converted buffer");
		SDL_FreeWAV(wavBuffer);
		SDL_UnlockAudioDevice(gAudioDevice);
		return;
	}
	
	wavCVT.len = wavLength;
	memcpy(wavCVT.buf, wavBuffer, wavLength);

	//Free the original wave data
	SDL_FreeWAV(wavBuffer);

	//Convert our data, finally
	SDL_ConvertAudio(&wavCVT);
	
	//Use the data given
	buffer = (float*)wavCVT.buf;
	size = (wavLength * wavCVT.len_mult) / sizeof(float) / 2;
	
	//Initialize other properties
	sample = 0.0;
	frequency = wavSpec.freq;
	baseFrequency = frequency;
	volume = 1.0f;
	volumeL = 1.0f;
	volumeR = 1.0f;
	
	//Attach to the linked list
	next = soundList;
	soundList = this;
	
	LOG(("Success!\n"));
}

SOUND::SOUND(SOUND *ourParent)
{
	//Clear class memory
	LOG(("Creating a sound from parent %p... ", (void*)ourParent));
	memset(this, 0, sizeof(SOUND));
	
	//Use our data from the parent
	parent = ourParent;
	buffer = parent->buffer;
	size = parent->size;
	
	//Initialize other properties
	sample = 0.0;
	frequency = parent->frequency;
	baseFrequency = frequency;
	volume = 1.0f;
	volumeL = 1.0f;
	volumeR = 1.0f;
	
	//Attach to the linked list
	next = soundList;
	soundList = this;
	
	LOG(("Success!\n"));
}

SOUND::~SOUND()
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(gAudioDevice);
	
	//Remove from linked list
	for (SOUND **sound = &soundList; *sound != nullptr; sound = &(*sound)->next)
	{
		if (*sound == this)
		{
			*sound = next;
			break;
		}
	}
	
	//Free our data (do not free if we're a child of another sound)
	if (parent == nullptr)
		free(buffer);
	
	//Resume audio device
	SDL_UnlockAudioDevice(gAudioDevice);
}

//Mixer function
void SOUND::Mix(float *stream, int samples)
{
	//Don't mix if not playing
	if (!playing)
		return;
	
	//Mix this song into the buffer
	const double freqMove = frequency / AUDIO_FREQUENCY;
	for (int i = 0; i < samples; i++)
	{
		float *channelVolume = &volumeL;
		
		for (int channel = 0; channel < 2; channel++)
		{
			//Get the in-between sample this is (linear interpolation)
			const int intSample = std::floor(sample);
			const float sample1 = buffer[intSample * 2 + channel];
			const float sample2 = ((intSample + 1) >= size) ? 0 : buffer[intSample * 2 + channel + 1];
			
			//Interpolate sample
			const float subPos = (float)fmod(sample, 1.0);
			const float sampleOut = sample1 + (sample2 - sample1) * subPos;

			//Mix into buffer
			*stream++ += sampleOut * volume * (*channelVolume++);
		}

		//Increment position
		sample += freqMove;

		//Stop if reached end
		if (sample >= size)
		{
			playing = false;
			break;
		}
	}
}

//The miniaudio converter function
ma_uint32 MusicReadSamples(ma_pcm_converter *dsp, void *buffer, ma_uint32 requestFrames, void *musicPointer)
{
	//Read X amount of frames from the music (in the music's format) to be read and converted to our native format
	MUSIC *music = (MUSIC*)musicPointer;
	return music->ReadSamplesToBuffer((float*)buffer, requestFrames);
}

//Music class (using stb_vorbis and miniaudio)
#ifdef WINDOWS //Include Windows stuff, required for opening the files with UTF-16 paths
	#include <wchar.h>
	#include <stringapiset.h>
#endif

//Constructor and destructor
MUSIC::MUSIC(const char *name, int initialPosition, float initialVolume)
{
	//Clear memory
	LOG(("Loading music file from %s... ", name));
	memset(this, 0, sizeof(MUSIC));
	
	//Get our paths
	source = name;
	
	GET_GLOBAL_PATH(basePath, "data/Audio/Music/");
	GET_APPEND_PATH(musicPath, basePath, name);
	
	GET_APPEND_PATH(oggPath, musicPath, ".ogg");
	GET_APPEND_PATH(metaPath, musicPath, ".mmt");
	
	//Open the given file (different between Windows and non-Windows, because Windows hates UTF-8)
	#ifdef WINDOWS
		//Convert to UTF-16
		wchar_t wcharBuffer[0x200];
		MultiByteToWideChar(CP_UTF8, 0, oggPath, -1, wcharBuffer, 0x200);
		
		//Open the file with our newly converted path
		FILE *fp = _wfopen(wcharBuffer, L"rb");
	#else
		//Open file
		FILE *fp = fopen(oggPath, "rb");
	#endif
	
	//If the file failed to open...
	if (fp == nullptr)
	{
		char error[0x300];
		sprintf(error, "Failed to open %s\n", oggPath);
		Error(fail = error);
		return;
	}
	
	//Open our stb_vorbis file from this
	int error;
	file = stb_vorbis_open_file(fp, 1, &error, nullptr);
	
	if (file == nullptr)
	{
		char error[0x40];
		sprintf(error, "Error: %d", error);
		Warn(error);
		return;
	}
	
	//Get information from vorbis file
	stb_vorbis_info info = stb_vorbis_get_info(file);
	frequency = info.sample_rate;
	channels = info.channels;
	
	//Open meta file
	SDL_RWops *metafp = SDL_RWFromFile(metaPath, "rb");
	if (metafp == nullptr)
	{
		Error(fail = "Failed to open the meta file");
		stb_vorbis_close(file);
		return;
	}
	
	//Read meta data
	loopStart = (int64_t)SDL_ReadBE64(metafp);
	SDL_RWclose(metafp);
	
	//Initialize the resampler
	const ma_pcm_converter_config config = ma_pcm_converter_config_init(ma_format_f32, channels, frequency, ma_format_f32, 2, AUDIO_FREQUENCY, MusicReadSamples, this);
	ma_pcm_converter_init(&config, &resampler);
	
	//Attach to the linked list
	next = musicList;
	musicList = this;
	
	//Seek to given position and use given volume
	if (stb_vorbis_seek_frame(file, initialPosition) < 0)
		stb_vorbis_seek_frame(file, 0); //Seek to the beginning if failed
	volume = initialVolume;
	
	LOG(("Success!\n"));
}

MUSIC::~MUSIC()
{
	//Close our loaded file
	if (file != nullptr)
		stb_vorbis_close(file);
	
	//Remove from linked list
	for (MUSIC **music = &musicList; *music != nullptr; music = &(*music)->next)
	{
		if (*music == this)
		{
			*music = next;
			break;
		}
	}
	
	//Free mix buffer
	free(mixBuffer);
}

//Playback functions
void MUSIC::PlayAtPosition(int setPosition)
{
	//Play at the given position
	playing = true;
	
	//Seek to the given position
	if (stb_vorbis_seek_frame(file, setPosition) < 0)
		stb_vorbis_seek_frame(file, 0); //Seek to the beginning if failed
}

//Functions for reading from the music file
void MUSIC::Loop()
{
	//Seek back to definition->loopStart
	if (stb_vorbis_seek_frame(file, loopStart) < 0)
		stb_vorbis_seek_frame(file, 0); //Seek to the beginning if failed
}

ma_uint32 MUSIC::ReadSamplesToBuffer(float *buffer, int samples)
{
	//Read the given amount of samples
	float *bufferPointer = buffer;
	int samplesRead = 0;
	
	while (playing && samplesRead < samples)
	{
		//Read data and advance through buffer
		int thisRead = stb_vorbis_get_samples_float_interleaved(file, channels, bufferPointer, (samples - samplesRead) * channels) * channels;
		bufferPointer += thisRead;
		samplesRead += thisRead;
		
		//If reached the end of the file, either loop, or if there's no loop point, stop playing
		if (thisRead == 0)
		{
			if (loopStart < 0)
				playing = false;
			else
				Loop();
		}
	}
	
	//Fill the rest of the buffer with 0.0 (reached end of song)
	for (; samplesRead < samples * channels; samplesRead++)
		*bufferPointer++ = 0.0f;
	
	return samplesRead;
}

//Used for mixing into the audio buffer
void MUSIC::ReadAndMix(float *stream, int frames)
{
	//Don't mix if not playing
	if (!playing)
		return;
	
	//Allocate mix buffer if unallocated
	if (mixBuffer == nullptr)
		mixBuffer = (float*)malloc(frames * sizeof(float) * 2);
	
	//Read the samples and convert to native format using miniaudio
	ma_pcm_converter_read(&resampler, mixBuffer, frames);
	
	//Mix to buffer, applying volume
	float *buffer = stream;
	float *srcBuffer = mixBuffer;
	
	for (int i = 0; i < frames * 2; i++)
		*buffer++ += (*srcBuffer++) * volume;
}

//Callback function
bool gAudioYield = false;

void YieldAudio(bool yield)
{
	//Set our yield while the audio device is locked
	AUDIO_LOCK;
	gAudioYield = yield;
	AUDIO_UNLOCK;
}

void AudioCallback(void *userdata, uint8_t *stream, int length)
{
	//We aren't using userdata, this prevents an unused variable warning
	(void)userdata;
	
	//Get how many frames are in our buffer
	int frames = length / (2 * sizeof(float));
	
	//Clear the stream with 0.0f
	float *buffer = (float*)stream;
	for (int i = 0; i < frames * 2; i++)
		*buffer++ = 0.0f;
	
	//If window is unfocused, don't mix anything, pause audio
	if (gAudioYield)
		return;
	
	//Mix all loaded sounds and music into the buffer
	for (MUSIC *music = musicList; music != nullptr; music = music->next)
		music->ReadAndMix((float*)stream, frames);
	for (SOUND *sound = soundList; sound != nullptr; sound = sound->next)
		sound->Mix((float*)stream, frames);
}

//Play sound functions
bool ringPanLeft = false;

unsigned int spindashPitch = 0;	//Spindash's pitch increase in semi-tones
unsigned int spindashTimer = 0;	//Timer for the spindash pitch to reset
bool spindashLast = false;	//Set to 1 if spindash was the last sound, set to 0 if it wasn't (resets the spindash pitch)

void PlaySound(SOUNDID id)
{
	//Wait for audio device to be locked before modifying sound effect
	AUDIO_LOCK;
	
	//Play sound (there's specific sound stuff here though, such as ring panning, or spindash rev frequency)
	if (id == SOUNDID_SPINDASH_REV)
	{
		//Check spindash pitch clear
		if (!spindashLast || SDL_GetTicks() > spindashTimer)
		{
			spindashLast = true;
			spindashPitch = 0;
		}
		
		//Increment pitch
		if (++spindashPitch > 11)
			spindashPitch = 11;
		
		//Set sound id
		id = (SOUNDID)((unsigned int)SOUNDID_SPINDASH_REV + spindashPitch);
		
		//Update timer
		spindashTimer = SDL_GetTicks() + 1000;
	}
	else
	{
		//Clear spindash last
		spindashLast = false;
		
		//Ring sound
		if (id == SOUNDID_RING)
		{
			//Flip between left and right every time the sound plays
			ringPanLeft ^= 1;
			id = ringPanLeft ? SOUNDID_RING_LEFT : SOUNDID_RING_RIGHT;
			sounds[id]->volumeL = ringPanLeft ? 1.0f : 0.0f;
			sounds[id]->volumeR = ringPanLeft ? 0.0f : 1.0f;
		}
	}
	
	//Stop sounds of the same channel
	for (int i = 0; i < SOUNDID_MAX; i++)
		if (sounds[i] != nullptr && soundDefinition[i].channel == soundDefinition[id].channel)
			sounds[i]->playing = false;
	
	//Actually play our sound
	if (sounds[id] != nullptr)
	{
		sounds[id]->sample = 0.0;
		sounds[id]->playing = true;
	}
	
	//Unlock the audio buffer
	AUDIO_UNLOCK;
	return;
}

void StopSound(SOUNDID id)
{
	//Stop the given sound while the audio device is locked
	AUDIO_LOCK;
	if (sounds[id] != nullptr)
		sounds[id]->playing = false;
	AUDIO_UNLOCK;
	return;
}

//Load sound function
bool LoadAllSoundEffects()
{
	//Load all of the defined sound effects
	for (int i = 0; i < SOUNDID_MAX; i++)
	{
		if (soundDefinition[i].path != nullptr)
			sounds[i] = new SOUND(soundDefinition[i].path); //Load from path
		else if (soundDefinition[i].parent != SOUNDID_NULL)
			sounds[i] = new SOUND(sounds[soundDefinition[i].parent]); //Load from parent
		else
		{
			sounds[i] = nullptr;
			continue;
		}
		
		if (sounds[i]->fail)
			return false;
	}
	
	return true;
}

//Subsystem functions
bool InitializeAudio()
{
	LOG(("Initializing audio...\n"));
	
	//Open our audio device
	SDL_AudioSpec want;
	want.freq = AUDIO_FREQUENCY;
	want.samples = AUDIO_SAMPLES;
	want.format = AUDIO_F32;
	want.channels = 2;
	want.callback = AudioCallback;
	
	gAudioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
	if (!gAudioDevice)
		return Error(SDL_GetError());
	
	//Load all of our sound effects
	if (!LoadAllSoundEffects())
		return false;
	
	//Allow our audio device to play audio
	SDL_PauseAudioDevice(gAudioDevice, 0);
	LOG(("Success!\n"));
	return true;
}

void QuitAudio()
{
	LOG(("Ending audio... "));
	
	//Lock the audio device, and unload all of our sounds and music
	AUDIO_LOCK;
	
	for (MUSIC *music = musicList; music != nullptr;)
	{
		MUSIC *next = music->next;
		delete music;
		music = next;
	}
	
	for (SOUND *sound = soundList; sound != nullptr;)
	{
		SOUND *next = sound->next;
		delete sound;
		sound = next;
	}
	
	//Close our audio device
	AUDIO_UNLOCK;
	SDL_CloseAudioDevice(gAudioDevice);
	
	LOG(("Success!\n"));
	return;
}
