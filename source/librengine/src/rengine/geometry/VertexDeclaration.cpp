// __!!rengine_copyright!!__ //

#include <rengine/geometry/VertexDeclaration.h>

namespace rengine
{

	void PositionVertexDeclaration::configure(VertexBuffer& vertex_buffer)
	{
		vertex_buffer.addChannel(VertexBuffer::Position);
		vertex_buffer.construct();
	}

	void PositionTextureVertexDeclaration::configure(VertexBuffer& vertex_buffer)
	{
		vertex_buffer.addChannel(VertexBuffer::Position);
		vertex_buffer.addChannel(VertexBuffer::TexCoords);
		vertex_buffer.construct();
	}

	void PositionNormalVertexDeclaration::configure(VertexBuffer& vertex_buffer)
	{

		vertex_buffer.addChannel(VertexBuffer::Position);
		vertex_buffer.addChannel(VertexBuffer::Normal);
		vertex_buffer.construct();
	}

	void PositionColorNormalVertexDeclaration::configure(VertexBuffer& vertex_buffer)
	{
		vertex_buffer.addChannel(VertexBuffer::Position);
		vertex_buffer.addChannel(VertexBuffer::Color);
		vertex_buffer.addChannel(VertexBuffer::Normal);
		vertex_buffer.construct();
	}

	void PositionNormalTextureVertexDeclaration::configure(VertexBuffer& vertex_buffer)
	{
		vertex_buffer.addChannel(VertexBuffer::Position);
		vertex_buffer.addChannel(VertexBuffer::Normal);
		vertex_buffer.addChannel(VertexBuffer::TexCoords);
		vertex_buffer.construct();
	}



} // namespace rengine

