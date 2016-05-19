// __!!rengine_copyright!!__ //

#include <rengine/resource/ResourceManager.h>

#include <rengine/image/ImageResourceLoader.h>
#include <rengine/text/FontResourceLoader.h>
#include <rengine/system/SystemScriptResourceLoader.h>
#include <rengine/state/ShaderResourceLoader.h>

namespace rengine
{
	//
	// add here the default resource loaders, they will be available on startup
	//
	void ResourceManager::registerDefaultLoaders()
	{
		addLoader(new ImageResourceLoader());
		addLoader(new Texture2DResourceLoader());
		addLoader(new DefaultFontsResourceLoader());
		addLoader(new WinfontResourceLoader());
		addLoader(new TruetypeFontResourceLoader());
		addLoader(new SystemScriptResourceLoader());
		addLoader(new ShaderResourceLoader());
		addLoader(new ProgramResourceLoader());
	}

} // end of namespace
