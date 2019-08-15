#pragma once
#include <stdint.h>
#include "Render.h"
#include "Mappings.h"
#include "LevelCollision.h"

class OBJECT
{
	public:
		//Fail
		const char *fail;
		
		//Object sub-type
		int subtype;
		
		//Rendering stuff
		struct
		{
			bool xFlip : 1;
			bool yFlip : 1;
			bool alignPlane : 1;
			bool alignBackground : 1; //Basically, align to background if above is set
			bool assumePixelHeight : 1;
			bool bit5 : 1;
			bool bit6 : 1;
			bool onScreen : 1;
		} renderFlags;
		
		bool doRender;
		
		//Our texture and mappings
		TEXTURE *texture;
		MAPPINGS *mappings;
		
		#if SDL_BYTEORDER == SDL_BIGENDIAN
			//X-position (big endian)
			union
			{
				struct
				{
					int16_t xPos;
					uint16_t xSub;
				};
				int32_t xPosLong;
			};
			
			//Y-position (big endian)
			union
			{
				struct
				{
					int16_t yPos;
					uint16_t ySub;
				};
				int32_t yPosLong;
			};
		#else
			//X-position (little endian)
			union
			{
				struct
				{
					uint16_t sub;
					int16_t pos;
				} x;
				int32_t xPosLong;
			};
			
			//Y-position (little endian)
			union
			{
				struct
				{
					uint16_t sub;
					int16_t pos;
				} y;
				int32_t yPosLong;
			};
		#endif
		
		//Speeds
		int16_t xVel;		//Global X-velocity
		int16_t yVel;		//Global Y-velocity
		int16_t inertia;	//Horizontal velocity (on ground)
		
		//Collision box size
		uint8_t xRadius;
		uint8_t yRadius;
		
		//Sprite properties
		uint8_t priority;		//Priority of sprite when drawing
		uint8_t widthPixels;	//Width of sprite in pixels
		
		//Animation and mapping
		uint16_t mappingFrame;
		
		uint16_t animFrame;
		int anim;
		int nextAnim;
		int16_t animFrameDuration;
		
		//Our status
		struct
		{
			bool xFlip : 1;				//Set if facing left
			bool yFlip : 1;				//In the air
		} status;
		
		uint8_t routine;			//Routine
		uint8_t routineSecondary;	//Routine Secondary
		uint8_t respawnIndex;		//Index of this object in the respawn table, this is to not respawn unloaded enemies / monitors
		
		uint8_t angle;	//Angle
		void *parent;	//Our parent object or player
		
		//Our collision layers
		COLLISIONLAYER topSolidLayer;
		COLLISIONLAYER lrbSolidLayer;
		
		//Scratch memory
		union
		{
			uint8_t scratchU8[0x10];
			uint16_t scratchU16[0x8];
			uint32_t scratchU32[0x4];
			int8_t scratchS8[0x10];
			int16_t scratchS16[0x8];
			int32_t scratchS32[0x4];
		};
		
		OBJECT *next;
		OBJECT **list;
		void (*function)(OBJECT *object);
		
	public:
		OBJECT(OBJECT **linkedList, void (*objectFunction)(OBJECT *object));
		~OBJECT();
		
		void Update();
		void Draw();
};
