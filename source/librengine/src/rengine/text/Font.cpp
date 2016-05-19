// __!!rengine_copyright!!__ //

#include <rengine/text/Font.h>

namespace rengine
{
	Font::Glyph::Glyph() :
		bearing_x(0.0f),
		bearing_y(0.0f),
		glyph_advance(0.0f),
		glyph_code(0)
	{
	}

	Font::Glyph::~Glyph()
	{

	}

	Font::Font()
		:resolution_(72, 72),
		name_("Base Font Class")
	{
	}

	Font::~Font()
	{
	}

	Font::GlyphMap const& Font::glyphMap() const
	{
		return glyph_map;
	}

	void Font::setResolution(Font::Resolution const& resolution)
	{
		resolution_ = resolution;
	}

	SharedPointer<Texture2D> const& Font::texture() const
	{
		return texture_;
	}

	Font::Resolution const& Font::resolution() const
	{
		return resolution_;
	}

	void Font::setMarging(Margin const& margin)
	{
		margin_ = margin;
	}

	Font::Margin const& Font::margin() const
	{
		return margin_;
	}

	std::string const& Font::name() const
	{
		return name_;
	}

	Font::Glyph* Font::glyph(GlyphCode const glyph_code)
	{
		Glyph* found_glyph = 0;
		GlyphMap::iterator found = glyph_map.find(glyph_code);

		if (found != glyph_map.end())
		{
			found_glyph = found->second.get();
		}

		return found_glyph;
	}

	Font::Glyph* Font::referenceGlyph()
	{
		Glyph* found_glyph = glyph('X');

		if (!found_glyph)
		{
			found_glyph = glyph('x');
		}

		if (!found_glyph)
		{
			found_glyph = glyph('x');
		}

		if (!found_glyph)
		{
			if (!glyph_map.empty())
			{
				found_glyph = glyph_map.begin()->second.get();
			}

		}

		return found_glyph;
	}


	void Font::addGlyph(GlyphCode const code, SharedPointer<Glyph> const& glyph)
	{
		glyph_map[code] = glyph;
	}

} // namespace rengine
