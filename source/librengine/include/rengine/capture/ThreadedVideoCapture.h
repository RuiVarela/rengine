// __!!rengine_copyright!!__ //

#ifndef __RENGINE_THREADEDVIDEOCAPTURE_H__
#define __RENGINE_THREADEDVIDEOCAPTURE_H__

#include <rengine/lang/Lang.h>
#include <rengine/capture/VideoCapture.h>
#include <rengine/thread/Thread.h>
#include <rengine/util/SynchronizedObjects.h>

namespace rengine
{

	class ThreadedVideoCapture : public VideoCapture, public Thread
	{
	public:

		// if frame queue drops frames, they must be deleted
		struct FrameAutoReleaser
		{
			FrameAutoReleaser();
			~FrameAutoReleaser();

			SharedPointer<Frame> frame;
		private:
			FrameAutoReleaser(FrameAutoReleaser const& non_copy) {};
			FrameAutoReleaser const& operator=(FrameAutoReleaser const& non_copy) { return *this; }
		};

		typedef SharedPointer<VideoCapture> SharedVideoCapture;
		typedef SharedPointer<FrameAutoReleaser> SharedFrame;
		typedef SynchronizedQueue<SharedFrame> FrameQueue;



		ThreadedVideoCapture(SharedVideoCapture video_capture);
		virtual ~ThreadedVideoCapture();


		//
		// Video Capture proxy methods
		//
		virtual Devices enumerateDevices() const;
		virtual Formats enumerateFormats(Device const& device) const;
		virtual void open(CaptureOptions const& capture_options);
		virtual void close();
		virtual Bool ready() const;

		// do not forget to call frame->release()
		virtual SharedPointer<Frame> grab(FrameOptions const& options, SharedPointer<Frame> const& frame = 0);



		//
		// Thread methods
		//	open(..) if succeeds calls thread start

		virtual void preRun();
		virtual void run();

		FrameQueue const& frameQueue() const;
		FrameQueue& frameQueue();

	private:
		SharedVideoCapture m_video_capture;
		Mutex m_mutex;
		FrameQueue m_queue;
	};

	//
	// implementation
	//

	RENGINE_INLINE ThreadedVideoCapture::FrameAutoReleaser::FrameAutoReleaser()
	{
		frame = 0;
	}

	RENGINE_INLINE ThreadedVideoCapture::FrameAutoReleaser::~FrameAutoReleaser()
	{
		if (frame)
		{
			frame->release();
			frame = 0;
		}
	}

	RENGINE_INLINE ThreadedVideoCapture::FrameQueue const& ThreadedVideoCapture::frameQueue() const
	{
		return m_queue;
	}

	RENGINE_INLINE ThreadedVideoCapture::FrameQueue & ThreadedVideoCapture::frameQueue()
	{
		return m_queue;
	}

} // namespace rengine

#endif // __RENGINE_THREADEDVIDEOCAPTURE_H__
