// __!!rengine_copyright!!__ //


#include <rengine/geometry/Heightmap.h>
#include <rengine/geometry/VertexDeclaration.h>

#include <rengine/image/Image.h>
#include <rengine/image/ImageResourceLoader.h>


#include <cstring>

namespace rengine
{
	static Real oneChannelFactor(Uchar* pointer)
	{
		return (Real(*pointer) / Real(255));
	}

	static Real threeChannelFactor(Uchar* pointer)
	{
		return (0.30f * Real(*(pointer + 0)) +
				0.59f * Real(*(pointer + 1)) +
				0.11f * Real(*(pointer + 2)) ) / Real(255);
	}

	typedef Real (*HeightScaler)(Uchar*);

	Heightmap::Heightmap()
	{
		PositionNormalVertexDeclaration::configure(*this);

		x_vertex = 0;
		z_vertex = 0;

		width_ = 1.0f;
		height_ = 1.0f;
		depth_ = 1.0f;
	}

	Bool Heightmap::load(std::string const& filename)
	{
		clear();
		index().clear();

		ImageResourceLoader image_loader;
		SharedPointer<Image> image = image_loader.load(filename);


		if (!image)
		{
			return false;
		}

		Uint image_width = image->getWidth();
		Uint image_height = image->getHeight();
		Uint image_channels = image->getColorChannels();

		x_vertex = image_width;
		z_vertex = image_height;

		Uint number_of_x_quads = (image_width - 1);
		Uint number_of_z_quads = (image_height - 1);

		if ((image_width < 2) || (image_height < 2))
		{
			return false;
		}

		HeightScaler height_scaler = oneChannelFactor;

		if (image_channels > 2)
		{
			height_scaler = threeChannelFactor;
		}

		//
		// Build Geometry
		//
		fill(image_width * image_height);
		index().resize(number_of_x_quads * number_of_z_quads * 3 * 2);

		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();

		Real x_position = 0.0f;
		Real y_position = 0.0f;
		Real z_position = 0.0f;
		for (Uint current_x = 0; current_x != image_width; current_x++)
		{
			x_position = -(width_ / 2.0f) + ((Real(current_x) / Real(image_width)) * width_);

			for (Uint current_z = 0; current_z != image_height; current_z++)
			{
				z_position = -(depth_ / 2.0f) + ((Real(current_z) / Real(image_height)) * depth_);
				y_position = -(height_ / 2.0f) + height_scaler(image->rawPixel(current_x, current_z)) * height_;

				vertex_stream[current_z * image_width + current_x].position.set(x_position, y_position, z_position);
			}
		}

		//
		// Calculate mesh index
		//
		for (Uint current_x_quad = 0; current_x_quad != number_of_x_quads; current_x_quad++)
		{
			for (Uint current_z_quad = 0; current_z_quad != number_of_z_quads; current_z_quad++)
			{
				Uint current_quad = (current_z_quad * number_of_x_quads + current_x_quad) * 3 * 2;

				//first triangle
				index()[current_quad + 0] = current_z_quad * image_width + (current_x_quad + 1);
				index()[current_quad + 1] = current_z_quad * image_width + current_x_quad;
				index()[current_quad + 2] = (current_z_quad + 1) * image_width + current_x_quad;

				//second triangle
				index()[current_quad + 3] = (current_z_quad + 1) * image_width + current_x_quad;
				index()[current_quad + 4] = (current_z_quad + 1) * image_width + (current_x_quad + 1);
				index()[current_quad + 5] = current_z_quad * image_width + (current_x_quad + 1);
			}
		}

		computeSmoothNormals();
		return true;
	}

	Vector3D Heightmap::computeFlatNormal(IndexType const x0, IndexType const z0, IndexType const x1, IndexType const z1, IndexType const x2, IndexType const z2)
	{
		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();

		Vector3D ab = vertex_stream[z1 * x_vertex + x1].position - vertex_stream[z0 * x_vertex + x0].position;
		Vector3D cb = vertex_stream[z2 * x_vertex + x2].position - vertex_stream[z0 * x_vertex + x0].position;

		Vector3D normal = ab ^ cb;
		normal.normalize();

		return normal;
	}

	void Heightmap::computeSmoothNormals()
	{

		//
		// Calculate Vertex Normals
		//
		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();

		Vector3D normal_0;
		Vector3D normal_1;
		Vector3D normal_2;
		Vector3D normal_3;
		Vector3D normal_4;
		Vector3D normal_5;

		for (Uint current_x = 0; current_x != x_vertex; current_x++)
		{
			for (Uint current_z = 0; current_z != z_vertex; current_z++)
			{
				if ((current_x == 0) && (current_z == 0)) // Corners
				{
					vertex_stream[current_z * x_vertex + current_x].normal = computeFlatNormal(1, 0, 0, 0, 0, 1);
				}
				else if ((current_x == x_vertex - 1) && (current_z == z_vertex - 1))
				{
					vertex_stream[current_z * x_vertex + current_x].normal = computeFlatNormal(current_x - 1, current_z, current_x, current_z, current_x, current_z - 1);
				}
				else if ((current_x == 0) && (current_z == z_vertex - 1))
				{
					normal_0 = computeFlatNormal(1, current_z - 1, 0, current_z - 1, 0, current_z);
					normal_1 = computeFlatNormal(0, current_z, 1, current_z, 1, current_z - 1);

					normal_0.normalize();
					normal_1.normalize();

					vertex_stream[current_z * x_vertex + current_x].normal = (normal_0 + normal_1) / 2.0f;
				}
				else if ((current_x == x_vertex - 1) && (current_z == 0))
				{
					normal_0 = computeFlatNormal(current_x, 0, current_x - 1, 0, current_x - 1, 1);
					normal_1 = computeFlatNormal(current_x - 1, 1, current_x, 1, current_x, 0);

					normal_0.normalize();
					normal_1.normalize();

					vertex_stream[current_z * x_vertex + current_x].normal = (normal_0 + normal_1) / 2.0f;
				}
				else if (current_x == 0) // borders

				{
					normal_0 = computeFlatNormal(current_x + 1, current_z - 1, current_x, current_z - 1, current_x, current_z);
					normal_1 = computeFlatNormal(current_x, current_z, current_x + 1, current_z, current_x + 1, current_z - 1);
					normal_2 = computeFlatNormal(current_x + 1, current_z, current_x, current_z, current_x, current_z + 1);

					normal_0.normalize();
					normal_1.normalize();
					normal_2.normalize();

					vertex_stream[current_z * x_vertex + current_x].normal = (normal_0 + normal_1 + normal_2) / 3.0f;
				}
				else if (current_x == x_vertex - 1)
				{
					normal_0 = computeFlatNormal(current_x - 1, current_z, current_x, current_z, current_x, current_z - 1);
					normal_1 = computeFlatNormal(current_x, current_z, current_x - 1, current_z, current_x - 1, current_z + 1);
					normal_2 = computeFlatNormal(current_x - 1, current_z + 1, current_x, current_z + 1, current_x, current_z);

					normal_0.normalize();
					normal_1.normalize();
					normal_2.normalize();

					vertex_stream[current_z * x_vertex + current_x].normal = (normal_0 + normal_1 + normal_2) / 3.0f;
				}
				else if (current_z == 0)
				{
					normal_0 = computeFlatNormal(current_x, current_z, current_x - 1, current_z, current_x - 1, current_z + 1);
					normal_1 = computeFlatNormal(current_x - 1, current_z + 1, current_x, current_z + 1, current_x, current_z);
					normal_2 = computeFlatNormal(current_x + 1, current_z, current_x, current_z, current_x, current_z + 1);

					normal_0.normalize();
					normal_1.normalize();
					normal_2.normalize();

					vertex_stream[current_z * x_vertex + current_x].normal = (normal_0 + normal_1 + normal_2) / 3.0f;
				}
				else if (current_z == z_vertex - 1)
				{
					normal_0 = computeFlatNormal(current_x - 1, current_z, current_x, current_z, current_x, current_z - 1);
					normal_1 = computeFlatNormal(current_x + 1, current_z - 1, current_x, current_z - 1, current_x, current_z);
					normal_2 = computeFlatNormal(current_x, current_z, current_x + 1, current_z, current_x + 1, current_z - 1);

					normal_0.normalize();
					normal_1.normalize();
					normal_2.normalize();

					vertex_stream[current_z * x_vertex + current_x].normal = (normal_0 + normal_1 + normal_2) / 3.0f;
				}
				else
				{
					normal_0 = computeFlatNormal(current_x + 1, current_z - 1, current_x, current_z - 1, current_x, current_z);
					normal_1 = computeFlatNormal(current_x, current_z, current_x + 1, current_z, current_x + 1, current_z - 1);
					normal_2 = computeFlatNormal(current_x + 1, current_z, current_x, current_z, current_x, current_z + 1);

					normal_3 = computeFlatNormal(current_x - 1, current_z, current_x, current_z, current_x, current_z - 1);
					normal_4 = computeFlatNormal(current_x, current_z, current_x - 1, current_z, current_x - 1, current_z + 1);
					normal_5 = computeFlatNormal(current_x - 1, current_z + 1, current_x, current_z + 1, current_x, current_z);

					normal_0.normalize();
					normal_1.normalize();
					normal_2.normalize();
					normal_3.normalize();
					normal_4.normalize();
					normal_5.normalize();

					vertex_stream[current_z * x_vertex + current_x].normal = (normal_0 + normal_1 + normal_2 + normal_3 + normal_4 + normal_5) / 6.0f;
				}

				vertex_stream[current_z * x_vertex + current_x].normal.normalize();
			}
		}
	}

} // end of namespace
