// __!!rengine_copyright!!__ //

#ifndef __RENGINE_CAPTURE_H__
#define __RENGINE_CAPTURE_H__

#include <rengine/lang/Lang.h>
#include <rengine/lang/exception/Exception.h>
#include <string>
#include <vector>

namespace rengine
{
	class VideoCapture
	{
	public:

		struct Device
		{
			Device();

			Uint index;
			std::string location;
			std::string name;
			std::string driver;
			std::string bus;
		};

		struct Format
		{
			Format();
			Bool match(Format const& filter) const;

			Uint width;
			Uint height;
			std::string mode; //fourcc
			Uint interval_numerator;
			Uint interval_denominator;
			Uint sample_size; //reference only, client should not need to set this
		};

		typedef std::vector<Device> Devices;
		typedef std::vector<Format> Formats;

		struct CaptureOptions : public Format, public Device
		{
			CaptureOptions();
			CaptureOptions(std::string const& location);
			CaptureOptions(Uint const& index);
			CaptureOptions(Uint const& width, Uint const& height);

			void set(Format const& format);
			void set(Device const& device);
		};

		struct FrameOptions
		{
			FrameOptions();
			FrameOptions(Uint64 const frame);
			FrameOptions(Real const time);

			Uint64 frame_number;
			Real time_stamp;
		};

		struct Frame
		{
			Frame();
			~Frame(); // does not call release!!

			void release();

			Uint8* data;
			Uint size;
			Real time_stamp;
		};

		VideoCapture();
		virtual ~VideoCapture();

		//
		// Creates a new VideoCapture
		//
		static SharedPointer<VideoCapture> create();

		CaptureOptions const& captureOptions() const;

		virtual Devices enumerateDevices() const;
		virtual Formats enumerateFormats(Device const& device) const;
		static Formats filterByFormat(Formats const& formats, Format const& format);
		static Format matchBestFormat(Formats const& formats, Format const& format);

		virtual void open(CaptureOptions const& capture_options) = 0;
		virtual void close() = 0;
		virtual Bool ready() const = 0;

		/***
		 * Grabs a new frame,
		 *
		 * Tries to reuse the input frame data, if needed the frame will be released or resized
		 * If the input frame is 0, a new frame is allocated and returned
		 * The caller is responsible for calling frame->release() for the output frame
		 *
		 * Frames raw RBG8 frames
		 * Returns the grabbed frame
		 */
		virtual SharedPointer<Frame> grab(FrameOptions const& options, SharedPointer<Frame> const& frame = 0) = 0;
	private:
		VideoCapture(VideoCapture const& copy);
		void operator=(VideoCapture const& copy);

	protected:
		CaptureOptions options_;
	};

	//
	// Exceptions
	//
	class VideoCaptureException : public Exception
	{
	public:
		MetaException(VideoCaptureException);
		virtual ~VideoCaptureException() {}
	};


	//
	// Implementation
	//
	RENGINE_INLINE VideoCapture::Devices VideoCapture::enumerateDevices() const
	{
		return Devices();
	}

	RENGINE_INLINE VideoCapture::Formats VideoCapture::enumerateFormats(Device const& device) const
	{
		return Formats();
	}

	RENGINE_INLINE VideoCapture::CaptureOptions const& VideoCapture::captureOptions() const
	{
		return options_;
	}

	RENGINE_INLINE VideoCapture::VideoCapture(VideoCapture const& copy)
	{
	}

	RENGINE_INLINE void VideoCapture::operator=(VideoCapture const& copy)
	{
	}
}

#endif //__RENGINE_CAPTURE_H__
