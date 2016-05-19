// __!!rengine_copyright!!__ //

#include <rengine/capture/VideoCapture.h>
#include <rengine/lang/debug/Debug.h>

#include <limits>
#include <cstdlib>

namespace rengine
{
	// Supported Modes (order means priority)
	static Uint const supported_modes = 2;
	static std::string const modes[] = {
									"RGB3", // RGB-8-8-8
									"BGR3", // BGR-8-8-8
									"YUYV", // YUV 4:2:2
									"YUY2", // YUV 4:2:2
									"MJPG", // MJPEG stream
									"JPEG"  // JPEG stream
									};

	VideoCapture::Device::Device()
	{
		index = 0;
	}

	VideoCapture::Format::Format()
	{
		this->width = 0;
		this->height = 0;
		this->mode = "";
		this->interval_numerator = 0;
		this->interval_denominator = 0;
		this->sample_size = 0;
	}

	Bool VideoCapture::Format::match(Format const& filter) const
	{
		Bool matched = true;

		if (filter.width)
		{
			matched &= (filter.width == width);
		}

		if (filter.height)
		{
			matched &= (filter.height == height);
		}

		if (!filter.mode.empty())
		{
			matched &= (filter.mode == mode);
		}

		if (filter.interval_numerator)
		{
			matched &= (filter.interval_numerator == interval_numerator);
		}

		if (filter.interval_denominator)
		{
			matched &= (filter.interval_denominator == interval_denominator);
		}

		return matched;
	}

	VideoCapture::CaptureOptions::CaptureOptions()
	{
	}

	VideoCapture::CaptureOptions::CaptureOptions(std::string const& location)
	{
		this->location = location;
		this->index = 0;
	}

	VideoCapture::CaptureOptions::CaptureOptions(Uint const& index)
	{
		this->index = index;
	}

	VideoCapture::CaptureOptions::CaptureOptions(Uint const& width, Uint const& height)
	{
		this->width = width;
		this->height = height;
	}

	void VideoCapture::CaptureOptions::set(Format const& format)
	{
		this->width = format.width;
		this->height = format.height;
		this->mode = format.mode;
		this->interval_numerator = format.interval_numerator;
		this->interval_denominator = format.interval_denominator;
		this->sample_size = format.sample_size;
	}

	void VideoCapture::CaptureOptions::set(Device const& device)
	{
		this->index = device.index;
		this->location = device.location;
		this->name = device.name;
		this->driver = device.driver;
		this->bus = device.bus;
	}

	VideoCapture::FrameOptions::FrameOptions()
		:frame_number(0), time_stamp(0.0f)
	{
	}

	VideoCapture::FrameOptions::FrameOptions(Uint64 const frame)
		:frame_number(frame), time_stamp(0.0f)
	{
	}

	VideoCapture::FrameOptions::FrameOptions(Real const time)
		:frame_number(0), time_stamp(time)
	{
	}

	VideoCapture::Frame::Frame()
		:data(0), size(0), time_stamp(0)
	{
	}

	void VideoCapture::Frame::release()
	{
		if (data)
		{
			delete[](data);
		}

		data = 0;
		size = 0;
		time_stamp = 0;
	}

	VideoCapture::Frame::~Frame()
	{
	}


	//
	// Video Capture
	//
	VideoCapture::VideoCapture()
	{
	}

	VideoCapture::~VideoCapture()
	{
	}

	VideoCapture::Formats VideoCapture::filterByFormat(Formats const& formats, Format const& format)
	{
		Formats filtered;

		for (Formats::const_iterator current = formats.begin(); current != formats.end(); ++current)
		{
			if (current->match(format))
			{
				filtered.push_back(*current);
			}
		}

		return filtered;
	}

	VideoCapture::Format VideoCapture::matchBestFormat(Formats const& formats, Format const& format)
	{
		Format filter = format;
		Formats filtered = filterByFormat(formats, filter);

		if (filtered.empty())
		{
			return Format();
		}

		if (filtered.size() == 1)
		{
			return filtered.back();
		}

		//
		// Match width
		//
		if (filter.width == 0)
		{
			for (Formats::const_iterator current = filtered.begin(); current != filtered.end(); ++current)
			{
				if (current->width > filter.width)
				{
					filter.width = current->width;
				}
			}

			filtered = filterByFormat(filtered, filter);
			RENGINE_ASSERT(!filtered.empty());

			if (filtered.size() == 1)
			{
				return filtered.back();
			}
		}

		//
		// Match height
		//
		if (filter.width == 0)
		{
			for (Formats::const_iterator current = filtered.begin(); current != filtered.end(); ++current)
			{
				if (current->height > filter.height)
				{
					filter.height = current->height;
				}
			}

			filtered = filterByFormat(filtered, filter);
			RENGINE_ASSERT(!filtered.empty());

			if (filtered.size() == 1)
			{
				return filtered.back();
			}
		}

		//
		// Match interval
		//
		if ((filter.interval_denominator == 0) || (filter.interval_numerator == 0))
		{
			Real64 interval = std::numeric_limits<Real64>::max();

			for (Formats::const_iterator current = filtered.begin(); current != filtered.end(); ++current)
			{
				if (current->interval_denominator > 0)
				{
					Real64 current_interval = Real64(current->interval_numerator) / Real64(current->interval_denominator);

					if (current_interval < interval)
					{
						interval = current_interval;
						filter.interval_denominator = current->interval_denominator;
						filter.interval_numerator = current->interval_numerator;
					}
				}
			}

			filtered = filterByFormat(filtered, filter);
			RENGINE_ASSERT(!filtered.empty());

			if (filtered.size() == 1)
			{
				return filtered.back();
			}
		}


		if (filter.mode.empty())
		{
			Int found_mode = -1;

			for (Uint current_mode = 0; ( (current_mode != supported_modes) && (found_mode < 0) ); ++current_mode)
			{
				for (Formats::const_iterator current = filtered.begin(); ( (current != filtered.end()) && (found_mode < 0) ); ++current)
				{
					if (current->mode == modes[current_mode])
					{
						found_mode = Int(current_mode);
					}
				}
			}

			if (found_mode >= 0)
			{
				filter.mode = modes[found_mode];

				filtered = filterByFormat(filtered, filter);
				RENGINE_ASSERT(!filtered.empty());

				if (filtered.size() == 1)
				{
					return filtered.back();
				}
			}
		}

		RENGINE_ASSERT(!filtered.empty());
		return filtered.back();
	}


} // namespace rengine
