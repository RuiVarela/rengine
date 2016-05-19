// __!!rengine_copyright!!__ //

#ifndef __RENGINE_FONT_H__
#define __RENGINE_FONT_H__

#include <rengine/lang/Lang.h>
#include <rengine/state/Texture.h>
#include <rengine/math/Vector.h>

#include <vector>
#include <string>
#include <map>

namespace rengine
{
	typedef std::string String;

	class Font
	{
	public:
		typedef Uint GlyphCode;

		//
		// Glyph
		//
		class Glyph
		{
		public:
			typedef Vector2<Uint> Dimension;
			typedef Vector2<Real> TextureCoords;

			Glyph();
			~Glyph();

			//all dimensions in pixels

			//the horizontal distance from the current cursor position to the leftmost border of the glyph image's bounding box
			Real const& bearingX() const { return bearing_x; }
			void setBearingX(Real const x) { bearing_x = x; }

			//the vertical distance from the current cursor position (on the baseline) to the topmost border of the glyph image's bounding box
			Real const& bearingY() const { return bearing_y; }
			void setBearingY(Real const y) { bearing_y = y; }

			//the horizontal distance used to increment the pen position when the glyph is drawn as part of a string of text
			Real const& advance() const { return glyph_advance; }
			void setAdvance(Real const value) { glyph_advance = value; }

			GlyphCode const& code() const { return glyph_code; }
			void setCode(GlyphCode const value) { glyph_code = value; }

			// Char pixel dimension
			Dimension const& dimension() const { return glyph_dimension; }
			void setDimension(Dimension const& glyph_dimension) { this->glyph_dimension = glyph_dimension; }

			TextureCoords const& textureCoords() const { return texture_coords; }
			void setTextureCoords(TextureCoords const& coords) { texture_coords = coords; }
		private:
			Real bearing_x;
			Real bearing_y;
			Real glyph_advance;
			Dimension glyph_dimension;
			GlyphCode glyph_code;
			TextureCoords texture_coords;
		};

		typedef Vector2<Real> Margin;
		typedef Vector2<Uint> Resolution;
		typedef std::map< GlyphCode, SharedPointer<Glyph> > GlyphMap;

		Font();
		virtual ~Font();

		virtual Glyph* glyph(GlyphCode const glyph_code);

		//
		// Returns a reference glyph for the current font,
		// usefull for discovering the average height and with of the font
		// should return 'X'
		//
		Glyph* referenceGlyph();


		GlyphMap const& glyphMap() const;

		void setResolution(Resolution const& resolution);
		Resolution const& resolution() const;

		// Glyph pixel margin
		void setMarging(Margin const& margin);
		Margin const& margin() const;

		std::string const& name() const;

		SharedPointer<Texture2D> const& texture() const;
	protected:
		GlyphMap glyph_map;
		Resolution resolution_;
		Margin margin_;
		std::string name_;

		SharedPointer<Texture2D> texture_;


		void addGlyph(GlyphCode const code, SharedPointer<Glyph> const& glyph);
	};

} // namespace rengine

#endif //__RENGINE_FONT_H__
