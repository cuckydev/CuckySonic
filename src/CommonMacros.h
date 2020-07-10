#pragma once
#ifdef ENDIAN_BIG
	//Big endian - [high>low] = long
	#define FPDEF(name, highType, highName, lowType, lowName, longType)	\
		union	\
		{	\
			struct	\
			{	\
				highType highName;	\
				lowType lowName;	\
			} name;	\
			longType name##Long = 0;	\
		};
#else
	//Little endian - [high<low] = long
	#define FPDEF(name, highType, highName, lowType, lowName, longType)	\
		union	\
		{	\
			struct	\
			{	\
				lowType lowName;	\
				highType highName;	\
			} name;	\
			longType name##Long = 0;	\
		};
#endif
