// __!!rengine_copyright!!__ //

#ifndef __RENGINE_TYPESERIALIZATION_H__
#define __RENGINE_TYPESERIALIZATION_H__

#include <rengine/file/XmlSerialization.h>
#include <rengine/image/Image.h>
#include <rengine/Configuration.h>

namespace rengine
{
	namespace serialization
	{
		template <>
		RENGINE_INLINE void serialize<StringElement>(XmlArchive& archive, StringElement& node)
		{
			XML_SERIALIZE(archive, node.text);
		}

		template <>
		RENGINE_INLINE void serialize<StringTable>(XmlArchive& archive, StringTable& table)
		{
			std::map<std::string, StringElement>& map = table;
			rengine::serialization::serialize(archive, map);
		}

		template <>
		RENGINE_INLINE void serialize<EngineConfiguration>(XmlArchive &archive, EngineConfiguration& configuration)
		{
			serialize(archive, "window_width", configuration.window_width);
			serialize(archive, "window_height", configuration.window_height);
			serialize(archive, "fullscreen", configuration.fullscreen);
			serialize(archive, "vsync", configuration.vsync);

			serialize(archive, "console_background", configuration.console_background);
			serialize(archive, "console_floating_background", configuration.console_floating_background);
			serialize(archive, "console_number_of_lines", configuration.console_number_of_lines);
			serialize(archive, "console_effect", configuration.console_effect);

			serialize(archive, "text_effect", configuration.text_effect);

			serialize(archive, "location_table", configuration.location_table);
		}
	}
}

#endif // __RENGINE__TYPESERIALIZATION_H__
