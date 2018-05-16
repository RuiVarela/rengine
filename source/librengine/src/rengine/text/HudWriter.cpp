// __!!rengine_copyright!!__ //

#include <rengine/text/HudWriter.h>
#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>

namespace rengine
{
	HudWriter::HudWriter()
	:Text(PixelMetrics)
	{
		Drawable::setDrawMode(DynamicDraw);
	}

	HudWriter::HudWriter(Metrics const metrics)
	:Text(metrics)
	{
		Drawable::setDrawMode(DynamicDraw);
	}

	HudWriter::HudWriter(std::string const& font_filename)
	:Text(font_filename, "")
	{
		Drawable::setDrawMode(DynamicDraw);
		setMetrics(PixelMetrics);
	}

	HudWriter::HudWriter(SharedPointer<Font> const& font)
	:Text(font, "")
	{
		Drawable::setDrawMode(DynamicDraw);
		setMetrics(PixelMetrics);
	}

	HudWriter::~HudWriter()
	{

	}

	void HudWriter::draw(RenderEngine& render_engine)
	{
		if (!vertex_buffer_.empty() && ((drawMode() == DynamicDraw) || (drawMode() == StaticDraw) || (drawMode() == StreamDraw)) )
		{
			if (needsDataRefresh())
			{
				prepareDrawing(render_engine);
				needs_data_refresh = false;
			}

			render_engine.drawVertexArrayObject(vertex_vao_, vertex_buffer_);
		}
	}

	void HudWriter::write(String const &message)
	{
		needs_data_refresh = true;
		addGeometry(getPosition(), message);
	}

	void HudWriter::write(Vector2D const& position, String const &message)
	{
		needs_data_refresh = true;
		addGeometry(Vector3D(position, 0.0f) , message);
	}
}

