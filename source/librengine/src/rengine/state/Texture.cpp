// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/state/Texture.h>
#include <rengine/state/Streams.h>
#include <rengine/lang/debug/Debug.h>

namespace rengine
{
	//
	// Texture2D
	//

	Texture2D::Texture2D()
	{
		initialize();
	}

	Texture2D::Texture2D(DataFormat const& data_format)
	{
		initialize();
		setInternalFormat(data_format);
	}

	Texture2D::Texture2D(DataFormat const& data_format, Int const width, Int const height)
	{
		initialize();
		setSize(width, height);
		setInternalFormat(data_format);
	}

	Texture2D::Texture2D(SharedPointer<Image> const& image)
	{
		initialize();
		setImage(image);
	}


	void Texture2D::initialize()
	{
		image = 0;

		flags = Texture2D::None;
		flags |= ReleaseImage;

		width = 0;
		height = 0;
		color_channels = 0;

		setFilter(Linear, Linear);
		setWrap(Repeat, Repeat);
		flags |= AutoFilter;

		internal_format_ = Rgba8;
		format_ = Rgba;
	}

	Texture2D::~Texture2D()
	{
		release();
	}

	void Texture2D::release()
	{
		// TODO : this should be implemented with an observer
		CoreEngine::instance()->renderEngine().unloadTexture(*this);

		initialize();
	}

	void Texture2D::setImage(SharedPointer<Image> const& image)
	{
		this->image = image;

		if (this->image.get())
		{
			width = image->getWidth();
			height = image->getHeight();
			color_channels = image->getColorChannels();

			changeFlags() |= ImageDataChanged;

			//compute internal format
			switch (color_channels)
			{
				case 1:
				internal_format_ = isFlagSet(CompressTexture) ? CompressedRed : Red8;
					break;
				case 2:
				internal_format_ = isFlagSet(CompressTexture) ? CompressedRedGreen : RedGreen8;
					break;
				case 3:
				internal_format_ = isFlagSet(CompressTexture) ? CompressedRgb : Rgb8;
					break;
				case 4:
				internal_format_ = isFlagSet(CompressTexture) ? CompressedRgba : Rgba8;
					break;
			}

			//compute format
			switch (color_channels)
			{
				case 1:
				format_ = Red;
					break;
				case 2:
				format_ = RedGreen;
					break;
				case 3:
				format_ = Rgb;
					break;
				case 4:
				format_ = Rgba;
					break;
			}
		}

	}

	void Texture2D::setInternalFormat(DataFormat const& internal_format)
	{
		changeFlags() |= ImageDataChanged;

		this->internal_format_ = internal_format;

		format_ = internal_format;

		switch (internal_format)
		{
			case Red8:
			case Red16Float:
			format_ = Red;
				break;

			case RedGreen8:
			case RedGreen16Float:
			format_ = RedGreen;
				break;

			case DepthComponent16:
			case DepthComponent24:
			case DepthComponent32:
			case DepthComponent32Float:
			case Depth24Stencil8:
			case Depth32FloatStencil8:
			format_ = DepthComponent;
				break;

			case Rgb8:
			case Rgb16Float:
			case Rgb32Float:
			format_ = Rgb;
				break;

			case Rgba8:
			case Rgba16Float:
			case Rgba32Float:
			format_ = Rgba;
				break;

			case StencilIndex1:
			case StencilIndex4:
			case StencilIndex8:
			case StencilIndex16:
			format_ = StencilIndex;
				break;


			default:
			format_ = Rgba;
				break;

		}
	}

	void Texture2D::setFilter(Filter const& min, Filter const& mag)
	{
		min_filter = min;
		mag_filter = mag;

		changeFlags() |= FilterChanged;

		// disable auto filter
		flags = (flags & ~AutoFilter);
	}

	Texture2D::Filter const& Texture2D::getMinFilter() const
	{
		return min_filter;
	}

	Texture2D::Filter const& Texture2D::getMagFilter() const
	{
		return mag_filter;
	}

	void Texture2D::setWrap(Wrap const& s, Wrap const& t)
	{
		wrap_t = t;
		wrap_s = s;

		changeFlags() |= WrapChanged;
	}

	Texture2D::Wrap const& Texture2D::getWrapS() const
	{
		return wrap_s;
	}

	Texture2D::Wrap const& Texture2D::getWrapT() const
	{
		return wrap_t;
	}

	//
	// Texture2DUnit
	//


	Texture2DUnit::Texture2DUnit() :
		State(DrawStates::Texture2D), unit_(0)
	{
	}

	Texture2DUnit::Texture2DUnit(SharedPointer<Texture2D> const& texture, Unit const& unit) :
		State(DrawStates::Texture2D), texture_(texture), unit_(unit)
	{
	}

	Texture2DUnit::Texture2DUnit(Texture2DUnit const& rhs) :
		State(DrawStates::Texture2D), texture_(rhs.texture_), unit_(rhs.unit_)
	{
	}

	Texture2DUnit::~Texture2DUnit()
	{
	}

	void Texture2DUnit::setTexture(SharedPointer<Texture2D> const& texture)
	{
		texture_ = texture;
	}

	SharedPointer<Texture2D> const& Texture2DUnit::getTexture() const
	{
		return texture_;
	}

	void Texture2DUnit::setUnit(Unit const& unit)
	{
		unit_ = unit;
	}

	Texture2DUnit::Unit const& Texture2DUnit::getUnit() const
	{
		return unit_;
	}


	Int Texture2DUnit::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(Texture2DUnit, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, unit_);
		META_STATE_COMPARE_PROPERTY(casted_rhs, texture_.get());

		if (casted_rhs.texture_.get())
		{
			if (casted_rhs.texture_->changeFlags())
			{
				return 1;
			}
		}

		return 0;
	}

	//
	// Support for ostream
	//

	META_STATE_HAS_OSTREAM_IMPLEMENTATION(Texture2DUnit)

} // namespace rengine


