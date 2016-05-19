// __!!rengine_copyright!!__ //

#ifndef __RENGINE_VIDEO_CAPTURE_DSHOW_H__
#define __RENGINE_VIDEO_CAPTURE_DSHOW_H__

#include <rengine/lang/Lang.h>
#include <rengine/capture/VideoCapture.h>

//
// direct show capture device my not enforce the selected framrate
// the system aproximates to the next available speed
//
namespace rengine
{
	class VideoCaptureDShow : public VideoCapture
	{
	public:
		VideoCaptureDShow();
		VideoCaptureDShow(CaptureOptions const& capture_options);
		virtual ~VideoCaptureDShow();

		virtual Formats enumerateFormats(Device const& device) const;
		virtual Devices enumerateDevices() const;

		virtual void open(CaptureOptions const& capture_options);
		virtual void close();
		virtual Bool ready() const;

		void enableStreaming();
		void disableStreaming();

		virtual SharedPointer<Frame> grab(FrameOptions const& options, SharedPointer<Frame> const& frame = 0);

	private:
		struct PrivateImplementation;

		PrivateImplementation* implementation_;
		Bool streaming_;
	};

} //namespace rengine

#endif //__RENGINE_VIDEO_CAPTURE_DSHOW_H__
