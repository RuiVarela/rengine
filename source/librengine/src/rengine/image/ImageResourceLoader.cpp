// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>

#include <rengine/image/ImageResourceLoader.h>
#include <rengine/image/stb_image.h>

#include <cstring>

namespace rengine
{
	Bool ImageResourceLoader::suportsFormat(std::string const& extension) const
	{
		Bool can_load = false;

		if ((extension == "bmp") || (extension == "png") || (extension == "tga") ||
			(extension == "jpg") || (extension == "jpeg") || (extension == "psd") || (extension == "hdr"))
		{
			can_load = true;
		}

		return can_load;
	}

	SharedPointer<Image> ImageResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<Image> image;

		Int width = 0;
		Int height = 0;
		Int color_channels = 0;
		Uchar* data = stbi_load(location.c_str(), &width, &height, &color_channels, 0);

//		if (data)
//		{
//			image = new Image(width, height, color_channels, data);
//		}

		if (data)
		{
			// TODO
			// for performace shake this should be changed
			//
			// convert to c++ memory, this is done for consistency
			Uchar* image_data = new Uchar[width * height * color_channels];
			memcpy(image_data, data, width * height * color_channels);
			if (image_data)
			{
				image = new Image(width, height, color_channels, image_data);
			}

			rg_free(data);
		}

		return image;
	}


	Bool Texture2DResourceLoader::suportsFormat(std::string const& extension) const
	{
		ImageResourceLoader image_loader;
		return image_loader.suportsFormat(extension);
	}

	SharedPointer<Texture2D> Texture2DResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<Texture2D> texture;

		ImageResourceLoader image_loader;
		SharedPointer<Image> image = image_loader.load(location, options);


		if (image)
		{
			image->makeOpenGlCompliant(true, !CoreEngine::instance()->renderEngine().supportsNonPowerOfTwoTextures());

			texture = new Texture2D();

			if (!options.empty())
			{
				Texture2D::Wrap wrap_s = texture->getWrapS();
				Texture2D::Wrap wrap_t = texture->getWrapS();

				if (options.hasProperty("wrap_s"))
				{
					wrap_s = any_cast<Texture2D::Wrap> (options["wrap_s"].value);
				}

				if (options.hasProperty("wrap_t"))
				{
					wrap_t = any_cast<Texture2D::Wrap> (options["wrap_t"].value);
				}

				texture->setWrap(wrap_s, wrap_t);
			}

			texture->setImage(image);
		}

		return texture;
	}

} //namespace


