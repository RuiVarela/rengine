// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SYSTEM_SCRIPT_RESOURCE_LOADER_H__
#define __RENGINE_SYSTEM_SCRIPT_RESOURCE_LOADER_H__

#include <rengine/system/SystemScript.h>
#include <rengine/resource/ResourceLoader.h>

namespace rengine
{
	class SystemScriptResourceLoader : public ResourceLoader<SystemScript>
	{
	public:
		virtual Bool suportsFormat(std::string const& extension) const;
		virtual SharedPointer<SystemScript> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	};
}

#endif //__RENGINE_SYSTEM_SCRIPT_RESOURCE_LOADER_H__
