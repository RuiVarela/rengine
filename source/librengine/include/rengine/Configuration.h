// __!!rengine_copyright!!__ //

#ifndef __RENGINE_CONFIGURATION__
#define __RENGINE_CONFIGURATION__

#include <string>
#include <rengine/lang/Lang.h>
#include <rengine/util/StringTable.h>

namespace rengine
{
	class EngineConfiguration
	{
	public:
		EngineConfiguration();
		Int window_width;
		Int window_height;
		Bool fullscreen;
		Bool vsync;
		std::string console_background;
		std::string console_floating_background;
		std::string console_effect;
		Int console_number_of_lines;
		std::string text_effect;
		StringTable location_table;
	};
}

#endif // __RENGINE_CONFIGURATION__
