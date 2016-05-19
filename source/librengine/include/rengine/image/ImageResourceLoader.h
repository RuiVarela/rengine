// __!!rengine_copyright!!__ //

#ifndef __RENGINE_IMAGE_RESOURCE_LOADER_H__
#define __RENGINE_IMAGE_RESOURCE_LOADER_H__

#include <rengine/image/Image.h>
#include <rengine/state/Texture.h>

#include <rengine/resource/ResourceLoader.h>

namespace rengine
{
	class ImageResourceLoader : public ResourceLoader<Image>
	{
	public:
		virtual bool suportsFormat(std::string const& extension) const;
		virtual SharedPointer<Image> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	};

	class Texture2DResourceLoader : public ResourceLoader<Texture2D>
	{
	public:
		virtual bool suportsFormat(std::string const& extension) const;
		virtual SharedPointer<Texture2D> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	};

} //namespace

#endif//__RENGINE_IMAGE_RESOURCE_LOADER_H__
