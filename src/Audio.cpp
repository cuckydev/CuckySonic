#include "SDL_audio.h"
#include "SDL_timer.h"

#include "Audio.h"
#include "Log.h"
#include "Error.h"
#include "Path.h"

#include "MathUtil.h"

SDL_AudioDeviceID audioDevice;

//Sound effects
SOUND *soundEffects[SOUNDID_MAX];
SOUNDDEFINITION soundDefinition[SOUNDID_MAX] = {
	{SOUNDCHANNEL_NULL, NULL, SOUNDID_NULL}, //SOUNDID_NULL
	{SOUNDCHANNEL_PSG0, "data/Audio/Sound/Jump.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,  "data/Audio/Sound/Roll.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_PSG1, "data/Audio/Sound/Skid.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_NULL, NULL, SOUNDID_NULL}, //SOUNDID_SPINDASH_REV
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
	{SOUNDCHANNEL_FM4,	NULL, SOUNDID_RING}, //SOUNDID_RING_LEFT
	{SOUNDCHANNEL_FM3,	NULL, SOUNDID_RING}, //SOUNDID_RING_RIGHT
	
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/RingLoss.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_FM4, "data/Audio/Sound/Pop.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3, "data/Audio/Sound/GoalpostSpin.wav", SOUNDID_NULL},
	
	{SOUNDCHANNEL_DAC,	"data/Audio/Sound/SplashJingle.wav", SOUNDID_NULL},
};

//Sound class
SOUND *sounds = NULL;

SOUND::SOUND(const char *path)
{
	LOG(("Creating a sound from %s... ", path));
	
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Clear class memory
	memset(this, 0, sizeof(SOUND));
	
	//Read the file given
	GET_GLOBAL_PATH(filepath, path);
	
	SDL_AudioSpec wavSpec;
	uint8_t *wavBuffer;
	uint32_t wavLength;
	
	SDL_AudioSpec *audioSpec = SDL_LoadWAV(filepath, &wavSpec, &wavBuffer, &wavLength);
	
	if (audioSpec == NULL)
	{
		Error(fail = SDL_GetError());
		SDL_UnlockAudioDevice(audioDevice);
		return;
	}
	
	//Build our audio CVT
	SDL_AudioCVT wavCVT;
	if (SDL_BuildAudioCVT(&wavCVT, wavSpec.format, wavSpec.channels, wavSpec.freq, AUDIO_F32, 2, wavSpec.freq) < 0)
	{
		Error(fail = SDL_GetError());
		SDL_FreeWAV(wavBuffer);
		SDL_UnlockAudioDevice(audioDevice);
		return;
	}
	
	//Set up our conversion
	if ((wavCVT.buf = (uint8_t*)malloc(wavLength * wavCVT.len_mult)) == NULL)
	{
		Error(fail = "Failed to allocate our converted buffer");
		SDL_FreeWAV(wavBuffer);
		SDL_UnlockAudioDevice(audioDevice);
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
	next = sounds;
	sounds = this;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
	
	LOG(("Success!\n"));
}

SOUND::SOUND(SOUND *ourParent)
{
	LOG(("Creating a sound from parent %p... ", (void*)ourParent));
	
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Clear class memory
	memset(this, 0, sizeof(SOUND));
	
	//Use our data from the parent
	parent = ourParent;
	
	buffer = ourParent->buffer;
	size = ourParent->size;
	
	//Initialize other properties
	sample = 0.0;
	frequency = ourParent->frequency;
	baseFrequency = frequency;
	volume = 1.0f;
	volumeL = 1.0f;
	volumeR = 1.0f;
	
	//Attach to the linked list
	next = sounds;
	sounds = this;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
	
	LOG(("Success!\n"));
}

SOUND::~SOUND()
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Remove from linked list
	for (SOUND **sound = &sounds; *sound != NULL; sound = &(*sound)->next)
	{
		if (*sound == this)
		{
			*sound = next;
			break;
		}
	}
	
	//Free our data (do not free if we're a child of another sound)
	if (parent == NULL)
		free(buffer);
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

void SOUND::Play()
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Start playing
	sample = 0.0;
	playing = true;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

void SOUND::Stop()
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Stop playing
	playing = false;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

void SOUND::SetVolume(float setVolume)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Set our volume to the volume given
	volume = setVolume;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

void SOUND::SetPan(float setVolumeL, float setVolumeR)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Set our volume to the volume given
	volumeL = setVolumeL;
	volumeR = setVolumeR;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

void SOUND::SetFrequency(double setFrequency)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Set our frequency to the frequency given
	frequency = setFrequency;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

void SOUND::Mix(float *stream, int samples)
{
	if (!playing)
		return;
	
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

			//Mix
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

//Music class (using stb_vorbis and miniaudio)
#ifdef WINDOWS //Include Windows stuff, required for opening the files with UTF-16 paths
	#include <wchar.h>
	#include <stringapiset.h>
#endif

//Constructor and destructor
MUSIC::MUSIC(const char *name)
{
	memset(this, 0, sizeof(MUSIC));
	
	source = name;
	LOG(("Loading music file from %s... ", name));
	
	//Get our paths
	GET_GLOBAL_PATH(basePath, "data/Audio/Music/");
	GET_APPEND_PATH(musicPath, basePath, name);
	
	GET_APPEND_PATH(oggPath, musicPath, ".ogg");
	GET_APPEND_PATH(metaPath, musicPath, ".mmt");
	
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
	if (fp == NULL)
	{
		Error(fail = "Failed to open the given file");
		return;
	}
	
	//Open our stb_vorbis file from this
	int error;
	file = stb_vorbis_open_file(fp, 1, &error, NULL);
	
	if (file == NULL)
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
	
	//Read metadata file
	SDL_RWops *metafp = SDL_RWFromFile(metaPath, "rb");
	if (metafp == NULL)
	{
		Error(fail = "Failed to open the meta file");
		stb_vorbis_close(file);
		return;
	}
	
	loopStart = (int64_t)SDL_ReadBE64(metafp);
	
	SDL_RWclose(metafp);
	
	//Initialize the resampler
	const ma_pcm_converter_config config = ma_pcm_converter_config_init(ma_format_f32, channels, frequency, ma_format_f32, 2, AUDIO_FREQUENCY, MusicReadSamples, this);
	ma_pcm_converter_init(&config, &resampler);
	
	LOG(("Success!\n"));
}

MUSIC::~MUSIC()
{
	//Close our loaded file
	if (file != NULL)
		stb_vorbis_close(file);
}

//Playback functions
void MUSIC::Play(int position)
{
	//Play at the given position
	internalPosition = position;
	playing = true;
	
	//Seek within the given file
	if (stb_vorbis_seek_frame(file, position) < 0)
		stb_vorbis_seek_frame(file, 0); //Seek to the beginning if failed
}

int MUSIC::Pause()
{
	//Pause and get the position we're at
	playing = false;
	return internalPosition;
}

//Mixer functions
void MUSIC::Loop()
{
	//Seek back to definition->loopStart
	if (stb_vorbis_seek_frame(file, (int)(internalPosition = loopStart)) < 0)
		stb_vorbis_seek_frame(file, 0); //Seek to the beginning if failed
}

ma_uint32 MUSIC::ReadSamplesToBuffer(float *buffer, int samples)
{
	//Read the given amount of samples
	float *bufferPointer = &(*buffer);
	int samplesRead = 0;
	
	while (playing && samplesRead < samples)
	{
		//Read data
		int thisRead = stb_vorbis_get_samples_float_interleaved(file, channels, bufferPointer, (samples - samplesRead) * channels);
		
		//Move through file and buffer
		bufferPointer += thisRead * channels;
		samplesRead += thisRead;
		internalPosition += thisRead;
		
		if (thisRead == 0)
		{
			if (loopStart < 0)
				playing = false;
			else
				Loop();
		}
	}
	
	for (; samplesRead < samples * channels; samplesRead++)
		*bufferPointer++ = 0.0f;
	
	return samplesRead;
}

void MUSIC::Mix(float *stream, int samples)
{
	//Read the samples and convert to native format
	ma_pcm_converter_read(&resampler, stream, samples);
	
	//Apply volume
	for (int i = 0; i < samples * 2; i++)
		*stream++ *= volume;
}

ma_uint32 MusicReadSamples(ma_pcm_converter *dsp, void *buffer, ma_uint32 requestFrames, void *musicPointer)
{
	MUSIC *music = (MUSIC*)musicPointer;
	return music->ReadSamplesToBuffer((float*)buffer, requestFrames);
}

//Music functions
MUSIC *internalMusic = NULL;
SDL_Thread *loadMusicThread = NULL;
MUSICSPEC gMusicSpec;

int LoadMusicThreadFunction(void *dummy)
{
	//Unload current song
	if (internalMusic != NULL && gMusicSpec.name != internalMusic->source)
	{
		delete internalMusic;
		internalMusic = NULL;
	}
	
	//Load new song (if not null)
	if (gMusicSpec.name != NULL)
	{
		//If not reloading, initialize our internal music
		if (internalMusic == NULL)
			internalMusic = new MUSIC(gMusicSpec.name);
		
		//Play our music using the given specifications
		internalMusic->volume = gMusicSpec.initialVolume;
		internalMusic->Play(gMusicSpec.initialPosition);
	}
	
	loadMusicThread = NULL;
	return 0;
}

void PlayMusic(const MUSICSPEC musicSpec)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Wait for song to have already finished loading
	SDL_WaitThread(loadMusicThread, NULL);
	loadMusicThread = NULL;
	
	//Load new song using the specifications in a separate thread
	gMusicSpec = musicSpec;
	loadMusicThread = SDL_CreateThread(LoadMusicThreadFunction, "LoadMusicThread", NULL);
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

int PauseMusic()
{
	int position = gMusicSpec.initialPosition;
	
	if (loadMusicThread == NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		//Set the volume
		if (internalMusic != NULL)
			position = internalMusic->Pause();
		else
			position = -1;
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
	}
	
	return position;
}

void SetMusicVolume(float volume)
{
	if (loadMusicThread == NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		//Set the volume
		if (internalMusic != NULL)
			internalMusic->volume = volume;
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
	}
}

float GetMusicVolume()
{
	float volume = gMusicSpec.initialVolume;
	
	if (loadMusicThread == NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		//Set the volume
		if (internalMusic != NULL)
			volume = internalMusic->volume;
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
	}
	
	return volume;
}

int GetMusicPosition()
{
	int position = gMusicSpec.initialPosition;
	
	if (loadMusicThread == NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		//Set the volume
		if (internalMusic != NULL)
			position = internalMusic->internalPosition;
		else
			position = 0;
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
	}
	
	return position;
}

bool IsMusicPlaying()
{
	bool playing = true;
	
	if (loadMusicThread == NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		//Set the volume
		if (internalMusic != NULL)
			playing = internalMusic->playing;
		else
			playing = false;
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
	}
	
	return playing;
}

//Callback function
bool gAudioYield = false;

void YieldAudio(bool yield)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Set yield
	gAudioYield = yield;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

void AudioCallback(void *userdata, uint8_t *stream, int length)
{
	//We aren't using userdata, this prevents the warning
	(void)userdata;
	
	//Get our buffer to render to
	int samples = length / (2 * sizeof(float));
	
	//If window is unfocused, don't mix anything, pause audio
	if (gAudioYield)
	{
		//Clear the stream with 0.0f
		float *buffer = (float*)stream;
		for (int i = 0; i < samples * 2; i++)
			*buffer++ = 0.0f;
		return;
	}
	
	//Mix music into the buffer or clear the buffer
	if (internalMusic != NULL && internalMusic->playing)
	{
		//This will clear the stream with 0.0f
		internalMusic->Mix((float*)stream, samples);
	}
	else
	{
		//Clear the stream with 0.0f
		float *buffer = (float*)stream;
		for (int i = 0; i < samples * 2; i++)
			*buffer++ = 0.0f;
	}
	
	//Mix our sound effects into the buffer
	for (SOUND *sound = sounds; sound != NULL; sound = sound->next)
		sound->Mix((float*)stream, samples);
}

//Play sound functions
bool ringPanLeft = false;

unsigned int spindashPitch = 0;	//Spindash's pitch increase in semi-tones
unsigned int spindashTimer = 0;	//Timer for the spindash pitch to reset
bool spindashLast = false;	//Set to 1 if spindash was the last sound, set to 0 if it wasn't (resets the spindash pitch)

void PlaySound(SOUNDID id)
{
	SOUNDID playId = id;
	
	//Sound specifics go here (i.e ring panning, or spindash rev frequency)
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
		playId = (SOUNDID)((unsigned int)SOUNDID_SPINDASH_REV + spindashPitch);
		
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
			playId = ringPanLeft ? SOUNDID_RING_LEFT : SOUNDID_RING_RIGHT;
			soundEffects[playId]->SetPan(ringPanLeft ? 1.0f : 0.0f, ringPanLeft ? 0.0f : 1.0f);
		}
	}
	
	//Get our sound
	SOUND *sound = soundEffects[playId];
	
	//Stop sounds of the same channel
	for (int i = 0; i < SOUNDID_MAX; i++)
		if (soundEffects[i] != NULL && soundDefinition[i].channel == soundDefinition[playId].channel)
			soundEffects[i]->Stop();
	
	//Actually play our sound
	if (sound != NULL)
		sound->Play();
	return;
}

void StopSound(SOUNDID id)
{
	//Stop the given sound
	if (soundEffects[id] != NULL)
		soundEffects[id]->Stop();
	return;
}

//Load sound function
bool LoadAllSoundEffects()
{
	for (int i = 0; i < SOUNDID_MAX; i++)
	{
		if (soundDefinition[i].path != NULL)
			soundEffects[i] = new SOUND(soundDefinition[i].path); //Load from path
		else if (soundDefinition[i].parent != SOUNDID_NULL)
			soundEffects[i] = new SOUND(soundEffects[soundDefinition[i].parent]); //Load from parent
		else
		{
			soundEffects[i] = NULL;
			continue;
		}
		
		if (soundEffects[i]->fail)
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
	
	audioDevice = SDL_OpenAudioDevice(NULL, 0, &want, NULL, 0);
	if (!audioDevice)
		return Error(SDL_GetError());
	
	//Load all of our sound effects
	if (!LoadAllSoundEffects())
		return false;
	
	//Allow our audio device to play audio
	SDL_PauseAudioDevice(audioDevice, 0);
	
	LOG(("Success!\n"));
	return true;
}

void QuitAudio()
{
	LOG(("Ending audio... "));
	
	//Unload all of our sounds
	for (SOUND *sound = sounds; sound != NULL;)
	{
		SOUND *next = sound->next;
		delete sound;
		sound = next;
	}
	
	//Unload current song
	if (internalMusic != NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		MUSIC *rememberMusic = internalMusic;
		internalMusic = NULL;
		
		//Resume audio device then delete original music
		SDL_UnlockAudioDevice(audioDevice);
		delete rememberMusic;
	}
	
	//Close our audio device
	SDL_CloseAudioDevice(audioDevice);
	
	LOG(("Success!\n"));
	return;
}