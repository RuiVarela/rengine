// __!!rengine_copyright!!__ //

#ifndef __RENGINE_FRAME_BUFFER_H__
#define __RENGINE_FRAME_BUFFER_H__

#include <rengine/state/Texture.h>

namespace rengine {

	//
	// RenderBuffer
	//
	class RenderBuffer : public DrawResource
	{
	public:
		static Uint const NotDefined = 0xFFFFFF;

		RenderBuffer(DataFormat const& internal_format);
		RenderBuffer(DataFormat const& internal_format, Uint const& buffer_width, Uint const& buffer_height);
		RenderBuffer(DataFormat const& internal_format, Uint const& buffer_width, Uint const& buffer_height, Uint const& buffer_samples);

		~RenderBuffer();

		void setSize(Uint const& buffer_width, Uint const& buffer_height);
		Uint width() const;
		Uint height() const;

		void setSamples(Uint const& buffer_samples);
		Uint samples();

		DataFormat internalFormat() const;
	private:
		RenderBuffer(RenderBuffer const& copy);

		DataFormat internal_format_;
		Uint width_;
		Uint height_;
		Uint samples_;
	};

	//
	// FrameBuffer
	//
	class FrameBuffer : public DrawResource
	{
	public:
		enum AttachmentTarget
		{
			AutoTarget		= 0,
			DepthStencil	= 0x821A,
			Depth			= 0x8D00,
			Stencil			= 0x8D20,
			Color			= 0x8CE0
		};

		enum AttachmentType
		{
			TextureAttachment			= 1,
			RenderBufferAttachment		= 2
		};

		struct Attachment
		{
			AttachmentTarget target;
			AttachmentType type;

			SharedPointer<Texture2D> texture;
			SharedPointer<RenderBuffer> render_buffer;
		};

		typedef std::vector< Attachment > Attachments;

		FrameBuffer();
		FrameBuffer(Uint const& buffer_width, Uint const& buffer_height);
		FrameBuffer(Uint const& buffer_width, Uint const& buffer_height, Uint const& buffer_samples);

		~FrameBuffer();


		void attach(SharedPointer<Texture2D> const& texture, AttachmentTarget const target = AutoTarget);
		void attach(SharedPointer<RenderBuffer> render_buffer, AttachmentTarget const target = AutoTarget);

		Attachments const& attachments() const;
		Attachments& attachments();

		void setSize(Uint const& buffer_width, Uint const& buffer_height);
		Uint width() const;
		Uint height() const;

		void setSamples(Uint const& buffer_samples);
		Uint samples();
	private:
		void attach(SharedPointer<Texture2D> const& texture, SharedPointer<RenderBuffer> render_buffer, AttachmentTarget const target, AttachmentType const type);

		FrameBuffer(FrameBuffer const& copy);
		Attachments attachments_;

		Uint width_;
		Uint height_;
		Uint samples_;
	};

	RENGINE_INLINE void RenderBuffer::setSize(Uint const& buffer_width, Uint const& buffer_height)
	{
		width_ = buffer_width;
		height_ = buffer_height;
	}

	RENGINE_INLINE Uint RenderBuffer::width() const
	{
		return width_;
	}

	RENGINE_INLINE Uint RenderBuffer::height() const
	{
		return height_;
	}

	RENGINE_INLINE void RenderBuffer::setSamples(Uint const& buffer_samples)
	{
		samples_ = buffer_samples;
	}

	RENGINE_INLINE Uint RenderBuffer::samples()
	{
		return samples_;
	}

	RENGINE_INLINE DrawResource::DataFormat RenderBuffer::internalFormat() const
	{
		return internal_format_;
	}

	RENGINE_INLINE RenderBuffer::RenderBuffer(RenderBuffer const& copy)
	{
	}

	RENGINE_INLINE void FrameBuffer::setSize(Uint const& buffer_width, Uint const& buffer_height)
	{
		width_ = buffer_width;
		height_ = buffer_height;
	}

	RENGINE_INLINE Uint FrameBuffer::width() const
	{
		return width_;
	}

	RENGINE_INLINE Uint FrameBuffer::height() const
	{
		return height_;
	}

	RENGINE_INLINE FrameBuffer::FrameBuffer(FrameBuffer const& copy)
	{
	}

	RENGINE_INLINE void FrameBuffer::attach(SharedPointer<Texture2D> const& texture, AttachmentTarget const target)
	{
		attach(texture, 0, target, TextureAttachment);
	}

	RENGINE_INLINE void FrameBuffer::attach(SharedPointer<RenderBuffer> render_buffer, AttachmentTarget const target)
	{
		attach(0, render_buffer, target, RenderBufferAttachment);
	}

	RENGINE_INLINE FrameBuffer::Attachments const& FrameBuffer::attachments() const
	{
		return attachments_;
	}

	RENGINE_INLINE FrameBuffer::Attachments& FrameBuffer::attachments()
	{
		return attachments_;
	}

	RENGINE_INLINE void FrameBuffer::setSamples(Uint const& buffer_samples)
	{
		samples_ = buffer_samples;
	}

	RENGINE_INLINE Uint FrameBuffer::samples()
	{
		return samples_;
	}
}

#endif //__RENGINE_FRAME_BUFFER_H__
