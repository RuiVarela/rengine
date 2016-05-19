// __!!rengine_copyright!!__ //

#include <rengine/image/Image.h>

#include <sstream>
#include <cmath>
#include <cstring>
#include <cstdlib>

namespace rengine
{
	Rectanglei Image::letterbox(Rectanglei const& src, Rectanglei const& dst)
	{
		Int64 src_width  = Int64(src.width());
		Int64 src_height = Int64(src.height());
		Int64 dst_width  = Int64(dst.width());
		Int64 dst_height = Int64(dst.height());

		Int64 width;
		Int64 height;

		if ( ((src_width * dst_height) / src_height) <= dst_width) 
		{
			// Column letterboxing ("pillar box")
			width = (dst_height * src_width) / src_height;
			height = dst_height;
		}
		else 
		{
			// Row letterboxing.
			width  = dst_width;
			height = (dst_width * src_height) / src_width;
		}

		// Create a centered rectangle within the current destination rect
		Int64 left = dst.left() + ((dst_width - width) / 2);
		Int64 bottom = dst.bottom() + ((dst_height - height) / 2);

		return Rectanglei(Int(left), Int(bottom), Int(left + width), Int(bottom + height));
	}

	Uint Image::previousPowerOfTwo(Uint value)
	{
		Uint power = 0;
		while (value> (2U << power))
		{	++power;}

		return ((2U << power) == value) ? value : (2U << (power - 1));
	}

	Image::Image()
	{
		delete_on_destructor = true;
		data = 0;

		freeImage();
	}

	Image::Image(Uint const width, Uint const height, Uint const color_channels)
	{
		delete_on_destructor = true;
		data = 0;
		freeImage();
		createImage(width, height, color_channels);

	}
	Image::Image(Uint const width, Uint const height, Uint const color_channels, Uchar* data)
	{
		delete_on_destructor = true;

		this->data = data;
		this->width = width;
		this->height = height;
		this->color_channels = color_channels;
	}

	Image::~Image()
	{
		if (getDeleteDataOnDestructor())
		{
			freeImage();
		}
	}

	Bool Image::isLoaded() const
	{
		return (data != 0);
	}

	void Image::freeImage()
	{
		if (data)
		{
			delete[](data);
		}
		data = 0;

		width = 0;
		height = 0;
		color_channels = 0;
	}

	void Image::zeroImage()
	{
		memset(data, 0,  width * height * color_channels);
	}

	void Image::createImage(Uint const width, Uint const height, Uint const color_channels)
	{
		freeImage();
		data = new Uchar[width * height * color_channels];
		if (data)
		{
			this->width = width;
			this->height = height;
			this->color_channels = color_channels;
		}
	}

	Bool Image::isOpenGlCompliant() const
	{
		return (previousPowerOfTwo(width) == width) && (previousPowerOfTwo(height) == height);
	}

	void Image::makeOpenGlCompliant(Bool vertical_flip, Bool rescale)
	{
		if (rescale)
		{
			Uint new_width = previousPowerOfTwo(width);
			Uint new_height = previousPowerOfTwo(height);

			if ((new_width != width) || (new_height != height))
			{
				downscale(new_width, new_height);
				//crop(0, 0, new_width, new_height);
			}
		}

		if (vertical_flip)
		{
			flip(FlipVertical);
		}
	}

	void Image::applyColorKey(const Color &color)
	{
		if (color_channels == 4)
		{
			for(Uint i = 0; i < width * height; ++i)
			{
				if ( (pixel(i)->r() == color.r()) && 
					 (pixel(i)->g() == color.g()) && 
					 (pixel(i)->b() == color.b()))
				{
					pixel(i)->a() = color.a();
				}
			}
		}
	}

	void Image::downscale(Uint new_width, Uint new_height)
	{
		if ( (new_width > width) || (new_height > height) )
		{
			return;
		}

		Uchar* scaled = new Uchar[new_width * new_height * color_channels];
		if (scaled)
		{
			for (Uint iterator_x = 0; iterator_x != new_width; ++iterator_x)
			{
				Int x = (Int) (iterator_x * (Real(width) / Real(new_width)) + 0.5f);

				for (Uint iterator_y = 0; iterator_y != new_height; ++iterator_y)
				{
					Int y = (Int) (iterator_y * (Real(height) / Real(new_height)) + 0.5f);

					memcpy(&scaled[(iterator_y * new_width + iterator_x) * color_channels],
						   &data[(y * width + x) * color_channels],
						   color_channels);
				}

			}

			delete[](data);
			data = scaled;

			width = new_width;
			height = new_height;
		}
	}

	void Image::crop(Uint const x, Uint const y, Uint const crop_width, Uint const crop_height)
	{
		Uchar* croped = new Uchar[crop_height * crop_width * color_channels];

		for (Uint iterator_x = 0; iterator_x != crop_width; ++iterator_x)
		{
			for (Uint iterator_y = 0; iterator_y != crop_height; ++iterator_y)
			{
				memcpy( &croped[ (iterator_y * crop_width + iterator_x) * color_channels ] ,
						&data[ (iterator_y * width + iterator_x) * color_channels],
						color_channels);
			}
		}

		delete[] data;
		data = croped;

		width = crop_width;
		height = crop_height;
	}

	void Image::flip(FlipMode mode)
	{
		if (isLoaded())
		{
			if (mode == FlipVertical)
			{
				Uint half_height = Uint( floor( Real(height) / 2.0f ) );

				Uint line_size = width * color_channels;
				Uchar *line = new Uchar[width * color_channels];
				Uchar *origin = 0;
				Uchar *destination = 0;

				for (Uint iterator_y = 0; iterator_y != half_height; ++iterator_y)
				{
					destination = line;
					origin = &data[iterator_y * width * color_channels];
					memcpy(destination, origin, line_size);

					destination = &data[iterator_y * width * color_channels];
					origin = &data[(height - iterator_y - 1) * width * color_channels];
					memcpy(destination, origin, line_size);

					destination = &data[(height - iterator_y - 1) * width * color_channels];
					origin = line;
					memcpy(destination, origin, line_size);

				}

				delete[](line);
			}
			else
			{
				Uchar *pixel_color = new Uchar[color_channels];
				Uint half_width = Uint( floor( Real(width) / 2.0f ) );

				//			Uint line_size = width * color_channels;
				Uchar *origin = 0;
				Uchar *destination = 0;

				for (Uint iterator_y = 0; iterator_y != height; ++iterator_y)
				{
					for (Uint iterator_x = 0; iterator_x != half_width; ++iterator_x)
					{
						destination = pixel_color;
						origin = &data[(iterator_y * width + iterator_x) * color_channels];
						memcpy(destination, origin, color_channels);

						destination = &data[(iterator_y * width + iterator_x) * color_channels];
						origin = &data[(iterator_y * width + (width - iterator_x - 1)) * color_channels];
						memcpy(destination, origin, color_channels);

						destination = &data[(iterator_y * width + (width - iterator_x - 1)) * color_channels];
						origin = pixel_color;
						memcpy(destination, origin, color_channels);
					}
				}

				delete[](pixel_color);
			}
		}
	}
} //namespace rengine
