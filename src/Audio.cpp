#include "SDL_audio.h"

#include "Audio.h"
#include "Log.h"
#include "Error.h"
#include "Path.h"

#include "MathUtil.h"

SDL_AudioDeviceID audioDevice;

//Music
MUSICDEFINITION musicDefinition[MUSICID_MAX] = {
	{"data/Audio/Music/GHZ.ogg", 635160},
};

//Sound effects
SOUND *soundEffects[SOUNDID_MAX];
SOUNDDEFINITION soundDefinition[SOUNDID_MAX] = {
	{SOUNDCHANNEL_NULL, NULL, SOUNDID_NULL}, //SOUNDID_NULL
	{SOUNDCHANNEL_PSG0, "data/Audio/Sound/Jump.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM3,  "data/Audio/Sound/Roll.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_PSG1, "data/Audio/Sound/Skid.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRev.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,  "data/Audio/Sound/SpindashRelease.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	"data/Audio/Sound/Death.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_NULL,	"data/Audio/Sound/Ring.wav", SOUNDID_NULL},
	{SOUNDCHANNEL_FM4,	NULL, SOUNDID_RING}, //SOUNDID_RING_LEFT
	{SOUNDCHANNEL_FM3,	NULL, SOUNDID_RING}, //SOUNDID_RING_RIGHT
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
	LOG(("Creating a sound from parent %p... ", ourParent));
	
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

//Callback function
void AudioCallback(void *userdata, uint8_t *stream, int length)
{
	//We aren't using userdata, this prevents the warning
	(void)userdata;
	
	//Get our buffer to render to
	float *buffer = (float*)stream;
	int samples = length / (2 * sizeof(float));
	
	//Clear the buffer (NOTE: I would memset the buffer to 0, but that'll break on FPUs that don't use the IEEE-754 standard)
	for (int i = 0; i < samples * 2; i++)
		*buffer++ = 0.0f;
	
	//Mix our sound effects into the buffer
	for (SOUND *sound = sounds; sound != NULL; sound = sound->next)
		sound->Mix((float*)stream, samples);
}

//Play sound functions
bool ringPanLeft = false;

void PlaySound(SOUNDID id)
{
	SOUNDID playId = id;
	
	//Sound specifics go here (i.e ring panning, or spindash rev frequency)
	if (id == SOUNDID_RING)
	{
		//Flip between left and right every time the sound plays
		ringPanLeft ^= 1;
		playId = ringPanLeft ? SOUNDID_RING_LEFT : SOUNDID_RING_RIGHT;
		soundEffects[playId]->SetPan(ringPanLeft ? 1.0f : 0.0f, ringPanLeft ? 0.0f : 1.0f);
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
	
	//Allow our audio device to play audio
	SDL_PauseAudioDevice(audioDevice, 0);
	
	//Load all of our sound effects
	if (!LoadAllSoundEffects())
		return false;
	
	LOG(("Success!\n"));
	return true;
}

void QuitAudio()
{
	LOG(("Ending audio... "));
	
	//Close our audio device
	SDL_CloseAudioDevice(audioDevice);
	
	//Unload all of our sounds
	for (SOUND *sound = sounds; sound != NULL;)
	{
		SOUND *next = sound->next;
		delete sound;
		sound = next;
	}
	
	LOG(("Success!\n"));
	return;
}