// __!!rengine_copyright!!__ //

#ifndef __RENGINE_COLORSPACE_H__
#define __RENGINE_COLORSPACE_H__

#include <rengine/lang/Lang.h>

namespace rengine
{
	Uchar* convertJPEG_RGB8(Uchar* in, Uint const& in_size, Uint& out_size);

	//In this format each four bytes is two pixels. Each four bytes is two Y's, a Cb and a Cr.
	//Each Y goes to one of the pixels, and the Cb and Cr belong to both pixels.
	//As you can see, the Cr and Cb components have half the horizontal resolution of the Y component.
	//YUYV  is known in the Windows environment as YUY2.

	// Sane but slow
	void convertYUYV_RGB8_Sane(Uchar* in, Uchar* out, Uint const& width, Uint const& height);
	void convertYUYV_RGB8(Uchar* in, Uchar* out, Uint const& width, Uint const& height);

	void convertBGR8_RGB8(Uchar* in, Uchar* out, Uint const& width, Uint const& height);
	void convertBGR8_RGB8(Uchar* in_out, Uint const& width, Uint const& height);

} //namespace rengine

#endif // __RENGINE_COLORSPACE_H__
