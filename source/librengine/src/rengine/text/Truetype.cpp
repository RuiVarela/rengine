// __!!rengine_copyright!!__ //

#include <rengine/text/Fonts.h>
#include <rengine/lang/Lang.h>
#include <fstream>

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)  rg_malloc(x)
#define STBTT_free(x,u)    rg_free(x)
#include <rengine/text/stb_truetype.h>

namespace rengine
{
	Truetypefont::Truetypefont()
	{
		pixel_height = 12.0f;
		first_glyph = ' ';
		last_glyph = '~';
	}

	Truetypefont::~Truetypefont()
	{
	}

	Bool Truetypefont::load(std::string const& filename)
	{
		SharedArray<Uchar> ttf;

		Bool is_good = true;
		std::ifstream in(filename.c_str(), std::ios::in | std::ios::ate | std::ios::binary);
		is_good = in.good();

		if (is_good)
		{
			Ulong end_position = Ulong(in.tellg());
			ttf = new Uchar[end_position];

			in.seekg(0, std::ios::beg);
			in.read((char*)ttf.get(), end_position);
			is_good = in.good();
			in.close();
		}

		SharedArray<stbtt_bakedchar> baked_data = new stbtt_bakedchar[last_glyph - first_glyph];
		Int const texture_width = 256;
		Uchar* data = ttf.get();
		Int const offset = 0; 
		SharedPointer<Image> pixels = new Image(texture_width, texture_width, 1);


		int result = stbtt_BakeFontBitmap(data, offset,													// font location (use offset=0 for plain .ttf);
										  pixel_height,													// height of font in pixels
										  pixels->getData(), pixels->getWidth(), pixels->getHeight(),	// bitmap to be filled in
										  first_glyph, last_glyph - first_glyph,						// characters to bake
										  baked_data.get());											// you allocate this, it's num_chars long
		// if return is positive, the first unused row of the bitmap
		// if return is negative, returns the negative of the number of characters that fit
		// if return is 0, no characters fit and no rows were used
		// This uses a very crappy packing.




		Real ipw = 1.0f / Real(pixels->getWidth()); 
		Real iph = 1.0f / Real(pixels->getHeight()); 

		for (GlyphCode code = first_glyph; code <= last_glyph; ++code)
		{
			GlyphCode current = code - first_glyph;

			//float x = 0;
			//float y = 0;
			//stbtt_aligned_quad q;
			//stbtt_GetBakedQuad(baked_data.get(), pixels->getWidth(), pixels->getHeight(), current, &x,&y,&q,1);//1=opengl,0=old d3d
			//glyph->setDimension(Glyph::Dimension(q.x1 - q.x0, q.y1 - q.y0));
			//glyph->setTextureCoords(Glyph::TextureCoords(q.s0, q.t0));
			//

			SharedPointer<Glyph> glyph = new Glyph();
			glyph->setCode(code);



			stbtt_bakedchar* baked = baked_data.get() + current;
			Uint width = baked->y1 - baked->y0;
			glyph->setDimension(Glyph::Dimension(baked->x1 - baked->x0, width));

			int round_x = ((int) floor((baked->xoff) + 0.5));
			int round_y = ((int) floor((baked->yoff) + 0.5));


			glyph->setTextureCoords(Glyph::TextureCoords(Real(baked->x0) * ipw, 
													     1.0f - Real(baked->y0) * iph - Real(width) * iph));
			glyph->setAdvance(baked->xadvance);
			glyph->setBearingX(baked->xoff);
			glyph->setBearingY(-baked->yoff);

			addGlyph(code, glyph);
		}

		setResolution(Resolution(96, 96));
		pixels->flip(Image::FlipVertical);

		texture_ = new Texture2D(pixels);





		return true;
	}
	
	void Truetypefont::setPixelHeight(Real const& pixel_height)
	{
		this->pixel_height = pixel_height;
	}

	void Truetypefont::setFirstGlyphCode(Font::GlyphCode const& code)
	{
		first_glyph = code;
	}

	void Truetypefont::setLastGlyphCode(Font::GlyphCode const& code)
	{
		last_glyph = code;

	}
}