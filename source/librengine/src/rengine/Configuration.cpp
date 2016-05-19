// __!!rengine_copyright!!__ //

#include <rengine/Configuration.h>

namespace rengine
{
	EngineConfiguration::EngineConfiguration() :
		window_width(800),
		window_height(600),
		fullscreen(false),
		vsync(false),
		console_background("data/images/console.bmp"),
		console_floating_background("data/images/console_back.bmp"),
		console_number_of_lines(50)
	{
	}

} // namespace rengine
