// __!!rengine_copyright!!__ //

#ifndef __RENGINE_HUD_WRITER_H__
#define __RENGINE_HUD_WRITER_H__

#include <rengine/text/Text.h>

namespace rengine
{
	class HudWriter : public Text
	{
	public:
		HudWriter();
		HudWriter(Metrics const metrics);
		HudWriter(std::string const& font_filename);
		HudWriter(SharedPointer<Font> const& font);
		~HudWriter();

		//
		// A efficient way to write multiple text lines
		// each write call generates more geometry,
		// Do not forget to call Clear()!!!
		//
		void write(String const &message);
		void write(Vector2D const& position, String const &message);

		virtual void draw(RenderEngine& render_engine);
	};
}

#endif //__RENGINE_HUD_WRITER_H__
