#ifndef __RENGINE_SEGA_H__
#define __RENGINE_SEGA_H__

#include <rengine/lang/debug/Debug.h>
#include <rengine/file/Zip.h>
#include <rengine/Scene.h>
#include <rengine/state/Texture.h>
#include <rengine/geometry/BaseShapes.h>
#include <rengine/image/Image.h>

#include <string>

class SegaVariableManager;

class Sega : public rengine::Scene, public rengine::SystemCommand::Handler
{
public:
	enum Commands
	{
		HardReset				= 0,
		SoftReset				= 1,
		ToggleBorderEmulation	= 2,
		SaveGame	        	= 3,
		LoadGame		    	= 4,
	};


	static Sega* instance;

	Sega();
	~Sega();

	virtual void init();
	virtual void shutdown();
	virtual void update();
	virtual void render();

	virtual void operator()(rengine::SystemCommand::CommandId const command, rengine::SystemCommand::Arguments const& arguments);

	rengine::SharedArray<rengine::Uint8> readFile(std::string const& filename, int& size);
	void loadRom(std::string const& rom);

	void saveGame(int slot);
	void loadGame(int slot);

	// do not call this directly
	void updateInput();
	void updateAudio();
	void updateVideo();
private:
	struct PrivateImplementation;
	rengine::SharedPointer<PrivateImplementation> m_pimpl;
	rengine::SharedPointer<SegaVariableManager> m_variable_manager;
	void loadBios();
	std::string filename;

	rengine::Real m_framerate;
	rengine::Real64 m_start_time;
	rengine::Uint64 m_rendered_frames;

	rengine::SharedPointer<rengine::Image> image;
	rengine::SharedPointer<rengine::Quadrilateral> quadrilateral;
	rengine::SharedPointer<rengine::Texture2D> texture;
};



#endif // __RENGINE_SEGA_H__
