// __!!rengine_copyright!!__ //

#include <rengine/image/Colorspace.h>
#include <rengine/image/stb_image.h>
#include <rengine/math/Math.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace rengine
{
	Uchar* convertJPEG_RGB8(Uchar* in, Uint const& in_size, Uint& out_size)
	{
		Int x = 0;
		Int y = 0;
		Int comp = 0;

		Uchar* data = stbi_jpeg_load_from_memory(in, Int(in_size), &x, &y, &comp, 0);
		Uchar* image_data = 0; //c++ memory

		// TODO
		// for performace sake this should be changed
		//
		if (data)
		{
			size_t outupuSize = x * y * comp;
			image_data = new Uchar[outupuSize];
			if(image_data)
			{
				memcpy(image_data, data, outupuSize);
			}
			rg_free(data);
		}

		out_size = x * y * comp;
		return image_data;
	}

	//In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
	//Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels.
	//As you can see, the Cr and Cb components have half the horizontal resolution of the Y component.
	//V4L2_PIX_FMT_YUYV  is known in the Windows environment as YUY2.
	void convertYUYV_RGB8_Sane(Uchar* in, Uchar* out, Uint const& width, Uint const& height)
	{
		Uint const size = width * height * 2;
		for (Uint i = 0; i < size; i += 4)
		{
			*out++ = clampTo(Real(in[i    ]) + 1.402f   * (Real(in[i + 3]) - 128.0f)                                         , 0.0f, 255.0f); //r0
			*out++ = clampTo(Real(in[i    ]) - 0.34414f * (Real(in[i + 1]) - 128.0f) - 0.71414f * (Real(in[i + 3]) - 128.0f) , 0.0f, 255.0f); //g0
			*out++ = clampTo(Real(in[i    ]) + 1.772f   * (Real(in[i + 1]) - 128.0f)                                         , 0.0f, 255.0f); //b0
			*out++ = clampTo(Real(in[i + 2]) + 1.402f   * (Real(in[i + 3]) - 128.0f)                                         , 0.0f, 255.0f); //r1
			*out++ = clampTo(Real(in[i + 2]) - 0.34414f * (Real(in[i + 1]) - 128.0f) - 0.71414f * (Real(in[i + 3]) - 128.0f) , 0.0f, 255.0f); //g1
			*out++ = clampTo(Real(in[i + 2]) + 1.772f   * (Real(in[i + 1]) - 128.0f)                                         , 0.0f, 255.0f); //b1
		}
	}

	#define SAT(c) \
        if (c & (~255)) { if (c < 0) c = 0; else c = 255; }

	void convertYUYV_RGB8(Uchar* in, Uchar* out, Uint const& width, Uint const& height)
	{
		Uchar *s;
		Uchar *d;
		int l, c;
		int r, g, b, cr, cg, cb, y1, y2;

		l = height;
		s = in;
		d = out;
		while (l--) 
		{
			c = width >> 1;
			while (c--) 
			{
				y1 = *s++;
				cb = ((*s - 128) * 454) >> 8;
				cg = (*s++ - 128) * 88;
				y2 = *s++;
				cr = ((*s - 128) * 359) >> 8;
				cg = (cg + (*s++ - 128) * 183) >> 8;

				r = y1 + cr;
				b = y1 + cb;
				g = y1 - cg;
				SAT(r);
				SAT(g);
				SAT(b);

				*d++ = r;
				*d++ = g;
				*d++ = b;

				r = y2 + cr;
				b = y2 + cb;
				g = y2 - cg;
				SAT(r);
				SAT(g);
				SAT(b);

				*d++ = r;
				*d++ = g;
				*d++ = b;
			}
		}

	}

	void convertBGR8_RGB8(Uchar* in, Uchar* out, Uint const& width, Uint const& height)
	{
		Uint const size = width * height * 3;
		for (Uint i = 0; i != size; i += 3)
		{
			out[i + 0] = in[i + 2];
			out[i + 1] = in[i + 1];
			out[i + 2] = in[i + 0];
		}
	}


	void convertBGR8_RGB8(Uchar* in_out, Uint const& width, Uint const& height)
	{
		Uchar swapper = 0;

		Uint const size = width * height * 3;
		for (Uint i = 0; i != size; i += 3)
		{
			swapper = in_out[i + 0];
			in_out[i + 0] = in_out[i + 2];
			in_out[i + 2] = swapper;
		}
	}
}
