// __!!rengine_copyright!!__ //

#include <rengine/system/SystemScriptResourceLoader.h>
#include <rengine/file/File.h>

namespace rengine
{
	Bool SystemScriptResourceLoader::suportsFormat(std::string const& extension) const
	{
		Bool can_load = false;

		if (extension == "rss")
		{
			can_load = true;
		}

		return can_load;
	}

	SharedPointer<SystemScript> SystemScriptResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<SystemScript> script;

		std::string script_text;

		if (readRawText(location, script_text))
		{
			script = new SystemScript();

			if (!script->fromScriptText(script_text))
			{
				script = 0;
			}
		}

		return script;
	}
}
