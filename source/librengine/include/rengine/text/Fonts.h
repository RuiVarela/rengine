// __!!rengine_copyright!!__ //

#ifndef __RENGINE_FONTS_H__
#define __RENGINE_FONTS_H__

#include <rengine/text/Font.h>
#include <rengine/image/Image.h>

#include <iosfwd>

namespace rengine
{
	//
	// Console Style Font
	// Fixed With 8 * 15,  chars :[32, 126]
	//
	class ConsoleFont : public Font
	{
	public:
		ConsoleFont();
		virtual ~ConsoleFont();

		Bool load();
	};

	//
	// Windows .Fon loader
	//
	class Winfont : public Font
	{
	public:
		Winfont();
		virtual ~Winfont();
		//
		// Load the first winfont from a .fon file
		//
		Bool load(std::string const& filename);
		//
		// Load a winfont from a stream
		//
		Bool load(std::istream& in);

		// Only load glyphs from 'first' to 'last'
		void setFirstGlyphCode(Font::GlyphCode const& code);
		void setLastGlyphCode(Font::GlyphCode const& code);
	private:
		Font::GlyphCode first_glyph;
		Font::GlyphCode last_glyph;
	};

	//
	// Windows .Fon loader
	//
	class Truetypefont : public Font
	{
	public:
		Truetypefont();
		virtual ~Truetypefont();
	
		Bool load(std::string const& filename);

		// Only load glyphs from 'first' to 'last'
		void setFirstGlyphCode(Font::GlyphCode const& code);
		void setLastGlyphCode(Font::GlyphCode const& code);
		void setPixelHeight(Real const& pixel_height);
	private:
		Font::GlyphCode first_glyph;
		Font::GlyphCode last_glyph;
		Real pixel_height;
	};

	//
	// This class builds a texture with glyph bitmapdata
	//
	class BitmapBuilder
	{
	public:
		struct CharData
		{
			CharData();
			rengine::SharedPointer<rengine::Font::Glyph> glyph;
			Uchar* data;
			Bool empty;
		};

		typedef std::vector<CharData> CharVector;
		typedef std::vector<CharVector> CharTable;
		typedef std::vector<Uint> FreePixels;

		BitmapBuilder(Uint height, Font::Margin const glyph_margin);
		~BitmapBuilder();

		void expand(Uint next_size);
		Uint biggestFreeSlot(Uint& index);

		void addChar(CharData const& new_char);

		rengine::SharedPointer<rengine::Texture2D> buildTexture();

		CharTable& charTable();

		void setTargetImageChannels(Uint const channels);
	private:
		void rasterChar(Uint x, Uint y, CharData current, SharedPointer<Image> image);

		Uint font_height;
		Uint texture_size;
		CharTable table;
		FreePixels free_pixels;
		Font::Margin margin;

		Uint image_channels;
	};

} //namespace rengine

#endif //__RENGINE_FONTS_H__
