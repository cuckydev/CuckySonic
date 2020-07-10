#pragma once
#include <stdint.h>

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
	SOUNDID_COLLAPSE,
	SOUNDID_WALL_SMASH,
	
	SOUNDID_WATERFALL,
	SOUNDID_WATERFALL_1,
	SOUNDID_WATERFALL_2,
	
	SOUNDID_GET_BLUE_BARRIER,
	SOUNDID_GET_FLAME_BARRIER,
	SOUNDID_GET_LIGHTNING_BARRIER,
	SOUNDID_GET_AQUA_BARRIER,
	
	SOUNDID_DOUBLE_SPIN_ATTACK,
	SOUNDID_USE_FLAME_BARRIER,
	SOUNDID_USE_LIGHTNING_BARRIER,
	SOUNDID_USE_AQUA_BARRIER,
	
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
#define SOUNDCHANNEL_TYPE	uint16_t

struct SOUNDDEFINITION
{
	SOUNDCHANNEL_TYPE channel;
	const char *path;
	SOUNDID parent;
};

//Sound functions
void PlaySound(SOUNDID id);
void StopSound(SOUNDID id);
void StopChannel(SOUNDCHANNEL_TYPE channel);

//Audio subsystem functions
bool InitializeAudio();
void QuitAudio();
