// __!!rengine_copyright!!__ //

#include <rengine/resource/ResourceLoader.h>
#include <rengine/file/File.h>

namespace rengine
{
	Bool BaseResourceLoader::suportsFormat(std::string const& extension) const
	{
		return false;
	}

	Bool BaseResourceLoader::canLoadResourceFromLocation(std::string const& resource_location) const
	{
		return (suportsFormat( getFileExtension(resource_location) ) && fileExists(resource_location));
	}
} // end of namespace

