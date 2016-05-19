// __!!rengine_copyright!!__ //

#include <rengine/text/FontResourceLoader.h>
#include <rengine/text/Fonts.h>
#include <rengine/file/File.h>
#include <rengine/string/String.h>

namespace rengine
{
	Bool DefaultFontsResourceLoader::suportsFormat(std::string const& extension) const
	{
		Bool can_load = false;

		if (extension == "Default")
		{
			can_load = true;
		}

		return can_load;
	}

	Bool DefaultFontsResourceLoader::canLoadResourceFromLocation(std::string const& resource_location) const
	{
		return suportsFormat(resource_location);
	}

	SharedPointer<Font> DefaultFontsResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<Font> font;

		if (location == "Default")
		{
			Font::Margin margin(1.0f, 1.0f);

			if (options.hasProperty("margin"))
			{
				margin = any_cast<Font::Margin>(options["margin"].value);
			}

			SharedPointer<ConsoleFont> console_font = new ConsoleFont();
			console_font->setMarging(margin);

			if (console_font->load())
			{
				font = console_font;
			}
		}

		return font;
	}


	Bool WinfontResourceLoader::suportsFormat(std::string const& extension) const
	{
		return equalCaseInsensitive(extension, "fon");
	}

	SharedPointer<Font> WinfontResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<Winfont> font = new Winfont();

		if (options.hasProperty("margin"))
		{
			Font::Margin margin = any_cast<Font::Margin>(options["margin"].value);
			font->setMarging(margin);
		}

		if (options.hasProperty("first_glyph_code"))
		{
			Font::GlyphCode code = any_cast<Font::GlyphCode>(options["first_glyph_code"].value);
			font->setFirstGlyphCode(code);
		}

		if (options.hasProperty("last_glyph_code"))
		{
			Font::GlyphCode code = any_cast<Font::GlyphCode>(options["last_glyph_code"].value);
			font->setLastGlyphCode(code);
		}

		if (!font->load(location))
		{
			font = 0;
		}

		return font;
	}


	
	Bool TruetypeFontResourceLoader::suportsFormat(std::string const& extension) const
	{
		return equalCaseInsensitive(extension, "ttf");
	}

	SharedPointer<Font> TruetypeFontResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{

		SharedPointer<Truetypefont> font = new Truetypefont();
		if (options.hasProperty("first_glyph_code"))
		{
			Font::GlyphCode code = any_cast<Font::GlyphCode>(options["first_glyph_code"].value);
			font->setFirstGlyphCode(code);
		}

		if (options.hasProperty("last_glyph_code"))
		{
			Font::GlyphCode code = any_cast<Font::GlyphCode>(options["last_glyph_code"].value);
			font->setLastGlyphCode(code);
		}

		if (options.hasProperty("pixel_height"))
		{
			Real pixel_height = any_cast<Font::GlyphCode>(options["pixel_height"].value);
			font->setPixelHeight(pixel_height);
		}

		if (!font->load(location))
		{
			font = 0;
		}

		return font;
	}


} // namespace rengine
