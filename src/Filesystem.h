#pragma once
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
		FS_FILE(const char *name, const char *mode)
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
		
		FS_FILE(std::string name, const char *mode) { FS_FILE(name.c_str(), mode); }
		
		//Destructor - Close file
		~FS_FILE()
		{
			//Close our opened standard file
			fclose(fp);
		}
		
		//Read functions
		//Any size
		inline size_t Read(void *ptr, size_t size, size_t maxnum)	{ return fread(ptr, size, maxnum, fp); }
		
		//One byte
		inline uint8_t	ReadU8()	{ return fgetc(fp); }
		
		//Multi-byte big endian
		inline uint16_t	ReadBE16()	{ return (((uint16_t)fgetc(fp) << 8) | ((uint16_t)fgetc(fp))); }
		inline uint32_t	ReadBE32()	{ return (((uint32_t)fgetc(fp) << 24) | ((uint32_t)fgetc(fp) << 16) | ((uint32_t)fgetc(fp) << 8) | ((uint32_t)fgetc(fp))); }
		inline uint64_t	ReadBE64()	{ return (((uint64_t)fgetc(fp) << 56) | ((uint64_t)fgetc(fp) << 48) | ((uint64_t)fgetc(fp) << 40) | ((uint64_t)fgetc(fp) << 32) | ((uint64_t)fgetc(fp) << 24) | ((uint64_t)fgetc(fp) << 16) | ((uint64_t)fgetc(fp) << 8) | ((uint64_t)fgetc(fp))); }
		
		//Multi-byte little endian
		inline uint16_t	ReadLE16()	{ return (((uint16_t)fgetc(fp)) | ((uint16_t)fgetc(fp) << 8)); }
		inline uint32_t	ReadLE32()	{ return (((uint32_t)fgetc(fp)) | ((uint32_t)fgetc(fp) << 8) | ((uint32_t)fgetc(fp) << 16) | ((uint32_t)fgetc(fp) << 24)); }
		inline uint64_t	ReadLE64()	{ return (((uint64_t)fgetc(fp)) | ((uint64_t)fgetc(fp) << 8) | ((uint64_t)fgetc(fp) << 16) | ((uint64_t)fgetc(fp) << 24) | ((uint64_t)fgetc(fp) << 32) | ((uint64_t)fgetc(fp) << 40) | ((uint64_t)fgetc(fp) << 48) | ((uint64_t)fgetc(fp) << 56)); }
		
		//Write function
		//Any size
		inline size_t Write(const void *ptr, size_t size, size_t maxnum)	{ return fwrite(ptr, size, maxnum, fp); }
		
		//One byte
		inline size_t	WriteU8(const uint8_t val)	{ return fputc(val, fp); }
		
		//Multi-byte big endian
		inline uint16_t	WriteBE16(const uint16_t val)	{ return (fputc((val >> 8) & 0xFF, fp) + fputc(val & 0xFF, fp)); }
		inline uint32_t	WriteBE32(const uint32_t val)	{ return (fputc((val >> 24) & 0xFF, fp) + fputc((val >> 16) & 0xFF, fp) + fputc((val >> 8) & 0xFF, fp) + fputc(val & 0xFF, fp)); }
		inline uint64_t	WriteBE64(const uint64_t val)	{ return (fputc((val >> 56) & 0xFF, fp) + fputc((val >> 48) & 0xFF, fp) + fputc((val >> 40) & 0xFF, fp) + fputc((val >> 32) & 0xFF, fp) + fputc((val >> 24) & 0xFF, fp) + fputc((val >> 16) & 0xFF, fp) + fputc((val >> 8) & 0xFF, fp) + fputc(val & 0xFF, fp)); }
		
		//Multi-byte little endian
		inline uint16_t WriteLE16(const uint16_t val)	{ return (fputc(val & 0xFF, fp) + fputc((val >> 8) & 0xFF, fp)); }
		inline uint32_t	WriteLE32(const uint32_t val)	{ return (fputc(val & 0xFF, fp) + fputc((val >> 8) & 0xFF, fp) + fputc((val >> 16) & 0xFF, fp) + fputc((val >> 24) & 0xFF, fp)); }
		inline uint64_t	WriteLE64(const uint64_t val)	{ return (fputc(val & 0xFF, fp) + fputc((val >> 8) & 0xFF, fp) + fputc((val >> 16) & 0xFF, fp) + fputc((val >> 24) & 0xFF, fp) + fputc((val >> 32) & 0xFF, fp) + fputc((val >> 40) & 0xFF, fp) + fputc((val >> 48) & 0xFF, fp) + fputc((val >> 56) & 0xFF, fp)); }
		
		//Seek and tell functions
		inline int Seek(long int offset, int origin)	{ return fseek(fp, offset, origin); }
		inline size_t Tell()							{ return ftell(fp); }
		
		//Size function
		inline size_t GetSize()	{ size_t origP = Tell(); Seek(0, SEEK_END); size_t size = Tell(); Seek(origP, SEEK_SET); return size; }
};

//Sub-system functions
bool InitializePath();
void QuitPath();
