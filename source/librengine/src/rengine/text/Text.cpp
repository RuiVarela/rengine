// __!!rengine_copyright!!__ //

#include <rengine/text/Text.h>
#include <rengine/CoreEngine.h>
#include <rengine/outputstream/OutputStream.h>
#include <rengine/RenderEngine.h>
#include <rengine/Configuration.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/state/BaseStates.h>
#include <rengine/state/Program.h>
#include <rengine/geometry/VertexDeclaration.h>
#include <rengine/lang/exception/BaseExceptions.h>

namespace rengine
{
	std::string Text::effect_file_ = "";

	Vector2<Real> Text::screenCentimeterDimensions(Real dpi)
	{
		// 1 inch = 2.54 centimeters
		Vector2<Real> dimensions;

		Real const window_width = (Real) CoreEngine::instance()->mainWindow()->contextOptions().width;
		Real const window_height = (Real) CoreEngine::instance()->mainWindow()->contextOptions().height;

		dimensions.x() = (window_width / dpi) * 2.54f;
		dimensions.y() = (window_height / dpi) * 2.54f;

		return dimensions;
	}

	Vector3<Real> Text::screenPositionPixels(Real const pixels_x, Real const pixels_y, Real dpi)
	{
		Vector2<Real> dimensions = screenCentimeterDimensions(dpi);

		Real const window_width = (Real) CoreEngine::instance()->mainWindow()->contextOptions().width;
		Real const window_height = (Real) CoreEngine::instance()->mainWindow()->contextOptions().height;

		dimensions *= (pixels_x / window_width);
		dimensions *= (pixels_y / window_height);

		return Vector3D(dimensions, 0.0f);
	}

	void Text::initialize()
	{
		PositionTextureVertexDeclaration::configure(vertex_buffer_);

		setDrawMode(Drawable::StaticDraw);

		states()->setState(new BlendFunction());

		std::string effect_filename = defaultEffectFile();
		SharedPointer<Program> loaded = CoreEngine::instance()->resourceManager().load<Program>( effect_filename );

		if (!loaded)
		{
			throw GraphicsException(601, "Unable to load Text effect: " + effect_filename);
		}

		states()->setProgram(loaded);

		setPosition(Vector3D(0.0f, 0.0f, 0.0f));
		setColor(Vector4D(1.0f, 1.0f, 1.0f, 1.0f));
		setMetrics(CentimeterMetrics);
	}

	Text::Text(Metrics const metrics)
	{
		initialize();
		setMetrics(metrics);
		setFont("Default");
	}

	Text::Text()
	{
		initialize();
		setFont("Default");
	}

	Text::Text(std::string const& font_filename, String const& text)
	:text_(text)
	{
		initialize();
		setFont(font_filename);
	}

	Text::Text(SharedPointer<Font> const& font, String const& text)
	:text_(text)
	{
		initialize();
		setFont(font);
	}

	Text::~Text()
	{
		// TODO : this should be implemented with an observer
		unprepareDrawing( CoreEngine::instance()->renderEngine()  );
	}

	void Text::setColor(Vector4D const& color)
	{
		color_ = color;
	}

	void Text::setPosition(Vector3D const& position)
	{
		position_ = position;
		needs_data_refresh = true;
	}

	void Text::setModelViewProjection(Matrix const mvp)
	{
		mvp_ = mvp;
	}

	void Text::setFont(SharedPointer<Font> const& font)
	{
		font_ = font;

		SharedPointer<Texture2DUnit> texture_unit = new Texture2DUnit(font->texture());
		states()->clearState(DrawStates::Texture2D);
		states()->setState(texture_unit);

		needs_data_refresh = true;
	}

	void Text::setMetrics(Metrics const metrics)
	{
		Matrix projection;
		if (metrics == Text::CentimeterMetrics)
		{
			// screen = 96 dpi, 1 inch = 2.54 centimeters
			Vector2D dimension = Text::screenCentimeterDimensions();
			projection = Matrix::ortho2D(0, dimension.x(), 0, dimension.y());
		}
		else
		{
			projection = Matrix::ortho2D(0, Real(CoreEngine::instance()->mainWindow()->contextOptions().width),
										 0, Real(CoreEngine::instance()->mainWindow()->contextOptions().height));
		}
		setModelViewProjection(projection);

		metrics_ = metrics;
		needs_data_refresh = true;
	}

	Bool Text::setFont(std::string const& font_filename)
	{
		SharedPointer<Font> loaded = CoreEngine::instance()->resourceManager().load<Font>(font_filename);

		if (loaded)
		{
			setFont(loaded);
		}

		return loaded;
	}

	void Text::setText(String const& text)
	{
		text_ = text;
		needs_data_refresh = true;
	}

	void Text::prepareDrawing(RenderEngine& render_engine)
	{
		if (!vertex_buffer_.empty())
		{
			render_engine.loadVertexBufferObject(vertex_vbo_,  vertex_buffer_, drawMode());
		}

		needs_prepare_rendering = false;
	}

	void Text::unprepareDrawing(RenderEngine& render_engine)
	{
		render_engine.unloadVertexBufferObject(vertex_vbo_);
	}

	void Text::updateUniforms(RenderEngine& render_engine)
	{
		if (states()->hasState(DrawStates::Program))
		{
			SharedPointer<Program> program = states()->getProgram();
			program->uniform("mvp").set(mvp_);
			program->uniform("color").set(color_);
			program->uniform("texture").set(0);
		}
	}

	void Text::draw(RenderEngine& render_engine)
	{
		if (!empty())
		{
			if (needsDataRefresh())
			{
				vertex_buffer_.clear();
				addGeometry(position_, text_);

				prepareDrawing(render_engine);

				needs_data_refresh = false;
			}

			render_engine.bindVertexBufferObject(vertex_vbo_,  vertex_buffer_);
			render_engine.drawVertexBufferObject(vertex_vbo_, vertex_buffer_);
			render_engine.unbindVertexBufferObject(vertex_vbo_,  vertex_buffer_);
		}
	}

	void Text::clear()
	{
		vertex_buffer_.clear();
		setText("");
	}

	void Text::addGeometry(Vector3D const& position, String const& text)
	{
		if (font_ && !text.empty())
		{
			//
			// To convert image pixels to cm (CentimeterMetrics)
			//
			Real pixel_size_x = 1.0f;
			Real pixel_size_y = 1.0f;

			if (metrics_ == CentimeterMetrics)
			{
				// 1 inch = 2.54 centimeters
				Real const inch_to_cm = 2.54f;
				pixel_size_x = inch_to_cm / Real(font_->resolution().x());
				pixel_size_y = inch_to_cm / Real(font_->resolution().y());
			}

			Vector3D current_position = position;
			Vector3D length;

			for (String::const_iterator i = text.begin(); i != text.end(); ++i)
			{
				Font::Glyph* glyph = font_->glyph(Font::GlyphCode(*i));

				if (glyph)
				{
					length.x() = pixel_size_x * glyph->dimension().x();
					length.y() = pixel_size_y * glyph->dimension().y();

					current_position.x() += pixel_size_x * glyph->bearingX();
					current_position.y() = position.y() - length.y() + glyph->bearingY() * pixel_size_y;

					Font::Glyph::TextureCoords texture_length(Real(glyph->dimension().x()) / Real(font_->texture()->getWidth()),
															  Real(glyph->dimension().y()) / Real(font_->texture()->getHeight()));

					addGlyphGeometry(current_position, length, glyph->textureCoords(), texture_length);
					current_position.x() += glyph->advance() * pixel_size_x - glyph->bearingX() * pixel_size_x;

					// current_position.y() += glyph->bearingX();
				}
			}
		}
	}

	void Text::addGlyphGeometry(Vector3D const& position, Vector3D const& length, Vector2D const& texture_position, Vector2D const& texture_length)
	{
		PositionTextureVertexDeclaration vertex;
		VertexBuffer::Interface<PositionTextureVertexDeclaration> vertex_stream = vertex_buffer_.interface<PositionTextureVertexDeclaration>();

		// _____
		// |2 /|
		// | / |
		// |/ 1|
		// -----
		//


		//
		// 2
		//
		vertex.tex_coord = texture_position;
		vertex.position = position;
		vertex_stream.add(vertex);


		vertex.tex_coord.x() = texture_position.x() + texture_length.x();
		vertex.tex_coord.y() = texture_position.y();
		vertex.position.x() = position.x() + length.x();
		vertex.position.y() = position.y();
		vertex.position.z() = position.z();
		vertex_stream.add(vertex);


		vertex.tex_coord.x() = texture_position.x() + texture_length.x();
		vertex.tex_coord.y() = texture_position.y() + texture_length.y();
		vertex.position.x() = position.x() + length.x();
		vertex.position.y() = position.y() + length.y();
		vertex.position.z() = position.z();
		vertex_stream.add(vertex);

		//
		// 1
		//

		vertex.tex_coord.x() = texture_position.x() + texture_length.x();
		vertex.tex_coord.y() = texture_position.y() + texture_length.y();
		vertex.position.x() = position.x() + length.x();
		vertex.position.y() = position.y() + length.y();
		vertex.position.z() = position.z();
		vertex_stream.add(vertex);


		vertex.tex_coord.x() = texture_position.x();
		vertex.tex_coord.y() = texture_position.y() + texture_length.y();
		vertex.position.x() = position.x();
		vertex.position.y() = position.y() + length.y();
		vertex.position.z() = position.z();
		vertex_stream.add(vertex);

		vertex.tex_coord = texture_position;
		vertex.position = position;
		vertex_stream.add(vertex);
	}
}

