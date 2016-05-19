// __!!rengine_copyright!!__ //

#include <rengine/capture/ThreadedVideoCapture.h>

namespace rengine
{
	ThreadedVideoCapture::ThreadedVideoCapture(SharedVideoCapture video_capture)
		:m_video_capture(video_capture)
	{
		m_queue.setMaxSize(30);
	}

	ThreadedVideoCapture::~ThreadedVideoCapture()
	{
		close();
		m_video_capture = 0;
	}

	VideoCapture::Devices ThreadedVideoCapture::enumerateDevices() const
	{
		return m_video_capture->enumerateDevices();
	}

	VideoCapture::Formats ThreadedVideoCapture::enumerateFormats(Device const& device) const
	{
		return m_video_capture->enumerateFormats(device);
	}

	void ThreadedVideoCapture::open(CaptureOptions const& capture_options)
	{
		ScopedLock lock(m_mutex);
		m_video_capture->open(capture_options);
		options_ = m_video_capture->captureOptions();

		if (m_video_capture->ready())
		{
			start();
		}
	}

	void ThreadedVideoCapture::close()
	{
		ScopedLock lock(m_mutex);
		stop();
		m_video_capture->close();
	}

	Bool ThreadedVideoCapture::ready() const
	{
		return m_video_capture->ready();
	}

	SharedPointer<VideoCapture::Frame> ThreadedVideoCapture::grab(FrameOptions const& options, SharedPointer<Frame> const& frame)
	{
		SharedPointer<VideoCapture::Frame> output = 0;

		SharedFrame auto_frame;
		if (m_queue.tryPop(auto_frame))
		{
			output = auto_frame->frame;

			// prevent frame relese on auto_frame object destruction
			auto_frame->frame = 0;
		}

		return output;
	}

	void ThreadedVideoCapture::preRun()
	{

	}

	void ThreadedVideoCapture::run()
	{
		FrameOptions options;
		SharedPointer<Frame> frame = 0;

		while(keepRunning())
		{

			frame = m_video_capture->grab(options, frame);
			if (frame)
			{
				SharedFrame auto_frame = new FrameAutoReleaser();
				auto_frame->frame = frame;

				m_queue.push(auto_frame);
				frame = 0;
			}
			else
			{
				int sleep = int((float(options_.interval_numerator) / float(options_.interval_denominator)) * 0.2 * 1000);
				Thread::microSleep(sleep);
			}


		}
	}

} // namespace rengine
