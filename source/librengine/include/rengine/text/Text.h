// __!!rengine_copyright!!__ //

#ifndef __RENGINE_TEXT_H__
#define __RENGINE_TEXT_H__

#include <rengine/geometry/Drawable.h>
#include <rengine/text/Font.h>
#include <rengine/math/Vector.h>

namespace rengine
{
	class Text : public Drawable
	{
	public:
		enum Metrics
		{
			CentimeterMetrics,
			PixelMetrics
		};

		Text();
		Text(Metrics const metrics);
		Text(std::string const& font_filename, String const& text);
		Text(SharedPointer<Font> const& font, String const& text);
		~Text();

		virtual void prepareDrawing(RenderEngine& render_engine);
		virtual void unprepareDrawing(RenderEngine& render_engine);
		virtual void updateUniforms(RenderEngine& render_engine);
		virtual void draw(RenderEngine& render_engine);

		void setFont(SharedPointer<Font> const& font);
		Bool setFont(std::string const& font_filename);
		SharedPointer<Font> const& getFont() const;

		String const& getText() const;
		void setText(String const& text);

		void setPosition(Vector3D const& position);
		Vector3D const& getPosition() const;

		void setColor(Vector4D const& color);
		Vector4D const& getColor() const;

		Metrics getMetrics() const;
		void setMetrics(Metrics const metrics);

		void setModelViewProjection(Matrix const mvp);
		Matrix const& modelViewProjection() const;

		//text empty?
		Bool empty();

		void clear();

		static Vector2<Real> screenCentimeterDimensions(Real dpi = 96.0f);
		static Vector3<Real> screenPositionPixels(Real const pixels_x, Real const pixels_y, Real dpi = 96.0f);

		static void setDefaultEffectFile(std::string const& filename);
		static std::string defaultEffectFile();
	private:
		SharedPointer<Font> font_;
		String text_;
		Vector3D position_;
		Vector4D color_;
		Metrics metrics_;
		Matrix mvp_;

		void initialize();

	protected:
		void addGlyphGeometry(Vector3D const& position, Vector3D const& length, Vector2D const& texture_position, Vector2D const& texture_length);
		void beginGeometry();
		void addGeometry(Vector3D const& position, String const& text);

		VertexBuffer vertex_buffer_;
		VertexBufferObject vertex_vbo_;

		static std::string effect_file_;
	};

	//implementation

	RENGINE_INLINE String const& Text::getText() const
	{
		return text_;
	}

	RENGINE_INLINE SharedPointer<Font> const& Text::getFont() const
	{
		return font_;
	}

	RENGINE_INLINE Bool Text::empty()
	{
		return text_.empty();
	}

	RENGINE_INLINE Vector3D const& Text::getPosition() const
	{
		return position_;
	}

	RENGINE_INLINE Vector4D const& Text::getColor() const
	{
		return color_;
	}

	RENGINE_INLINE Text::Metrics Text::getMetrics() const
	{
		return metrics_;
	}

	RENGINE_INLINE Matrix const& Text::modelViewProjection() const
	{
		return mvp_;
	}

	RENGINE_INLINE void Text::setDefaultEffectFile(std::string const& filename)
	{
		effect_file_ = filename;
	}

	RENGINE_INLINE std::string Text::defaultEffectFile()
	{
		return effect_file_;
	}

}

#endif //__RENGINE_TEXT_H__



