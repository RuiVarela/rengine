// __!!rengine_copyright!!__ //

#include <rengine/capture/VideoCapture.h>

#ifdef RENGINE_WITH_V4L
#include <rengine/capture/VideoCaptureV4L.h>
#endif //RENGINE_WITH_V4L

#ifdef RENGINE_WITH_DSHOW
#include <rengine/capture/VideoCaptureDShow.h>
#endif //RENGINE_WITH_DSHOW

namespace rengine
{

	SharedPointer<VideoCapture> VideoCapture::create()
	{
		SharedPointer<VideoCapture> capture;

#ifdef RENGINE_WITH_V4L
		capture = new VideoCaptureV4L();
#endif //RENGINE_WITH_V4L

#ifdef RENGINE_WITH_DSHOW
		capture = new VideoCaptureDShow();
#endif //RENGINE_WITH_DSHOW
		return capture;
	}

} // namespace rengine
