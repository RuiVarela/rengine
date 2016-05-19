// __!!rengine_copyright!!__ //

#ifndef __RENGINE_VERTEX_DECLARATION_H__
#define __RENGINE_VERTEX_DECLARATION_H__

#include <rengine/math/Vector.h>
#include <rengine/geometry/VertexBuffer.h>

namespace rengine {

	//
	// Default VertexDeclarations
	//

	struct PositionVertexDeclaration
	{
		Vector3D position;

		static void configure(VertexBuffer& vertex_buffer);
	};

	struct PositionTextureVertexDeclaration
	{
		Vector3D position;
		Vector2D tex_coord;

		static void configure(VertexBuffer& vertex_buffer);
	};

	struct PositionNormalVertexDeclaration
	{
		Vector3D position;
		Vector3D normal;

		static void configure(VertexBuffer& vertex_buffer);
	};

	struct PositionColorNormalVertexDeclaration
	{
		Vector3D position;
		Vector4D color;
		Vector3D normal;

		static void configure(VertexBuffer& vertex_buffer);
	};

	struct PositionNormalTextureVertexDeclaration
	{
		Vector3D position;
		Vector3D normal;
		Vector2D tex_coord;

		static void configure(VertexBuffer& vertex_buffer);
	};


} // end of namespace

#endif // __RENGINE_VERTEX_DECLARATION_H__
