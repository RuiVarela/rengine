#include <rengine/lang/debug/Debug.h>
#include <rengine/file/Zip.h>
#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/system/System.h>
#include <rengine/math/Math.h>
#include <rengine/state/BaseStates.h>
#include <rengine/state/Program.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/outputstream/Log.h>
#include <rengine/outputstream/OutputStream.h>

#include <sstream>
#include <cstdio>

#include <AL/al.h>
#include <AL/alc.h>


#include "..\..\driver.h"

#include "Nes.h"

using namespace rengine;

extern "C"
{
	int PPUViewScanline =0;
	int PPUViewer = 0;

	void UpdatePPUView(int refreshchr)
	{
		FCEUD_PrintError("void UpdatePPUView(int refreshchr) not implemented!");
	}

	void FCEUD_SetPalette(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
	{
		Nes::instance->SetPalette(index, r, g, b);
	}

	FILE *FCEUD_UTF8fopen(const char *n, const char *m)
	{
		return(fopen(n,m));
	}

	void FCEUD_Message(char *text)
	{
		CoreEngine::instance()->log() << text << std::endl;
		RLOG_INFO("fceud", text);
	}

	void FCEUD_PrintError(char *s)
	{
		CoreEngine::instance()->log() << s << std::endl;
		RLOG_ERROR("fceud", s);
	}

	uint8* FCEUD_LoadFromDatabase(const char *fn, int* size)
	{
		*size = 0;
		uint8* data = 0;

		rengine::Zip zip;

		if (!zip.fileExists(fn))
		{
			zip.close();
			zip.load(NES_DATABASE);
		}

		//if (!zip.fileExists(fn))
		//{
		//	zip.close();
		//	zip.load(MASTERSYSTEM_DATABASE);
		//}

		if (zip.fileExists(fn))
		{
			Zip::FileData file_data = zip.read(fn);
			*size = file_data.size;

			if (file_data.size && file_data.data)
			{
				data = (uint8*)malloc(file_data.size);
				if (data)
				{
					memcpy(data, file_data.data.get(), file_data.size);
				}
				else
				{
					*size = 0;
				}
			}
		}

		zip.close();

		return data;
	}
}

Nes* Nes::instance = 0;

Nes::Nes()
{
	RENGINE_ASSERT(Nes::instance == 0);
	Nes::instance = this;


	memset(m_color_palette, 0, sizeof(m_color_palette));
	m_color_palette_changed = 0;
}

Nes::~Nes()	
{
	Nes::instance = 0;
}

void Nes::SetPalette(unsigned char index, unsigned char r, unsigned char g, unsigned char b)
{
	m_color_palette[index].r = r;
	m_color_palette[index].g = g;
	m_color_palette[index].b = b;
	m_color_palette_changed = 1;
}

void Nes::init()
{
}

void Nes::shutdown()

{
}

void Nes::update()

{
}

void Nes::render()
{
}
