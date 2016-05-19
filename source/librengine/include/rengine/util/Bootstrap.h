// __!!rengine_copyright!!__ //

#ifndef __RENGINE_BOOTSTRAP__
#define __RENGINE_BOOTSTRAP__

#include <rengine/lang/debug/Debug.h>
#include <rengine/Raw.h>
#include <rengine/Scene.h>
#include <rengine/lang/exception/BaseExceptions.h>
#include <rengine/file/File.h>
#include <rengine/TypeSerialization.h>
#include <rengine/system/System.h>
#include <rengine/text/Fonts.h>
#include <rengine/text/Text.h>
#include <rengine/text/HudWriter.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/geometry/BaseShapes.h>
#include <rengine/geometry/Heightmap.h>
#include <rengine/state/BaseStates.h>
#include <rengine/state/Program.h>
#include <rengine/state/FrameBuffer.h>
#include <rengine/event/EventManager.h>
#include <rengine/camera/FpsCamera.h>
#include <rengine/camera/OrbitCamera.h>
#include <rengine/capture/VideoCapture.h>
#include <rengine/capture/ThreadedVideoCapture.h>
#include <rengine/image/processing/ImageProcessor.h>
#include <rengine/interface/InterfaceComponent.h>

#include <limits>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>

using namespace std;
using namespace rengine;


template<typename T>
RENGINE_INLINE int run(int argc, char **argv)
{
	rengine::enableApplicationDebugger();

	rengine::CoreEngine::create();

	std::string const config_file("engine_configuration.xml");
	rengine::EngineConfiguration core_engine_configuration;

	try
	{
		if (rengine::fileType(config_file) == rengine::FileRegular)
		{
			rengine::XmlArchive archive;
			archive.open(config_file);
			XML_SERIALIZE(archive, core_engine_configuration);
		}
		else
		{
			throw rengine::FileNotFoundException(10001, config_file);
		}

		rengine::CoreEngine::instance()->startEngine(core_engine_configuration);

		rengine::SharedPointer<T> scene(new T());
		rengine::CoreEngine::instance()->eventManager().addGuiEventHandler(scene);
		rengine::CoreEngine::instance()->setScene(scene);
	}
	catch (Exception caught)
	{
		std::cout << caught << std::endl;
		throw ;
	}


	CoreEngine::instance()->renderLoop();

	CoreEngine::destroy();

	return 0;
}

#define RENGINE_BOOT() int main(int argc, char **argv) { return run<MainScene>(argc, argv); }

#endif //__RENGINE_BOOTSTRAP__