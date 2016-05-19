#include <3ds/3ds.h>
#include <3ds/ModelLoader3ds.h>
#include <rengine/geometry/Model.h>

#include <rengine/CoreEngine.h>


#include <cmath>
#include <cstring>
#include <iostream>

rengine::ModelLoader3ds::ModelLoader3ds()
{
	use_3ds_normals = true;
}

rengine::ModelLoader3ds::~ModelLoader3ds()
{
}

rengine::SharedPointer<rengine::Model> rengine::ModelLoader3ds::operator()(std::string const& filename)
{
	//TODO: Implement this
	return new rengine::Model();

	/*

	data3ds_t data;
	memset(&data, 0, sizeof(data3ds_t));

	Read3dsFile(filename.c_str(), &data);
	if (data.triCount == 0 )
	{
		throw GfxException( 500, "Unable to load 3ds Model" + filename );
	}


	if (use_3ds_normals)
	{
		Calculate3dsNormals(&data);
	}

//	Calculate3dsBoundingBox(&data);
//	Calculate3dsTangentSpace(&data);

	rengine::SharedPointer<rengine::Model> model = new rengine::Model();

	unsigned int number_of_valid_meshes = 0;

	for (int i = 0; i != data.meshCount; ++i)
	{
		if (data.meshes[i].vertCount)
		{
			++number_of_valid_meshes;
		}
	}

	//
	// Copy Data
	//
	model->meshes().resize(number_of_valid_meshes);
	unsigned int valid_mesh = 0;
	for (int i = 0; i != data.meshCount; ++i)
	{
		if (data.meshes[i].vertCount)
		{
			model->meshes()[valid_mesh] = new rengine::Mesh();

			model->meshes()[valid_mesh]->vertex().resize( data.meshes[i].vertCount );
			memcpy(model->meshes()[valid_mesh]->vertexArray(), data.meshes[i].verts, data.meshes[i].vertCount * sizeof(Mesh::DataType));

			model->meshes()[valid_mesh]->index().resize( data.meshes[i].triCount * 3);
			memcpy(model->meshes()[valid_mesh]->indexArray(), data.meshes[i].tris, data.meshes[i].triCount * sizeof(long) * 3);

			if (use_3ds_normals)
			{
				model->meshes()[valid_mesh]->normal().resize( data.meshes[i].triCount * 3);
				memcpy(model->meshes()[valid_mesh]->normalArray(), data.meshes[i].norms, data.meshes[i].triCount * sizeof(Mesh::DataType) * 3);
				model->meshes()[i]->setNormalBinding(Drawable::PrimitiveVertexBinding);
			}
			else
			{
				model->meshes()[i]->computeSmoothNormals();
			}

			++valid_mesh;
		}
	}


	Free3dsData(&data);

	return model;
	*/
}
