#pragma once
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string>

//Path globals
extern std::string gBasePath;
extern std::string gPrefPath;

//File class
#ifdef WINDOWS //Include Windows stuff, required for opening the files with UTF-16 paths
	#include <wchar.h>
	#include <stringapiset.h>
#endif

class FS_FILE
{
	public:
		const char *fail = nullptr;
		FILE *fp = nullptr;
	public:
		//Constructor - Open file
		FS_FILE(const char *name, const char *mode) { OpenFile(name, mode); }
		FS_FILE(std::string name, const char *mode) { OpenFile(name.c_str(), mode); }
		
		//Destructor - Close file
		~FS_FILE()
		{
			if (fp != nullptr)
			//Close our opened file
			fclose(fp);
		}
		
		//File open function
		inline void OpenFile(const char *name, const char *mode)
		{
			//Open the given file
			#ifdef WINDOWS
				//Convert name to UTF-16
				size_t nameBufferSize = MultiByteToWideChar(CP_UTF8, 0, name, -1, nullptr, 0);
				wchar_t *nameWcharBuffer = new wchar_t[nameBufferSize];
				MultiByteToWideChar(CP_UTF8, 0, name, -1, nameWcharBuffer, nameBufferSize);
				
				//Convert mode to UTF-16
				size_t modeBufferSize = MultiByteToWideChar(CP_UTF8, 0, mode, -1, nullptr, 0);
				wchar_t *modeWcharBuffer = new wchar_t[modeBufferSize];
				MultiByteToWideChar(CP_UTF8, 0, mode, -1, modeWcharBuffer, modeBufferSize);
				
				//Open the file with our newly converted path
				fp = _wfopen(nameWcharBuffer, modeWcharBuffer);
				delete[] nameWcharBuffer;
				delete[] modeWcharBuffer;
			#else
				//Open file with the given name
				fp = fopen(name, mode);
			#endif
			
			//Check for errors
			if (fp == nullptr)
				fail = "Failed to open file";
		}
		
		//Read functions
		//Any size
		inline size_t Read(void *ptr, size_t size, size_t maxnum)	{ return fread(ptr, size, maxnum, fp); }
		
		//One byte
		inline uint8_t	ReadU8()
		{
			return fgetc(fp);
		}
		
		//Multi-byte big endian
		inline uint16_t	ReadBE16()
		{
			uint8_t bytes[2];
			Read(bytes, 1, 2);
			return ((uint16_t)bytes[0] << 8) | bytes[1];
		}
		
		inline uint32_t	ReadBE32()
		{
			uint8_t bytes[4];
			Read(bytes, 1, 4);
			return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint16_t)bytes[2] << 8) | bytes[3];
		}
		
		inline uint64_t	ReadBE64()
		{
			uint8_t bytes[8];
			Read(bytes, 1, 8);
			return ((uint64_t)bytes[0] << 56) | ((uint64_t)bytes[1] << 48) | ((uint64_t)bytes[2] << 40) | ((uint64_t)bytes[3] << 32) | ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) | ((uint16_t)bytes[2] << 8) | bytes[3];
		}
		
		//Multi-byte little endian
		inline uint16_t	ReadLE16()
		{
			uint8_t bytes[2];
			Read(bytes, 1, 2);
			return ((uint16_t)bytes[1] << 8) | bytes[0];
		}
		
		inline uint32_t	ReadLE32()
		{
			uint8_t bytes[4];
			Read(bytes, 1, 4);
			return ((uint32_t)bytes[3] << 24) | ((uint32_t)bytes[2] << 16) | ((uint16_t)bytes[1] << 8) | bytes[0];
		}
		
		inline uint64_t	ReadLE64()
		{
			uint8_t bytes[8];
			Read(bytes, 1, 8);
			return ((uint64_t)bytes[7] << 56) | ((uint64_t)bytes[6] << 48) | ((uint64_t)bytes[5] << 40) | ((uint64_t)bytes[4] << 32) | ((uint32_t)bytes[3] << 24) | ((uint32_t)bytes[2] << 16) | ((uint16_t)bytes[1] << 8) | bytes[0];
		}
		
		//Write functions
		//Any size
		inline size_t Write(const void *ptr, size_t size, size_t maxnum)	{ return fwrite(ptr, size, maxnum, fp); }
		
		//One byte
		inline void	WriteU8(uint8_t val)
		{
			fputc(val, fp);
		}
		
		//Multi-byte big endian
		inline void	WriteBE16(uint16_t val)
		{
			for (size_t i = 2; i-- != 0;)
				fputc(val >> (8 * i), fp);
		}
		
		inline void	WriteBE32(uint32_t val)
		{
			for (size_t i = 4; i-- != 0;)
				fputc(val >> (8 * i), fp);
		}
		
		inline void	WriteBE64(uint64_t val)
		{
			for (size_t i = 8; i-- != 0;)
				fputc(val >> (8 * i), fp);
		}
		
		//Multi-byte little endian
		inline void	WriteLE16(uint16_t val)
		{
			for (size_t i = 0; i < 2; i++)
				fputc(val >> (8 * i), fp);
		}
		
		inline void	WriteLE32(uint32_t val)
		{
			for (size_t i = 0; i < 4; i++)
				fputc(val >> (8 * i), fp);
		}
		
		inline void	WriteLE64(uint64_t val)
		{
			for (size_t i = 0; i < 8; i++)
				fputc(val >> (8 * i), fp);
		}
		
		//Seek and tell functions
		inline int Seek(long int offset, int origin)	{ return fseek(fp, offset, origin); }
		inline size_t Tell()							{ return ftell(fp); }
		
		//Size function
		inline size_t GetSize()	{ size_t origP = Tell(); Seek(0, SEEK_END); size_t size = Tell(); Seek(origP, SEEK_SET); return size; }
};

//Sub-system functions
bool InitializePath();
void QuitPath();
