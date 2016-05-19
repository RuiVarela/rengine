// __!!rengine_copyright!!__ //

#ifndef __RENGINE_IMAGE_GFX____
#define __RENGINE_IMAGE_GFX____

#include <string>
#include <rengine/math/Vector.h>

namespace rengine
{
	class Image
	{
	public:
		typedef Vector4<Uchar> Color;

		enum FlipMode
		{
			FlipVertical,
			FlipHorizontal
		};

		Image();
		Image(Uint const width, Uint const height, Uint const color_channels);
		Image(Uint const width, Uint const height, Uint const color_channels, Uchar* data);

		~Image();

		void createImage(Uint const width, Uint const height, Uint const color_channels);

		void freeImage();
		void zeroImage();

		Bool isLoaded() const;

		void applyColorKey(Color const&color);
		void crop(Uint const x, Uint const y, Uint const crop_width, Uint const crop_height);
		void downscale(Uint new_width, Uint new_height);
		void flip(FlipMode mode);


		static Rectanglei letterbox(Rectanglei const& src, Rectanglei const& dst);
		
		static Uint previousPowerOfTwo(Uint value);
		Bool isOpenGlCompliant() const;
		void makeOpenGlCompliant(Bool vertical_flip = true, Bool rescale = true);

		Uchar* rawPixel(Uint x, unsigned y)
		{
			return &data[(y * width + x) * color_channels];
		}

		Color* pixel(Uint x, unsigned y)
		{
			return (Color*) &data[(y * width + x) * color_channels];
		}

		Color* pixel(Uint i)
		{
			return (Color*) &data[i * color_channels];
		}
		
		Uchar* getData() const
		{
			return data;
		}

		Uint getWidth() const
		{
			return width;
		}

		Uint getHeight() const
		{
			return height;
		}

		Uint getColorChannels() const
		{
			return color_channels;
		}

		//
		// By default destructor will free image data.
		//
		void setDeleteDataOnDestructor(Bool delete_data)
		{
			delete_on_destructor = delete_data;
		}
		Bool getDeleteDataOnDestructor() const
		{
			return delete_on_destructor;
		}
	private:
		Uchar *data;
		Uint width;
		Uint height;
		Uint color_channels;

		Bool delete_on_destructor;
	};

} //namespace rengine

#endif // __RENGINE_IMAGE_GFX__
