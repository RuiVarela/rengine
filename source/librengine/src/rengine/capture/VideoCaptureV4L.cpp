// __!!rengine_copyright!!__ //

#include <rengine/capture/VideoCaptureV4L.h>
#include <rengine/image/Colorspace.h>
#include <rengine/file/File.h>
#include <rengine/string/String.h>

#include <linux/videodev2.h>

#include <string>

#include <cassert>
#include <cstring>
#include <cerrno>
#include <cstdlib>

#include <unistd.h>
#include <fcntl.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>


#include <iostream>

#define RENGINE_V4L_BUFFERS 4
#define RENGINE_V4L_IOCTL_RETRY 4


namespace rengine
{

	int myIoctl(int fd, unsigned long int request, void *arg)
	{
		int result = 0;
		int tries = RENGINE_V4L_IOCTL_RETRY;

		result = ioctl(fd, request, arg);

		while( (result == -1) &&
			   (errno == EINTR || errno == EAGAIN || errno == ETIMEDOUT) &&
			   (tries > 0)
			 )
		{
			--tries;
			result = ioctl(fd, request, arg);
		}

		return result;
	}

	struct Buffer
	{
		void *start;
	    size_t length;
	};


	struct VideoCaptureV4L::PrivateImplementation
	{
		PrivateImplementation()
			:fd(-1), buffers(0)
		{
			memset(&format, 0, sizeof(format));
			memset(&request_buffers, 0, sizeof(request_buffers));
			memset(&buffer, 0, sizeof(buffer));

		}

		int fd;
		struct v4l2_format format;
		struct v4l2_requestbuffers request_buffers;
		struct v4l2_buffer buffer;
		Buffer* buffers;
	};

	VideoCaptureV4L::VideoCaptureV4L()
		:implementation_(new PrivateImplementation()), streaming_(false)
	{

	}

	VideoCaptureV4L::VideoCaptureV4L(CaptureOptions const& capture_options) :
		implementation_(new PrivateImplementation()), streaming_(false)
	{
		try
		{
			open(capture_options);
		}
		catch (...)
		{
			close();
			delete (implementation_);
		}
	}

	VideoCaptureV4L::~VideoCaptureV4L()
	{
		close();
		delete(implementation_);
	}

	VideoCapture::Devices VideoCaptureV4L::enumerateDevices() const
	{
		Devices devices;

		std::string const device_directory = "/sys/class/video4linux";
		DirectoryContents contents = getDirectoryContents(device_directory);
		for (DirectoryContents::const_iterator file = contents.begin(); file != contents.end(); ++file)
		{
			if (!startsWith(*file, "video"))
			{
				continue;
			}

			Device device;
			device.location = "/dev/" + *file;

			struct v4l2_capability v4l2_capability;
			int fd = ::open(device.location.c_str(), O_RDONLY);

			if (fd != -1)
			{
				if (ioctl(fd, VIDIOC_QUERYCAP, &v4l2_capability) >= 0)
				{
					std::string index_as_string = device.location;
					replace(index_as_string, "/dev/video", "");
					device.index = lexical_cast<Int>(index_as_string, 0);

					device.name = std::string((Char*) v4l2_capability.card);
					device.driver = std::string((Char*) v4l2_capability.driver);
					device.bus = std::string((Char*) v4l2_capability.bus_info);

					devices.push_back(device);
				}
				::close(fd);
			}
		}

		return devices;
	}

	VideoCapture::Formats VideoCaptureV4L::enumerateFormats(Device const& device) const
	{
		Formats formats;

		int fd = ::open(device.location.c_str(), O_RDONLY);
		if (fd != -1)
		{
			//
			// Pixel Format
			//
			struct v4l2_fmtdesc v4l_format;
			memset(&v4l_format, 0, sizeof(v4l_format));
			v4l_format.index = 0;
			v4l_format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

			while (myIoctl(fd, VIDIOC_ENUM_FMT, &v4l_format) == 0)
			{
				v4l_format.index++;

				Format format;
				format.mode = "cccc";
				format.mode[0] = Char(v4l_format.pixelformat & 0xFF);
				format.mode[1] = Char((v4l_format.pixelformat >> 8) & 0xFF);
				format.mode[2] = Char((v4l_format.pixelformat >> 16) & 0xFF);
				format.mode[3] = Char((v4l_format.pixelformat >> 24) & 0xFF);

				//
				// Frame sizes
				//
				struct v4l2_frmsizeenum frame_size;
				memset(&frame_size, 0, sizeof(frame_size));
				frame_size.index = 0;
				frame_size.pixel_format = v4l_format.pixelformat;

				while (myIoctl(fd, VIDIOC_ENUM_FRAMESIZES, &frame_size) == 0)
				{
					frame_size.index++;
					if (frame_size.type != V4L2_FRMSIZE_TYPE_DISCRETE)
					{
						continue;
					}

					format.width = frame_size.discrete.width;
					format.height = frame_size.discrete.height;

					//
					// Frame Interval
					//
					struct v4l2_frmivalenum frame_interval;
					memset(&frame_interval, 0, sizeof(frame_interval));
					frame_interval.index = 0;
					frame_interval.pixel_format = v4l_format.pixelformat;
					frame_interval.width = frame_size.discrete.width;
					frame_interval.height = frame_size.discrete.height;

					while (myIoctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frame_interval) == 0)
					{
						frame_interval.index++;
						if (frame_interval.type == V4L2_FRMIVAL_TYPE_DISCRETE)
						{
							format.interval_numerator = frame_interval.discrete.numerator;
							format.interval_denominator = frame_interval.discrete.denominator;
							formats.push_back(format);
						}
					}
				}
			}

			::close(fd);
		}

		return formats;
	}

	void VideoCaptureV4L::setRate()
	{
		//
		// Set rate
		//
		struct v4l2_streamparm streamparm;
		memset(&streamparm, 0, sizeof(streamparm));
		streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		streamparm.parm.capture.timeperframe.numerator = options_.interval_numerator;
		streamparm.parm.capture.timeperframe.denominator = options_.interval_denominator;

		if (myIoctl(implementation_->fd, VIDIOC_S_PARM, &streamparm) < 0)
		{
			throw VideoCaptureException(  9, "Unable to set framerate: " + options_.location);
		}

	}

	void VideoCaptureV4L::initializeDevice()
	{
		struct v4l2_capability capability;


        if (-1 == myIoctl(implementation_->fd, VIDIOC_QUERYCAP, &capability))
        {
        	throw VideoCaptureException(004, "Invalid device: " + options_.location + " [" + strerror(errno) + "]");
        }

    	if (!(capability.capabilities & V4L2_CAP_VIDEO_CAPTURE))
    	{
    		throw VideoCaptureException(005, "Video capture not supported: " + options_.location);
    	}

    	if (!(capability.capabilities & V4L2_CAP_STREAMING))
    	{
    		throw VideoCaptureException(006, "Streaming not supported: " + options_.location);
    	}

//		if (!(capability.capabilities & V4L2_CAP_READWRITE))
//		{
//			throw VideoCaptureException(007, "Read write i/o not supported: " + options_.location);
//		}

    	//
    	// Choose format
    	//
		Format format = VideoCapture::matchBestFormat( enumerateFormats(options_) , options_);
		options_.set(format);

		implementation_->format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		implementation_->format.fmt.pix.width = options_.width;
		implementation_->format.fmt.pix.height = options_.height;
		implementation_->format.fmt.pix.pixelformat = v4l2_fourcc(options_.mode[0], options_.mode[1], options_.mode[2], options_.mode[3]);
		implementation_->format.fmt.pix.field = V4L2_FIELD_ANY;

		//
		// Set format
		//
		if (myIoctl(implementation_->fd, VIDIOC_S_FMT, &implementation_->format) < 0)
		{
			throw VideoCaptureException(  8, "Unable to set format: " + options_.location);
		}

		//
		// Set rate
		//
		setRate();

		//
		// Request Buffers
		//
		implementation_->request_buffers.count = RENGINE_V4L_BUFFERS;
		implementation_->request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		implementation_->request_buffers.memory = V4L2_MEMORY_MMAP;

		if (myIoctl(implementation_->fd, VIDIOC_REQBUFS, &implementation_->request_buffers) < 0)
		{
			throw VideoCaptureException( 10, "Unable to request buffers: " + options_.location);
		}

		implementation_->buffers = new Buffer[implementation_->request_buffers.count];

		//
		// Map buffers
		//
		for (unsigned int i = 0; i != implementation_->request_buffers.count; ++i)
		{
	        memset (&implementation_->buffer, 0, sizeof (v4l2_buffer));
	        implementation_->buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	        implementation_->buffer.memory = V4L2_MEMORY_MMAP;
	        implementation_->buffer.index = i;

	        if (myIoctl(implementation_->fd, VIDIOC_QUERYBUF, &implementation_->buffer) < 0)
	        {
	        	throw VideoCaptureException(  10, "Unable to map buffer [" + lexical_cast<std::string>(i) +  "]: " + options_.location);
	        }

	        implementation_->buffers[i].length = implementation_->buffer.length; /* remember for munmap() */
	        implementation_->buffers[i].start = mmap(0, implementation_->buffer.length,
													 PROT_READ | PROT_WRITE, /* recommended */
													 MAP_SHARED,             /* recommended */
													 implementation_->fd, implementation_->buffer.m.offset);
	        if (MAP_FAILED == implementation_->buffers[i].start)
	        {
					throw VideoCaptureException(  11, "Unable to map buffer [" + lexical_cast<std::string>(i) +  "]: " + options_.location);
	        }
		}

		// Queue buffers
		for (unsigned int i = 0; i != implementation_->request_buffers.count; ++i)
		{
	        implementation_->buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	        implementation_->buffer.memory = V4L2_MEMORY_MMAP;
	        implementation_->buffer.index = i;

	        if (myIoctl(implementation_->fd, VIDIOC_QBUF, &implementation_->buffer) < 0)
	        {
	        	throw VideoCaptureException(  12, "Unable tqueue buffer");
	        }
		}
	}

	void VideoCaptureV4L::open(CaptureOptions const& capture_options)
	{
		close();

		options_ = capture_options;

		Bool matched = false;
		Devices devices = enumerateDevices();
		for (Devices::const_iterator i = devices.begin(); ((i != devices.end()) && !matched); ++i)
		{
			matched = ((i->location == options_.location) || (i->index == options_.index) );
			if (matched)
			{
				options_.set(*i);
			}
		}

		struct stat stats;

		//
		// Open Device
		//
        if (stat(options_.location.c_str(), &stats) == -1)
        {
        	throw VideoCaptureException(001, "Invalid device: " + options_.location + " [" + strerror(errno) + "]");
        }

        if (!S_ISCHR(stats.st_mode))
        {
        	throw VideoCaptureException(002, "Is no device: " + options_.location);
        }

        implementation_->fd = ::open(options_.location.c_str(), O_RDWR /* required */ | O_NONBLOCK, 0);

        if (implementation_->fd == -1)
        {
        	throw VideoCaptureException(003, "Unable to open device: " + options_.location);
        }

        initializeDevice();
        enableStreaming();
	}

	void VideoCaptureV4L::close()
	{

		//
		// Disable Streaming
		//
		disableStreaming();

		//
		// unmap buffers
		//

		if (implementation_->buffers)
		{
			for (unsigned int i = 0; i != implementation_->request_buffers.count; ++i)
			{
			    if (MAP_FAILED != implementation_->buffers[i].start)
			    {
			    	 munmap(implementation_->buffers[i].start, implementation_->buffers[i].length);
			    }
			}

			delete[](implementation_->buffers);
			implementation_->buffers = 0;


			memset(&implementation_->request_buffers, 0, sizeof(struct v4l2_requestbuffers));
			implementation_->request_buffers.count = 0;
			implementation_->request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			implementation_->request_buffers.memory = V4L2_MEMORY_MMAP;
			if (implementation_->fd != -1)
			{
				myIoctl(implementation_->fd, VIDIOC_REQBUFS, &implementation_->request_buffers);
			}

		}




		//
		// Close Device
		//
		if (implementation_->fd != -1)
		{
			//
			// Stop Streaming
			//
			int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			myIoctl(implementation_->fd, VIDIOC_STREAMOFF, &type);


			::close(implementation_->fd);
			implementation_->fd = -1;
		}

	}

	void VideoCaptureV4L::enableStreaming()
	{
		if (!streaming_ && (implementation_->fd != -1))
		{
			streaming_ = true;

			int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			if (myIoctl(implementation_->fd, VIDIOC_STREAMON, &type) < 0)
			{
				streaming_ = false;
			}
		}
	}

	void VideoCaptureV4L::disableStreaming()
	{
		if (streaming_ && (implementation_->fd != -1))
		{
			int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			myIoctl(implementation_->fd, VIDIOC_STREAMOFF, &type);
		}

		streaming_ = false;
	}

	Bool VideoCaptureV4L::ready() const
	{
		return streaming_;
	}

	SharedPointer<VideoCaptureV4L::Frame> VideoCaptureV4L::grab(VideoCaptureV4L::FrameOptions const& options, SharedPointer<VideoCaptureV4L::Frame> const& frame)
	{
		if (!ready())
		{
			if (frame.get())
			{
				frame->release();
			}

			return 0;
		}

		fd_set rdset;
		struct timeval timeout;


		FD_ZERO(&rdset);
		FD_SET(implementation_->fd, &rdset);
		//timeout.tv_sec = 1; // 1 sec timeout
		//timeout.tv_usec = 0;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100;

		SharedPointer<Frame> output_frame = new Frame();
		Uint const frame_size = options_.width * options_.height * 3;

		if (frame.get() && (frame->size == frame_size))
		{
			output_frame = frame;
		}
		else if (frame.get() && frame->data && frame->size)
		{
			frame->release();
		}

		int ret = select(implementation_->fd + 1, &rdset, NULL, NULL, &timeout);
		if (ret < 0)
		{
			output_frame->release();
			throw VideoCaptureException(  13, "Unable to grab frame (select error).");
		}
		else if (ret == 0)
		{
			output_frame->release();
			return 0;
			//throw VideoCaptureException(  14, "Unable to grab frame (select timeout).");
		}
		else if ((ret > 0) && (FD_ISSET(implementation_->fd, &rdset)))
		{
	        memset(&implementation_->buffer, 0, sizeof (v4l2_buffer));
	        implementation_->buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	        implementation_->buffer.memory = V4L2_MEMORY_MMAP;

			ret = myIoctl(implementation_->fd, VIDIOC_DQBUF, &implementation_->buffer);
			Uint64 ts = Uint64(implementation_->buffer.timestamp.tv_sec * 1000000000 +  implementation_->buffer.timestamp.tv_usec * 1000); //in nanosec

			if (ret < 0)
			{
				throw VideoCaptureException(  15, "Unable to dequeue buffer.");
			}

			output_frame->time_stamp = Real(ts) / (1000000000.0f);

			//
			// Decode Frame
			//
			if (implementation_->buffer.bytesused)
			{
				if (options_.mode == "RGB3")
				{
					if (!output_frame->data)
					{
						output_frame->data = new Uchar[frame_size];
						output_frame->size = frame_size;
					}


					if (implementation_->buffers[implementation_->buffer.index].start &&
						implementation_->buffer.bytesused == frame_size)
					{
						memcpy(output_frame->data,
							   implementation_->buffers[implementation_->buffer.index].start,
							   implementation_->buffer.bytesused);
					}

					
				}
				if (options_.mode == "BGR3")
				{
					if (!output_frame->data)
					{
						output_frame->data = new Uchar[frame_size];
						output_frame->size = frame_size;
					}

					if (implementation_->buffers[implementation_->buffer.index].start &&
						implementation_->buffer.bytesused == frame_size)
					{
						convertBGR8_RGB8((Uchar*)implementation_->buffers[implementation_->buffer.index].start,
												 output_frame->data,
												 options_.width, options_.height);
					}
				}
				else if ( (options_.mode == "YUYV") || (options_.mode == "YUY2") )
				{
					if (!output_frame->data)
					{
						output_frame->data = new Uchar[frame_size];
						output_frame->size = frame_size;
					}

					convertYUYV_RGB8((Uchar*)implementation_->buffers[implementation_->buffer.index].start,
											 output_frame->data,
											 options_.width, options_.height);

				}
				else if ( (options_.mode == "MJPG") || (options_.mode == "JPEG") )
				{
					output_frame->release();
					output_frame->data = convertJPEG_RGB8((Uchar*)implementation_->buffers[implementation_->buffer.index].start,
																  implementation_->buffer.bytesused, output_frame->size);
					output_frame->size = frame_size;
				}
				else
				{
					output_frame->release();
				}
			}

			ret = myIoctl(implementation_->fd, VIDIOC_QBUF,  &implementation_->buffer);
			if (ret < 0)
			{
				output_frame->release();
				throw VideoCaptureException(  16, "Unable to queue buffer.");
			}
		}


	     //   std::cout << "Got Frame" << std::endl;

		return output_frame;
	}

} // namespace rengine
