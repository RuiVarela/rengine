// __!!rengine_copyright!!__ //

#ifndef __RENGINE_FONT_RESOURCE_LOADER_H__
#define __RENGINE_FONT_RESOURCE_LOADER_H__

#include <rengine/resource/ResourceLoader.h>
#include <rengine/text/Font.h>

namespace rengine
{

	class DefaultFontsResourceLoader : public ResourceLoader<Font>
	{
	public:
		virtual Bool suportsFormat(std::string const& extension) const;
		virtual Bool canLoadResourceFromLocation(std::string const& resource_location) const;

		virtual SharedPointer<Font> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	};

	class WinfontResourceLoader : public ResourceLoader<Font>
	{
	public:
		virtual Bool suportsFormat(std::string const& extension) const;
		virtual SharedPointer<Font> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	};

	class TruetypeFontResourceLoader : public ResourceLoader<Font>
	{
	public:
		virtual Bool suportsFormat(std::string const& extension) const;
		virtual SharedPointer<Font> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	};

} // namespace rengine

#endif //__RENGINE_FONT_RESOURCE_LOADER_H__
