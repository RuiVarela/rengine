// __!!rengine_copyright!!__ //

#ifndef __RENGINE_TEXTURE_H__
#define __RENGINE_TEXTURE_H__

#include <rengine/image/Image.h>
#include <rengine/state/DrawStates.h>
#include <rengine/state/DrawResource.h>
#include <rengine/RenderEngine.h>

#include <string>

namespace rengine
{
	//
	// Texture2D
	//

	class Texture2D : public DrawResource
	{
	public:
		enum Wrap
		{
			Clamp	= 0x812F, //Clamp to Edge
			Repeat	= 0x2901
		};


		enum Filter
		{
			Nearest					= 0x2600,
			Linear					= 0x2601,
			NearestMipmapNearest	= 0x2700,
			LinearMipmapNearest		= 0x2701,
			NearestMipmapLinear		= 0x2702,
			LinearMipmapLinear		= 0x2703
		};

		enum Flag
		{
			None				= 0,
			CompressTexture		= 1,
			GenerateMipmap		= 2,
			AutoFilter			= 4,
			ReleaseImage		= 8,

			WrapChanged			= 1024,
			FilterChanged 		= 2048,
			ImageDataChanged	= 4096
		};

		Texture2D();
		Texture2D(DataFormat const& data_format);
		Texture2D(DataFormat const& data_format, Int const width, Int const height);

		Texture2D(SharedPointer<Image> const& image);
		~Texture2D();

		Bool loaded() const;

		void setImage(SharedPointer<Image> const& image);
		void release();

		void setFilter(Filter const& min, Filter const& mag);
		Filter const& getMinFilter() const;
		Filter const& getMagFilter() const;

		void setWrap(Wrap const& s, Wrap const& t);
		Wrap const& getWrapS() const;
		Wrap const& getWrapT() const;

		void setFlags(Uint const& flags);
		Uint const& getFlags() const;
		Bool isFlagSet(Uint const& flag) const;

		SharedPointer<Image> const& getImage() const;

		void setSize(Int const& width, Int const& height);
		void setColorChannels(Int const& channels);
		void setInternalFormat(DataFormat const& internal_format);

		Int getWidth() const;
		Int getHeight() const;
		Int getColorChannels() const;

		DataFormat getInternalFormat() const;
		DataFormat getFormat() const;
	private:
		void initialize();
		Uint flags;

		Int width;
		Int height;
		Int color_channels;

		DataFormat internal_format_;
		DataFormat format_;

		Filter min_filter;
		Filter mag_filter;
		Wrap wrap_s;
		Wrap wrap_t;

		SharedPointer<Image> image;
	};

	//
	// Texture2DUnit
	//

	class Texture2DUnit : public DrawStates::State
	{
	public:
		typedef Uint Unit;

		META_STATE_FUNCTIONS(Texture2DUnit);
		META_STATE_HAS_OSTREAM_DECLARATION()

		Texture2DUnit();
		Texture2DUnit(SharedPointer<Texture2D> const& texture, Unit const& unit = 0);
		Texture2DUnit(Texture2DUnit const& rhs);
		~Texture2DUnit();

		void setTexture(SharedPointer<Texture2D> const& texture);
		SharedPointer<Texture2D> const& getTexture() const;

		void setUnit(Unit const& unit);
		Unit const& getUnit() const;

		virtual Int compare(DrawStates::State const& rhs) const;
	private:
		SharedPointer<Texture2D> texture_;
		Unit unit_;
	};

	//
	// Implementation
	//

	//
	// Texture2D
	//

	RENGINE_INLINE void Texture2D::setFlags(Uint const& flags)
	{
		this->flags = flags;
	}

	RENGINE_INLINE Uint const& Texture2D::getFlags() const
	{
		return flags;
	}

	RENGINE_INLINE Bool Texture2D::isFlagSet(Uint const& flag) const
	{
		return (flags & flag) != 0;
	}

	RENGINE_INLINE SharedPointer<Image> const& Texture2D::getImage() const
	{
		return image;
	}

	RENGINE_INLINE Int Texture2D::getWidth() const
	{
		return width;
	}

	RENGINE_INLINE Int Texture2D::getHeight() const
	{
		return height;
	}

	RENGINE_INLINE Int Texture2D::getColorChannels() const
	{
		return color_channels;
	}

	RENGINE_INLINE void Texture2D::setSize(Int const& width, Int const& height)
	{
		this->width = width;
		this->height = height;
	}

	RENGINE_INLINE void Texture2D::setColorChannels(Int const& channels)
	{
		color_channels = channels;
	}

	RENGINE_INLINE DrawResource::DataFormat Texture2D::getInternalFormat() const
	{
		return internal_format_;
	}

	RENGINE_INLINE DrawResource::DataFormat Texture2D::getFormat() const
	{
		return format_;
	}
}

#endif //__RENGINE_RENGINE_H__

