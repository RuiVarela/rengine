// __!!rengine_copyright!!__ //

#include <rengine/text/Fonts.h>
#include <rengine/lang/Lang.h>

#include <cstdlib>
#include <cstring>
#include <climits>
#include <fstream>
#include <sstream>

//fntheader.type
#define FNT_TYPE_VECTOR			0x0001	// If not set, it is a raster font.
#define FNT_TYPE_MEMORY			0x0004
#define FNT_TYPE_DEVICE			0x0080

//fntheader.pitchfamily
#define FNT_PITCH_VARIABLE		0x01	// Variable width font
#define FNT_FAMILY_MASK			0xf0
#define FNT_FAMILY_DONTCARE		0x00
#define FNT_FAMILY_SERIF		0x10
#define FNT_FAMILY_SANSSERIF	0x20
#define FNT_FAMILY_FIXED		0x30
#define FNT_FAMILY_SCRIPT		0x40
#define FNT_FAMILY_DECORATIVE	0x50

//fntheader.flags
#define FNT_FLAGS_FIXED			0x0001
#define FNT_FLAGS_PROPORTIONAL	0x0002
#define FNT_FLAGS_ABCFIXED		0x0004
#define FNT_FLAGS_ABCPROP		0x0008
#define FNT_FLAGS_1COLOR		0x0010
#define FNT_FLAGS_16COLOR		0x0020
#define FNT_FLAGS_256COLOR		0x0040
#define FNT_FLAGS_RGBCOLOR		0x0080

#define FON_MZ_MAGIC			0x5A4D
#define FON_NE_MAGIC			0x454E

using namespace rengine;

struct FontChar
{
	FontChar()
	{
		width = 0;
		offset = 0;
	}

	Uint16 width;
	Uint32 offset;
};

struct WinfontHeader
{
    Uint16	version;		// Either 0x200 or 0x300
    Uint32	filesize;
    Char	copyright[60 + 1];
    Uint16	type;
    Uint16	pointsize;
    Uint16	vertres;		// Vertical resolution of font
    Uint16	hortres;		// Horizontal resolution of font
    Uint16	ascent;
    Uint16	internal_leading;
    Uint16	external_leading;
    Uint8	italic;			// Set to 1 for italic fonts
    Uint8	underline;		// Set to 1 for underlined fonts
    Uint8	strikeout;		// Set to 1 for strikeout fonts
    Uint16	weight;			// 1-1000 windows weight value
    Uint8	charset;
    Uint16	width;			// Fixed width font if not 0, width of all chars
    Uint16	height;			// Height of font bounding box
    Uint8	pitchfamily;
    Uint16	avgwidth;		// Width of "X"
    Uint16	maxwidth;
    Uint8	firstchar;
    Uint8	lastchar;
    Uint8	defchar;
    Uint8	breakchar;
    Uint16	widthbytes;		// Number of bytes in a row
    Uint32	deviceoffset;	// set to 0
    Uint32	faceoffset;		// Offset from start of file to face name (C string)
    Uint32	bitspointer;	// set to 0
    Uint32	bitsoffset;		// Offset from start of file to start of bitmap info
    Uint8	padding_00;
    Uint32	flags;
    Uint16	aspace;
    Uint16	bspace;
    Uint16	cspace;
    Uint32	coloroffset;	// Offset to color table
    Uint8	padding_01[16];

    FontChar chars[256];

    WinfontHeader();
    Bool load(std::istream& in);

};

WinfontHeader::WinfontHeader()
{
	memset(this, 0, sizeof(WinfontHeader));
}

static void read8(std::istream& in, Char* data)
{
	in.read(data, 1);
}

static void read16(std::istream& in, Char* data)
{
	in.read(data, 2);
}

static void read32(std::istream& in, Char* data)
{
	in.read(data, 4);
}

Bool WinfontHeader::load(std::istream& in)
{
	read16(in, (Char*)&version);
	if ((version != 0x200) && (version != 0x300))
	{
		return false;
	}
	read32(in, (Char*)&filesize);

	for(Uint i = 0; i != 60; ++i)
	{
		read8(in, &copyright[i]);
	}
	copyright[60] = '\0';


	read16(in, (Char*)&type);

    if (type & (FNT_TYPE_VECTOR | FNT_TYPE_MEMORY | FNT_TYPE_DEVICE))
    {
    	return false;
    }

    read16(in, (Char*)&pointsize);
    read16(in, (Char*)&vertres);
    read16(in, (Char*)&hortres);
    read16(in, (Char*)&ascent);
    read16(in, (Char*)&internal_leading);
    read16(in, (Char*)&external_leading);
    read8(in, (Char*)&italic);
    read8(in, (Char*)&underline);
    read8(in, (Char*)&strikeout);
    read16(in, (Char*)&weight);
    read8(in, (Char*)&charset);
    read16(in, (Char*)&width);
    read16(in, (Char*)&height);
    read8(in, (Char*)&pitchfamily);
    read16(in, (Char*)&avgwidth);
    read16(in, (Char*)&maxwidth);
    read8(in, (Char*)&firstchar);
    read8(in, (Char*)&lastchar);
    read8(in, (Char*)&defchar);
    read8(in, (Char*)&breakchar);
    read16(in, (Char*)&widthbytes);
    read32(in, (Char*)&deviceoffset);
    read32(in, (Char*)&faceoffset);
    read32(in, (Char*)&bitspointer);
    read32(in, (Char*)&bitsoffset);
    read8(in, (Char*)&padding_00);

	if (version == 0x300)
	{
		read32(in, (Char*)&flags);

		if (flags & (FNT_FLAGS_ABCFIXED | FNT_FLAGS_ABCPROP | FNT_FLAGS_16COLOR | FNT_FLAGS_256COLOR | FNT_FLAGS_RGBCOLOR))
		{
			return false;
		}

	    read16(in, (Char*)&aspace);
	    read16(in, (Char*)&bspace);
	    read16(in, (Char*)&cspace);
	    read32(in, (Char*)&coloroffset);

		for (Uint i = 0; i != 16; ++i)
		{
			read8(in, ((Char*) &padding_01[i]));
		}
	}


	for (Uint8 i = firstchar; i != lastchar; ++i)
	{
		read16(in, (Char*)&chars[i].width);

		if (version == 0x200)
		{
			read16(in, (Char*)&chars[i].offset);
		}
		else
		{
			read32(in, (Char*)&chars[i].offset);
		}
	}


	return true;
}

//
// WinFontClass
//

namespace rengine
{
	Winfont::Winfont()
	{
		first_glyph = ' ';
		last_glyph = '~';
	}

	Winfont::~Winfont()
	{
	}

	void Winfont::setFirstGlyphCode(Font::GlyphCode const& code)
	{
		first_glyph = code;
	}

	void Winfont::setLastGlyphCode(Font::GlyphCode const& code)
	{
		last_glyph = code;

	}

	Bool Winfont::load(std::istream& in)
	{
		typedef std::ifstream::pos_type Position;

		Position initial_position = in.tellg();

		WinfontHeader header;
		if (!header.load(in))
		{
			return false;
		}

		//
		// Load Name
		//
		in.seekg(initial_position + Position(header.faceoffset), std::ios::beg);

		std::stringstream string_stream;
		Char current_byte = 0;
		read8(in, &current_byte);
		while (current_byte != '\0')
		{
			string_stream << current_byte;
			read8(in, &current_byte);
		}

		string_stream << ' ' << Int(header.weight);
		if (header.italic)
		{
			string_stream << " italic";
		}

		if (header.underline)
		{
			string_stream << " underline";
		}

		if (header.strikeout)
		{
			string_stream << " strikeout";
		}

		if ((header.hortres == 0) || (header.vertres == 0))
		{
			header.hortres = 96;
			header.vertres = 96;
		}

		setResolution(Resolution(header.hortres, header.vertres));
		name_ = string_stream.str();


		Position letter_load_position = in.tellg();

		//
		// Find the best texture to hold the chars
		//
		Uint8 begin = header.firstchar;
		Uint8 end = header.lastchar;

		if ( (first_glyph >= begin) && (first_glyph <= end) )
		{
			begin = first_glyph;
		}

		if ( (last_glyph >= begin) && (last_glyph <= end) )
		{
			end = last_glyph;
		}

		BitmapBuilder map_builder(header.height, margin());

#ifdef EXPORT_C_FONT
		std::ofstream file_out("dump.c");
		if (file_out.is_open())
		{
			file_out << "// font chars [" << Int(begin) << " - " << Int(end) << "]" << std::endl;
			file_out << "// size [" << header.width << " , " << header.height << "]" << std::endl;
			file_out << "Uchar char_data[" << Int(end) - Int(begin) << "][" << header.height << "] = { " << std::endl;;
			file_out << std::hex;
		}
#endif //EXPORT_C_FONT

		//A point is a physical distance, equaling 1/72th of an inch. Normally, it is not equivalent to a pixel.
		//
		//The horizontal and vertical device resolutions are expressed in dots-per-inch, or dpi.
		//Normal values are 72 or 96 dpi for display devices like the screen.
		//The resolution is used to compute the character pixel size from the character point size.

		// 1 inch = 2.54 centimeters
		//Real const inch_to_cm = 2.54f;
		//Real pixel_size_x = inch_to_cm / Real(header.hortres);
		//Real pixel_size_y = inch_to_cm / Real(header.vertres);
		Real pixel_size_x = 1.0f;
		Real pixel_size_y = 1.0f;

		for (Uint8 i = begin; i <= end; ++i)
		{
			if (header.chars[i].width != 0)
			{
				BitmapBuilder::CharData element;
				element.glyph = new Glyph();
				element.glyph->setCode(i);
				element.glyph->setDimension(Glyph::Dimension(header.chars[i].width, header.height));

				element.glyph->setAdvance( Real(header.chars[i].width + header.aspace + header.cspace) * pixel_size_x );
				element.glyph->setBearingX( Real(header.aspace) * pixel_size_x );
				element.glyph->setBearingY( Real(header.ascent) * pixel_size_y );

//				std::cout << " Ascent : " << header.ascent
//						  << " Height : " << header.height
//						  << " aspace : " << header.aspace
//						  << " bspace : " << header.bspace
//						  << " cspace : " << header.cspace << std::endl;

				Uint bytes_per_line = ((header.chars[i].width - 1) >> 3) + 1;

				element.data = new Uint8[bytes_per_line * header.height];
				in.seekg(initial_position + Position(header.chars[i].offset), std::ios::beg);
				element.empty = true;



				for (Uint8 column = 0; column != bytes_per_line; ++column)
				{
					for (Uint8 line = 0; line != header.height; ++line)
					{
						Uint8 byte = 0;
						read8(in, (Char*)&byte);
						element.data[column + line * bytes_per_line] = byte;
						element.empty &= (byte == 0);
					}
				}

#ifdef EXPORT_C_FONT
				if (file_out.is_open())
				{
					file_out << "{";
					for (Uint8 line = 0; line != header.height; ++line)
					{
						file_out << "0x" << std::setfill('0') << std::setw(2)
						<< Int( element.data[line] ) << ((line == (header.height - 1)) ? "" : ", ");
					}

					file_out << ((i == end) ? "}" : "}, ") << std::endl;
				}
#endif //EXPORT_C_FONT


//				std::cout << i << " width " << Int(header.chars[i].width)
//								<< " " << bytes_per_line << " " << header.height << " " << element.empty << std::endl;
//
				map_builder.addChar(element);
			}
		}

#ifdef EXPORT_C_FONT
		if (file_out.is_open())
		{
			file_out << "};";
			file_out.close();
		}
#endif //EXPORT_C_FONT

		texture_ = map_builder.buildTexture();

		for (BitmapBuilder::CharTable::iterator line = map_builder.charTable().begin(); line != map_builder.charTable().end(); ++line)
		{
			for (BitmapBuilder::CharVector::iterator current_char = line->begin(); current_char != line->end(); ++current_char)
			{
				addGlyph(current_char->glyph->code(), current_char->glyph);
			}
		}

		//std::cout << "Name : '" << name() << "'" << std::endl;
		return true;
	}


	Bool Winfont::load(std::string const& filename)
	{
		Bool state = false;

		std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open())
		{
			return state;
		}

		Uint16 magic = 0;
		read16(in, (Char*) &magic);
		if ((magic == 0x200) || (magic == 0x300))
		{
			in.seekg(0, std::ios::beg);
			state = load(in);
		}
		else if (magic == FON_MZ_MAGIC)
		{
			in.seekg(30 * 2, std::ios::beg);
			Uint32 offset = 0;
			read32(in, (Char*) &offset);

			in.seekg(offset, std::ios::beg);
			magic = 0;
			read16(in, (Char*) &magic);

			if (magic == FON_NE_MAGIC)
			{
				in.seekg(34, std::ios::cur);

				Uint32 fonts_offset = 0;
				Uint32 fonts_end = 0;

				read16(in, (Char*) &fonts_offset);
				read16(in, (Char*) &fonts_end);

				fonts_offset += offset;
				fonts_end += offset;

				in.seekg(fonts_offset, std::ios::beg);
				Uint32 shift_size = 0;
				read16(in, (Char*) &shift_size);

				//
				// Count fonts
				//

				Int number_of_fonts = 0;

				Bool seeking = true;
				while (seeking)
				{
					Uint16 id;
					read16(in, (Char*) &id);

					if (id == 0)
					{
						seeking = false;
					}
					else
					{
						Uint16 count;
						read16(in, (Char*) &count);

						if (id == 0x8008)
						{
							number_of_fonts = count;
							in.seekg(4, std::ios::cur);
							seeking = false;
						}
						else
						{
							in.seekg(4 + count * 12, std::ios::cur);


							if ( (Int32(in.tellg()) == -1) ||  (Uint32(in.tellg()) >= fonts_end) )
							{
								seeking = false;
							}
						}
					}
				}

				//
				// load fonts
				//
				if (number_of_fonts)
				{
					// we only load the first_font for now
					Uint const max_loaded_fonts = 1;

					for (Uint i = 0; i != max_loaded_fonts; ++i)
					{
						std::ifstream::pos_type position = in.tellg();
						offset = 0;
						read16(in, (Char*) &offset);
						offset = offset << shift_size;

						in.seekg(offset, std::ios::beg);

						state = load(in);
						in.seekg(position, std::ios::beg);
					}
				}
			}
		}

		in.close();

		return state;
	}

} // namespace rengine
