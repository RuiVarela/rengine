// __!!rengine_copyright!!__ //
#ifndef __RENGINE_MODEL3DS_H__
#define __RENGINE_MODEL3DS_H__

#include <vector>
#include <rengine/geometry/Mesh.h>

namespace rengine
{
	class Model : public Drawable
	{
	public:
		typedef std::vector< rengine::SharedPointer< Mesh > > MeshVector;

		Model();
		~Model();

		MeshVector &meshes() { return meshes_; }
		MeshVector const& meshes() const { return meshes_; }

		Uint numberOfMeshes() const { return meshes_.size(); }

		virtual void prepareDrawing(RenderEngine& render_engine);
		virtual void unprepareDrawing(RenderEngine& render_engine);
		virtual void draw(RenderEngine& render_engine);

		virtual void setDrawMode(DrawMode const draw_mode);
	private:
		MeshVector meshes_;
	};

} // end of namespace

#endif // __RENGINE__MODEL3DS_H__
