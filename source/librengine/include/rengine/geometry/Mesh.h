// __!!rengine_copyright!!__ //

#ifndef __RENGINE_MESH_H__
#define __RENGINE_MESH_H__


#include <rengine/math/BoundingVolume.h>
#include <rengine/math/Vector.h>
#include <rengine/geometry/Drawable.h>
#include <rengine/geometry/VertexBuffer.h>
#include <rengine/state/DrawResource.h>

#include <vector>
#include <string>

namespace rengine
{
	class Mesh : public VertexBuffer, public Drawable
	{
	public:
		Mesh();
		~Mesh();

		IndexVector const& index() const;
		IndexVector & index();
		IndexType* indexArray();

		Uint numberOfIndexes() const;

		void computeSmoothNormals();

		BoundingBox const& calculateBoundingBox();
		BoundingBox const& boundingBox() const;

		virtual void prepareDrawing(RenderEngine& render_engine);
		virtual void unprepareDrawing(RenderEngine& render_engine);
		virtual void draw(RenderEngine& render_engine);


		VertexBufferObject& vertexVbo();
		VertexBufferObject const& vertexVbo() const;

		VertexBufferObject& indexVbo();
		VertexBufferObject const& indexVbo() const;
	private:
		IndexVector indexes_;
		VertexBufferObject vertex_vbo_;
		VertexBufferObject indexes_vbo_;

		BoundingBox bounding_box_;
	};

	//
	// Implementation
	//

	RENGINE_INLINE Drawable::IndexVector & Mesh::index()
	{
		return indexes_;
	}

	RENGINE_INLINE Drawable::IndexVector const& Mesh::index() const
	{
		return indexes_;
	}

	RENGINE_INLINE Drawable::IndexType* Mesh::indexArray()
	{
		return &indexes_[0];
	}

	RENGINE_INLINE Uint Mesh::numberOfIndexes() const
	{
		return indexes_.size();
	}


	RENGINE_INLINE BoundingBox const& Mesh::boundingBox() const
	{
		return bounding_box_;
	}



} // end of namespace

#endif // __RENGINE__MESH_H__
