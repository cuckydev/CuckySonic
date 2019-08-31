#include "SDL_audio.h"
#include "SDL_timer.h"

#include "Audio.h"
#include "Log.h"
#include "Error.h"
#include "Path.h"

#include "MathUtil.h"

SDL_AudioDeviceID audioDevice;

//Music definitions
MUSICDEFINITION musicDefinition[MUSICID_MAX] = {
	{NULL, 0}, //MUSICID_NULL
	{"data/Audio/Music/Title.ogg", -1},
	{"data/Audio/Music/Menu.ogg", 0},
	{"data/Audio/Music/SpeedShoes.ogg", -1},
	{"data/Audio/Music/GHZ.ogg", 635083},
	{"data/Audio/Music/EHZ.ogg", 153816},
};

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
			const float sample1 = buffer[(int)sample * 2 + channel];
			const float sample2 = (((int)sample + 1) >= size) ? 0 : buffer[(int)sample * 2 + channel + 1];
			
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

//Music class (using stb_vorbis backend)
#ifdef WINDOWS
	#include <wchar.h> //Required for opening the actual files
	#include <stringapiset.h>
#endif

//Constructor and destructor
MUSIC::MUSIC(MUSICDEFINITION *ourDefinition)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	memset(this, 0, sizeof(MUSIC));
	definition = ourDefinition;
	
	LOG(("Loading music file from %s... ", definition->path));
	
	#ifdef WINDOWS
		//Open file (Windows only UTF-16 wchar bullshit)
		GET_GLOBAL_PATH(musicPath, definition->path);
		
		//Convert to UTF-16
		wchar_t wcharBuffer[0x200];
		MultiByteToWideChar(CP_UTF8, 0, musicPath, -1, wcharBuffer, 0x200);
		
		//Open the file with our newly converted path
		FILE *fp = _wfopen(wcharBuffer, L"rb");
	#else
		//Open file
		GET_GLOBAL_PATH(musicPath, definition->path);
		FILE *fp = fopen(musicPath, "rb");
	#endif
	
	//If the file failed to open...
	if (fp == NULL)
	{
		Error(fail = "Failed to open the given file");
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
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
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
		return;
	}
	
	//Get information from file
	stb_vorbis_info info = stb_vorbis_get_info(file);
	channels = info.channels;
	
	//Initialize other properties
	volume = 1.0f;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
	
	LOG(("Success!\n"));
}

MUSIC::~MUSIC()
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Close our loaded file
	if (file != NULL)
		stb_vorbis_close(file);
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

//Playback functions
void MUSIC::Play(int position)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Play at the given position
	internalPosition = (double)position;
	playing = true;
	
	//Seek within the given file
	if (stb_vorbis_seek_frame(file, position) < 0)
	{
		Warn("Failed to play the music at given position");
		playing = false;
		
		//Resume audio device
		SDL_UnlockAudioDevice(audioDevice);
		return;
	}
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

int MUSIC::Pause()
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Pause and get the position we're at
	int position = (int)internalPosition;
	playing = false;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
	return position;
}

void MUSIC::SetVolume(float setVolume)
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Set volume
	volume = setVolume;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
}

float MUSIC::GetVolume()
{
	//Wait for audio device to be finished and lock it
	SDL_LockAudioDevice(audioDevice);
	
	//Set volume
	float getVolume = volume;
	
	//Resume audio device
	SDL_UnlockAudioDevice(audioDevice);
	return getVolume;
}

//Mixer functions
void MUSIC::Loop()
{
	//Seek backwards definition->loopLength frames
	if (stb_vorbis_seek_frame(file, (int)(internalPosition = definition->loopStart)) < 0)
	{
		Warn("Failed to play the music at given position");
		playing = false;
		return;
	}
}

void MUSIC::Mix(float *stream, int samples)
{
	//Read the given amount of samples
	float buffer[AUDIO_SAMPLES * 2];
	float *bufferCopy = &(*buffer);
	
	int samplesRead = 0;
	
	while (samplesRead < samples)
	{
		//Read data
		int thisRead = stb_vorbis_get_samples_float_interleaved(file, channels, buffer, (samples - samplesRead) * channels);
		
		//Move through file and buffer
		bufferCopy += thisRead * 2;
		samplesRead += thisRead;
		internalPosition += (double)thisRead;
		
		if (thisRead == 0)
		{
			if (definition->loopStart < 0)
			{
				//Doesn't loop
				playing = false;
				break;
			}
			else
			{
				//Loop back
				Loop();
			}
		}
	}
	
	//Copy the buffer into our stream
	bufferCopy = &(*buffer);
	
	for (int i = 0; i < samples; i++)
	{
		if (i < samplesRead)
		{
			*stream++ = (*bufferCopy++) * volume;
			*stream++ = (*bufferCopy++) * volume;
		}
		else
		{
			*stream++ = 0.0f;
			*stream++ = 0.0f;
		}
	}
}

//Music functions
MUSIC *currentMusic;

void PlayMusic(MUSICID music)
{
	//Unload current song
	if (currentMusic != NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		MUSIC *rememberMusic = currentMusic;
		currentMusic = NULL;
		
		//Resume audio device then delete original music
		SDL_UnlockAudioDevice(audioDevice);
		delete rememberMusic;
	}
	
	//Load new song (if not null)
	if (music != MUSICID_NULL)
	{
		currentMusic = new MUSIC(&musicDefinition[music]);
		currentMusic->Play(0);
	}
}

int PauseMusic()
{
	//Pause and return current position
	if (currentMusic != NULL)
		return currentMusic->Pause();
	return -1;
}

void ResumeMusic(int position)
{
	//Resume at given position
	if (currentMusic != NULL)
		currentMusic->Play(position);
}

void SetMusicVolume(float volume)
{
	//Set the volume
	if (currentMusic != NULL)
		currentMusic->SetVolume(volume);
}

float GetMusicVolume()
{
	//Get and return the volume
	if (currentMusic != NULL)
		return currentMusic->GetVolume();
	return 1.0f;
}

//Callback function
void AudioCallback(void *userdata, uint8_t *stream, int length)
{
	//We aren't using userdata, this prevents the warning
	(void)userdata;
	
	//Get our buffer to render to
	int samples = length / (2 * sizeof(float));
	
	if (currentMusic != NULL && currentMusic->playing)
	{
		//Mix the music into the buffer, we don't need to clear the buffer here because the music sets the buffer, rather than add onto it
		currentMusic->Mix((float*)stream, samples);
	}
	else
	{
		//Clear the buffer (NOTE: I would memset the buffer to 0, but that'll break on FPUs that don't use the IEEE-754 standard)
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
	if (currentMusic != NULL)
	{
		//Wait for audio device to be finished and lock it
		SDL_LockAudioDevice(audioDevice);
		
		MUSIC *rememberMusic = currentMusic;
		currentMusic = NULL;
		
		//Resume audio device then delete original music
		SDL_UnlockAudioDevice(audioDevice);
		delete rememberMusic;
	}
	
	//Close our audio device
	SDL_CloseAudioDevice(audioDevice);
	
	LOG(("Success!\n"));
	return;
}