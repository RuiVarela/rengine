#pragma once

#include <rengine/lang/debug/Debug.h>
#include <rengine/file/Zip.h>
#include <rengine/Scene.h>
#include <rengine/state/Texture.h>
#include <rengine/geometry/BaseShapes.h>
#include <rengine/image/Image.h>

#include <string>

#define NES_DATABASE	"data\\emulator\\roms\\nes.zip"


class Nes : public rengine::Scene, public rengine::SystemCommand::Handler
{
public:
	
	static Nes* instance;

	Nes();
	~Nes();

	virtual void init();
	virtual void shutdown();
	virtual void update();
	virtual void render();

	void SetPalette(unsigned char index, unsigned char r, unsigned char g, unsigned char b);
private:
	struct PaleteEntry
	{
		uint8 r;
		uint8 g;
		uint8 b;
	};

	PaleteEntry m_color_palette[256];
	int m_color_palette_changed;

};


