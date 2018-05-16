// __!!rengine_copyright!!__ //

#include <rengine/geometry/Mesh.h>
#include <rengine/RenderEngine.h>
#include <rengine/CoreEngine.h>
#include <rengine/math/Vector.h>

#include <limits>

#include <cstring>
#include <cstdlib>

namespace rengine
{
	Mesh::Mesh()
	{
		needs_prepare_rendering = true;
	}

	Mesh::~Mesh()
	{
		//TODO: This should be implemented with an oberver
		unprepareDrawing( CoreEngine::instance()->renderEngine() );
	}

	BoundingBox const& Mesh::calculateBoundingBox()
	{
		bounding_box_.reset();

		//TODO: Implement this

	/*	for (Uint current_vertex = 0; current_vertex != vertex_.size(); ++current_vertex)
		{
			bounding_box_.merge( vertex_[current_vertex] );
		}
*/
		return bounding_box_;
	}

	void Mesh::prepareDrawing(RenderEngine &render_engine) {
		if (needs_prepare_rendering) {
			//
			// build the vertex array buffer
			//
			render_engine.loadVertexArrayObject(vertex_array_object_);
		}

		render_engine.bindVertexArrayObject(vertex_array_object_);

		//
		// build the vertex buffers objects
		//
		render_engine.loadVertexBufferObject(vertex_vbo_, *this, drawMode());

		if (numberOfIndexes() > 0) {
			render_engine.loadIndexBufferObject(indexes_vbo_, indexes_, drawMode());
		}

		if (needs_prepare_rendering) {
			//
			// bind vbo
			//
			render_engine.bindVertexBufferObject(vertex_vbo_, *this);
			if (numberOfIndexes() > 0) {
				render_engine.bindIndexBufferObject(indexes_vbo_, indexes_);
			}
		}

		needs_prepare_rendering = false;
		needs_data_refresh = false;
	}

	void Mesh::unprepareDrawing(RenderEngine& render_engine)
	{
		render_engine.unloadVertexBufferObject(vertex_vbo_);
		render_engine.unloadVertexBufferObject(indexes_vbo_);
		render_engine.unloadVertexArrayObject(vertex_array_object_);
	}

	void Mesh::draw(RenderEngine &render_engine) {
		if (needs_prepare_rendering || (needs_data_refresh && drawMode() != StaticDraw)) {
			prepareDrawing(render_engine);
		}


		if (numberOfIndexes() > 0) {
			render_engine.drawVertexArrayObject(vertex_array_object_, indexes_);
		} else {
			render_engine.drawVertexArrayObject(vertex_array_object_, *this);
		}
	}

	//
	// Smooth Normal calculation
	//

	void Mesh::computeSmoothNormals()
	{
		//TODO: Implement this
		/*
		if (vertex().size() == 0)
		{
			return;
		}

		Uint number_of_vertex = vertex().size();

		//
		// Index Table. Associates a vertex to the corresponding triangles
		//
		typedef Mesh::IndexType IndexType;
		Uint const max_faces_for_normal = 50;

		IndexType *index_counter = (IndexType*) rg_malloc(sizeof(IndexType) * number_of_vertex);
		if (!index_counter)
		{
			throw GfxException(501, "Out of Memory : Smooth Normal Computation");
		}

		IndexType *index_table = (IndexType*) rg_malloc(sizeof(IndexType) * number_of_vertex * max_faces_for_normal);
		if (!index_table)
		{
			rg_free(index_counter);
			throw GfxException(501, "Out of Memory : Smooth Normal Computation");
		}

		memset(index_counter, 0, sizeof(IndexType) * number_of_vertex);
		memset(index_table, 0, sizeof(IndexType) * number_of_vertex * max_faces_for_normal);

		IndexType current_index = 0;
		for (Uint triangle = 0; triangle != numberOfFaces(); ++triangle)
		{
			for (Uint i = 0; i != 3; ++i)
			{
				current_index = index()[triangle * 3 + i];
				if (index_counter[current_index] < max_faces_for_normal)
				{
					index_table[index_counter[current_index] * number_of_vertex + current_index] = triangle;
					index_counter[current_index]++;
				}
			}
		}

		//
		// Calculate Flat Normals
		//
		std::vector<Vector3D> triangle_normals(numberOfFaces());
		for (Uint triangle = 0; triangle != numberOfFaces(); ++triangle)
		{
			Vector3D ab = vertex()[ index()[triangle * 3 + 1] ] - vertex()[ index()[triangle * 3 + 0] ];
			Vector3D cb = vertex()[ index()[triangle * 3 + 2] ] - vertex()[ index()[triangle * 3 + 0] ];

			triangle_normals[triangle] = ab ^ cb;
			triangle_normals[triangle].normalize();
		}

		//
		// Calculate Vertex Normals
		//
		normal().resize(number_of_vertex);
		memset(normalArray(), 0, number_of_vertex);

		for (Uint current_vextex = 0; current_vextex != number_of_vertex; ++current_vextex)
		{
			for (Uint current_triangle = 0; current_triangle != index_counter[current_vextex]; ++current_triangle)
			{
				current_index = index_table[current_triangle * number_of_vertex + current_vextex];
				normal()[current_vextex] += triangle_normals[current_index];
			}

			if (index_counter[current_vextex])
			{
				normal()[current_vextex] /= Real( index_counter[current_vextex] );
				normal()[current_vextex].normalize();
			}
		}

		rg_free(index_counter);
		rg_free(index_table);

	*/
	}

} //end of namespace


