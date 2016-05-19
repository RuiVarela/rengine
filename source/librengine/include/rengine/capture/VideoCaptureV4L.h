// __!!rengine_copyright!!__ //

#ifndef __RENGINE_CAPTURE_V4L_H__
#define __RENGINE_CAPTURE_V4L_H__

#include <rengine/capture/VideoCapture.h>

namespace rengine
{
	class VideoCaptureV4L : public VideoCapture
	{
	public:
		VideoCaptureV4L();
		VideoCaptureV4L(CaptureOptions const& capture_options);
		virtual ~VideoCaptureV4L();

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
		void initializeDevice();
		void setRate();

		PrivateImplementation* implementation_;
		Bool streaming_;
	};

} // namespace rengine

#endif //__RENGINE_CAPTURE_V4L_H__
