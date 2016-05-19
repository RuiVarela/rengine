/*
 * Windows Bitmap File Loader
 * Version 1.2.4 (20111211)
 *
 * Supported Formats: 1, 4, 8, 16, 24, 32 Bit Images
 * Alpha Bitmaps are also supported.
 * Supported compression types: RLE 8, BITFIELDS
 *
 * Created by: Benjamin Kalytta, 2006 - 2011
 *
 * Licence: Free to use, URL to my source and my name is required in your source code.
 *
 * Source can be found at http://www.kalytta.com/bitmap.h
 *
 * Warning: This code should not be used in unmodified form in a production environment.
 * It should only serve as a basis for your own development.
 * There is only a minimal error handling in this code. (Notice added 20111211)
 */

#ifndef BITMAP_H
#define BITMAP_H

#include <iostream>
#include <string>

#define BITMAP_SIGNATURE 0x4d42

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int8 uint8_t;
	typedef __int32 int32_t;
#elif defined(__GNUC__) || defined(__CYGWIN__) || defined(__MWERKS__) || defined(__WATCOMC__) || defined(__PGI) || defined(__LCC__)
	#include <stdint.h>
#else
	typedef unsigned int uint32_t;
	typedef unsigned short int uint16_t;
	typedef unsigned char uint8_t;
	typedef int int32_t;
#endif

#pragma pack(push, 1)

typedef struct _BITMAP_FILEHEADER {
	uint16_t Signature;
	uint32_t Size;
	uint32_t Reserved;
	uint32_t BitsOffset;
} BITMAP_FILEHEADER;

#define BITMAP_FILEHEADER_SIZE 14

typedef struct _BITMAP_HEADER {
	uint32_t HeaderSize;
	int32_t Width;
	int32_t Height;
	uint16_t Planes;
	uint16_t BitCount;
	uint32_t Compression;
	uint32_t SizeImage;
	int32_t PelsPerMeterX;
	int32_t PelsPerMeterY;
	uint32_t ClrUsed;
	uint32_t ClrImportant;
	uint32_t RedMask;
	uint32_t GreenMask;
	uint32_t BlueMask;
	uint32_t AlphaMask;
	uint32_t CsType;
	uint32_t Endpoints[9]; // see http://msdn2.microsoft.com/en-us/library/ms536569.aspx
	uint32_t GammaRed;
	uint32_t GammaGreen;
	uint32_t GammaBlue;
} BITMAP_HEADER;

typedef struct _RGBA {
	uint8_t Red;
	uint8_t Green;
	uint8_t Blue;
	uint8_t Alpha;
} RGBA;

typedef struct _BGRA {
	uint8_t Blue;
	uint8_t Green;
	uint8_t Red;
	uint8_t Alpha;
} BGRA;

#pragma pack(pop)

class CBitmap {
private:
	BITMAP_FILEHEADER m_BitmapFileHeader;
	BITMAP_HEADER m_BitmapHeader;
	RGBA *m_BitmapData;
	unsigned int m_BitmapSize;
	char* m_src;
	unsigned int m_src_pos;

public:	
	CBitmap() : m_BitmapData(0), m_BitmapSize(0)  { Dispose(); }
	~CBitmap() { Dispose(); }
	
	void Dispose() {
		if (m_BitmapData) {
			delete[] m_BitmapData;
			m_BitmapData = 0;
		}
		memset(&m_BitmapFileHeader, 0, sizeof(m_BitmapFileHeader));
		memset(&m_BitmapHeader, 0, sizeof(m_BitmapHeader));
	}
	

	void* GetBits() { return m_BitmapData; }
	unsigned int GetWidth() { return m_BitmapHeader.Width < 0 ? -m_BitmapHeader.Width : m_BitmapHeader.Width; }
	unsigned int GetHeight() {	return m_BitmapHeader.Height < 0 ? -m_BitmapHeader.Height : m_BitmapHeader.Height; }
	unsigned int GetBitCount() { return m_BitmapHeader.BitCount; }
	

	void read(char* dst, int size)
	{
		for (int i = 0; i != size; ++i)
			*(dst + i) = *(m_src + m_src_pos + i);
	
		m_src_pos += size;
	}
	void seekg(int offset) { m_src_pos = offset; }


	bool Load(char* data) {
		m_src = data;
		m_src_pos = 0;
		
		
		Dispose();
		
		read((char*) &m_BitmapFileHeader, BITMAP_FILEHEADER_SIZE);
		if (m_BitmapFileHeader.Signature != BITMAP_SIGNATURE) {
			return false;
		}

		read((char*) &m_BitmapHeader, sizeof(BITMAP_HEADER));
		
		/* Load Color Table */
		
		seekg(BITMAP_FILEHEADER_SIZE + m_BitmapHeader.HeaderSize);
		
		unsigned int ColorTableSize = 0;

		if (m_BitmapHeader.BitCount == 1) {
			ColorTableSize = 2;
		} else if (m_BitmapHeader.BitCount == 4) {
			ColorTableSize = 16;
		} else if (m_BitmapHeader.BitCount == 8) {
			ColorTableSize = 256;
		}
		
		/* Only full color tables are supported */

		if (ColorTableSize != m_BitmapHeader.ClrUsed) {
			return false;
		}
		
		BGRA* ColorTable = new BGRA[ColorTableSize]; // std::bad_alloc exception should be thrown if memory is not available
		
		read((char*) ColorTable, sizeof(BGRA) * ColorTableSize);

		/* ... Color Table for 16 bits images are not supported yet */	
		
		m_BitmapSize = GetWidth() * GetHeight();
		m_BitmapData = new RGBA[m_BitmapSize];
		
		unsigned int LineWidth = ((GetWidth() * GetBitCount() / 8) + 3) & ~3;
		uint8_t *Line = new uint8_t[LineWidth];
		
		seekg(m_BitmapFileHeader.BitsOffset);

		int Index = 0;
		bool Result = true;

		if (m_BitmapHeader.Compression == 0) {
			for (unsigned int i = 0; i < GetHeight(); i++) {
				read((char*) Line, LineWidth);

				uint8_t *LinePtr = Line;
				
				for (unsigned int j = 0; j < GetWidth(); j++) {
					if (m_BitmapHeader.BitCount == 1) {
						uint32_t Color = *((uint8_t*) LinePtr);
						for (int k = 0; k < 8; k++) {
							m_BitmapData[Index].Red = ColorTable[Color & 0x80 ? 1 : 0].Red;
							m_BitmapData[Index].Green = ColorTable[Color & 0x80 ? 1 : 0].Green;
							m_BitmapData[Index].Blue = ColorTable[Color & 0x80 ? 1 : 0].Blue;
							m_BitmapData[Index].Alpha = ColorTable[Color & 0x80 ? 1 : 0].Alpha;
							Index++;
							Color <<= 1;
						}
						LinePtr++;
						j += 7;
					} else if (m_BitmapHeader.BitCount == 4) {
						uint32_t Color = *((uint8_t*) LinePtr);
						m_BitmapData[Index].Red = ColorTable[(Color >> 4) & 0x0f].Red;
						m_BitmapData[Index].Green = ColorTable[(Color >> 4) & 0x0f].Green;
						m_BitmapData[Index].Blue = ColorTable[(Color >> 4) & 0x0f].Blue;
						m_BitmapData[Index].Alpha = ColorTable[(Color >> 4) & 0x0f].Alpha;
						Index++;
						m_BitmapData[Index].Red = ColorTable[Color & 0x0f].Red;
						m_BitmapData[Index].Green = ColorTable[Color & 0x0f].Green;
						m_BitmapData[Index].Blue = ColorTable[Color & 0x0f].Blue;
						m_BitmapData[Index].Alpha = ColorTable[Color & 0x0f].Alpha;
						Index++;
						LinePtr++;
						j++;
					} else if (m_BitmapHeader.BitCount == 8) {
						uint32_t Color = *((uint8_t*) LinePtr);
						m_BitmapData[Index].Red = ColorTable[Color].Red;
						m_BitmapData[Index].Green = ColorTable[Color].Green;
						m_BitmapData[Index].Blue = ColorTable[Color].Blue;
						m_BitmapData[Index].Alpha = ColorTable[Color].Alpha;
						Index++;
						LinePtr++;
					} else if (m_BitmapHeader.BitCount == 16) {
						uint32_t Color = *((uint16_t*) LinePtr);
						m_BitmapData[Index].Red = ((Color >> 10) & 0x1f) << 3;
						m_BitmapData[Index].Green = ((Color >> 5) & 0x1f) << 3;
						m_BitmapData[Index].Blue = (Color & 0x1f) << 3;
						m_BitmapData[Index].Alpha = 255;
						Index++;
						LinePtr += 2;
					} else if (m_BitmapHeader.BitCount == 24) {
						uint32_t Color = *((uint32_t*) LinePtr);
						m_BitmapData[Index].Blue = Color & 0xff;
						m_BitmapData[Index].Green = (Color >> 8) & 0xff;
						m_BitmapData[Index].Red = (Color >> 16) & 0xff;
						m_BitmapData[Index].Alpha = 255;
						Index++;
						LinePtr += 3;
					} else if (m_BitmapHeader.BitCount == 32) {
						uint32_t Color = *((uint32_t*) LinePtr);
						m_BitmapData[Index].Blue = Color & 0xff;
						m_BitmapData[Index].Green = (Color >> 8) & 0xff;
						m_BitmapData[Index].Red = (Color >> 16) & 0xff;
						m_BitmapData[Index].Alpha = Color >> 24;
						Index++;
						LinePtr += 4;
					}
				}
			}
		} 
		else
		{
			Result = false;
		}
		
		delete [] ColorTable;
		delete [] Line;

		return Result;
	}
};

#endif