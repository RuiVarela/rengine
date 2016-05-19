/*
* R-Engine project
* 2007
*/

#ifndef __RENGINE_MODEL_LOADER_3DS_H__
#define __RENGINE_3DS_MODEL_LOADER_H__

#include <string>
#include <rengine/geometry/Model.h>

namespace rengine
{
	class ModelLoader3ds
	{
	public:
		ModelLoader3ds();
		virtual ~ModelLoader3ds();
		SharedPointer<Model> operator()(std::string const& filename);

		void setUse3dsNormals(bool use) { use_3ds_normals = use; }
		bool getUse3dsNormals() const { return use_3ds_normals; }
	private:
		bool use_3ds_normals;
	};

} // end of namespace

#endif // __RENGINE_MODEL_LOADER_3DS_H__
