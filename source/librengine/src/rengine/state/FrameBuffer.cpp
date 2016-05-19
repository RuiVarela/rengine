// __!!rengine_copyright!!__ //

#include <rengine/state/FrameBuffer.h>
#include <rengine/CoreEngine.h>

namespace rengine
{
	RenderBuffer::RenderBuffer(DataFormat const& internal_format)
		:internal_format_(internal_format), width_(0), height_(0), samples_(NotDefined)
	{
	}

	RenderBuffer::RenderBuffer(DataFormat const& internal_format, Uint const& buffer_width, Uint const& buffer_height)
		:internal_format_(internal_format), width_(buffer_width), height_(buffer_height), samples_(NotDefined)
	{
	}

	RenderBuffer::RenderBuffer(DataFormat const& internal_format, Uint const& buffer_width, Uint const& buffer_height, Uint const& buffer_samples)
		:internal_format_(internal_format), width_(buffer_width), height_(buffer_height), samples_(buffer_samples)
	{
	}

	RenderBuffer::~RenderBuffer()
	{
		// TODO : this should be implemented with an observer
		CoreEngine::instance()->renderEngine().unloadRenderBuffer(*this);
	}

	//
	// FrameBufferObject
	//
	FrameBuffer::FrameBuffer()
		:width_(0), height_(0), samples_(RenderBuffer::NotDefined)
	{
	}

	FrameBuffer::FrameBuffer(Uint const& buffer_width, Uint const& buffer_height)
		:width_(buffer_width), height_(buffer_height), samples_(RenderBuffer::NotDefined)
	{
	}

	FrameBuffer::FrameBuffer(Uint const& buffer_width, Uint const& buffer_height, Uint const& buffer_samples)
		:width_(buffer_width), height_(buffer_height), samples_(buffer_samples)
	{
	}

	FrameBuffer::~FrameBuffer()
	{
		CoreEngine::instance()->renderEngine().unloadFrameBuffer(*this);
		attachments_.clear();
	}

	void FrameBuffer::attach(SharedPointer<Texture2D> const& texture, SharedPointer<RenderBuffer> render_buffer, AttachmentTarget const target, AttachmentType const type)
	{
		Attachment attachment;
		attachment.render_buffer = render_buffer;
		attachment.texture = texture;
		attachment.type = type;
		attachment.target = target;

		DataFormat internal_format = DrawResource::Rgb;

		if (type == TextureAttachment)
		{
			if (texture->getWidth() == 0 || texture->getHeight() == 0)
			{
				texture->setSize(width(), height());
			}

			internal_format = texture->getInternalFormat();
		}
		else if (type == RenderBufferAttachment)
		{
			if (render_buffer->width() == 0 || render_buffer->height() == 0)
			{
				render_buffer->setSize(width(), height());
			}

			if (render_buffer->samples() == RenderBuffer::NotDefined)
			{
				render_buffer->setSamples(samples());
			}

			internal_format = render_buffer->internalFormat();
		}

		if (target == AutoTarget)
		{
			switch (internal_format)
			{
				case DepthComponent:
				case DepthComponent16:
				case DepthComponent24:
				case DepthComponent32:
				case DepthComponent32Float:
				attachment.target = Depth;
					break;

				case StencilIndex:
				case StencilIndex1:
				case StencilIndex4:
				case StencilIndex8:
				case StencilIndex16:
				attachment.target = Stencil;
					break;

				case Depth24Stencil8:
				case Depth32FloatStencil8:
				attachment.target = DepthStencil;
					break;

				default:
				attachment.target = Color;
					break;

			}
		}

		attachments_.push_back(attachment);
	}

}
