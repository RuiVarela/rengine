// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>
#include <rengine/geometry/Model.h>

namespace rengine
{
	Model::Model()
	{
	}

	Model::~Model()
	{
		//TODO: This should be implemented with an oberver
		unprepareDrawing( CoreEngine::instance()->renderEngine() );
	}

	void Model::prepareDrawing(RenderEngine& render_engine)
	{
		for (MeshVector::size_type i = 0; i != meshes_.size(); ++i)
		{
			meshes_[i]->prepareDrawing(render_engine);
		}
	}

	void Model::unprepareDrawing(RenderEngine& render_engine)
	{
		for (MeshVector::size_type i = 0; i != meshes_.size(); ++i)
		{
			meshes_[i]->unprepareDrawing(render_engine);
		}
	}

	void Model::setDrawMode(DrawMode const draw_mode)
	{
		Drawable::setDrawMode(draw_mode);

		for (MeshVector::size_type i = 0; i != meshes_.size(); ++i)
		{
			meshes_[i]->setDrawMode(draw_mode);
		}
	}

	void Model::draw(RenderEngine& render_engine)
	{
		for (MeshVector::size_type i = 0; i != meshes_.size(); ++i)
		{
			meshes_[i]->draw(render_engine);
		}
	}

} // namespace rengine

