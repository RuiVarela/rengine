#include "UnitTest/UnitTest.h"

#include <rengine/geometry/VertexBuffer.h>
#include <rengine/geometry/VertexDeclaration.h>

#include <rengine/math/Vector.h>
#include <rengine/math/Streams.h>

#include <cstdlib>

using namespace rengine;


//
// UnitTestVertexBuffer
//

UNITT_TEST_BEGIN_CLASS(UnitTestVertexBuffer)

	virtual void run()
	{
		{
			VertexBuffer vertex_buffer;

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 0);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 0);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channelsEnabled(), 0);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.isValid(), false);

			vertex_buffer.addChannel(VertexBuffer::Position);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channelsEnabled(), 1);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[0].semantic, VertexBuffer::Position);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[0].component_size, VertexBuffer::SizeType(sizeof(Vector3D::ValueType)));
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[0].number_of_components, 3);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[0].offset, 0);

			vertex_buffer.addChannel(VertexBuffer::Normal);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channelsEnabled(), 2);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[1].semantic, VertexBuffer::Normal);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[1].component_size, VertexBuffer::SizeType(sizeof(Vector3D::ValueType)));
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[1].number_of_components, 3);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[1].offset, 3 * VertexBuffer::SizeType(sizeof(Vector3D::ValueType)));

			vertex_buffer.addChannel(VertexBuffer::Color);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channelsEnabled(), 3);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[2].semantic, VertexBuffer::Color);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[2].component_size, VertexBuffer::SizeType(sizeof(Vector4D::ValueType)));
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[2].number_of_components, 4);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.channels()[2].offset, (3 + 3) * VertexBuffer::SizeType(sizeof(Vector3D::ValueType)));


			vertex_buffer.construct();
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 0);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 2);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.isValid(), true);
			UNITT_FAIL_NOT_EQUAL(sizeof(Vector3D::ValueType) * 3 +
								 sizeof(Vector3D::ValueType) * 3 +
								 sizeof(Vector4D::ValueType) * 4 , size_t( vertex_buffer.vertexSize() ));
		}


		{
			VertexBuffer vertex_buffer;
			vertex_buffer.addChannel(VertexBuffer::Position);
			vertex_buffer.construct(10);

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 10);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 16);
		}

		{
			VertexBuffer vertex_buffer;
			vertex_buffer.addChannel(VertexBuffer::Position);
			vertex_buffer.construct(31, 15);

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 31);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 32);
		}

		{
			VertexBuffer vertex_buffer;
			vertex_buffer.addChannel(VertexBuffer::Position);
			vertex_buffer.construct(1, 100);

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 1);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 128);
		}

		{
			VertexBuffer vertex_buffer;
			vertex_buffer.addChannel(VertexBuffer::Position);
			vertex_buffer.construct(0, 129);

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 0);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 256);
		}

		{
			VertexBuffer vertex_buffer;
			vertex_buffer.addChannel(VertexBuffer::Position);
			vertex_buffer.construct();

			vertex_buffer.reserve(10);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 16);

			vertex_buffer.reserve(17);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 32);

			vertex_buffer.reserve(384);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 512);

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 0);
		}

		{
			VertexBuffer vertex_buffer;
			vertex_buffer.addChannel(VertexBuffer::Position);
			vertex_buffer.construct();
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 2);

			VertexBuffer::DataPointer element = (VertexBuffer::DataPointer) rg_malloc(vertex_buffer.vertexSize());


			vertex_buffer.push_back(element);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 1);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 2);

			vertex_buffer.push_back(element);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 2);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 2);

			vertex_buffer.push_back(element);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 3);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 4);

			vertex_buffer.push_back(element);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 4);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 4);

			vertex_buffer.push_back(element);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 5);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 8);


			for (int i = 0; i != 20; ++ i)
			{
				vertex_buffer.push_back(element);
			}

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 25);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 32);


			for (int i = 0; i != 20; ++ i)
			{
				vertex_buffer.pop_back();
			}

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 5);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 32);

			vertex_buffer.clear();

			UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), 0);
			UNITT_FAIL_NOT_EQUAL(vertex_buffer.capacity(), 32);

			rg_free(element);
		}

		{
			VertexBuffer vertex_buffer;
			PositionVertexDeclaration::configure(vertex_buffer);


			std::vector<PositionVertexDeclaration> vertex_vector;

			PositionVertexDeclaration vertex;
			vertex.position.x() = 1.0f;
			vertex.position.y() = 2.0f;
			vertex.position.z() = 3.0f;

			Vector3D increment(10.0f, 11.0f, 12.0f);

			VertexBuffer::Interface<PositionVertexDeclaration> interface = vertex_buffer.interface<PositionVertexDeclaration>();

			for (std::vector<PositionVertexDeclaration>::size_type i = 0; i != 100; ++i)
			{
				vertex_vector.push_back(vertex);
				interface.add(vertex);

				vertex.position += increment;

				UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), VertexBuffer::SizeType( i + 1 ));
			}

			for (std::vector<PositionVertexDeclaration>::size_type i = 0; i != 100; ++i)
			{
				PositionVertexDeclaration match = interface[i];
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].position, match.position);
			}

			for (std::vector<PositionVertexDeclaration>::size_type i = 0; i != 100; ++i)
			{
				PositionVertexDeclaration const& match = interface[i];
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].position, match.position);
			}
		}

		{
			VertexBuffer vertex_buffer;
			PositionColorNormalVertexDeclaration::configure(vertex_buffer);

			std::vector<PositionColorNormalVertexDeclaration> vertex_vector;

			PositionColorNormalVertexDeclaration vertex;
			vertex.position.x() = 1.0f;
			vertex.position.y() = 2.0f;
			vertex.position.z() = 3.0f;

			vertex.normal.x() = 4.0f;
			vertex.normal.y() = 5.0f;
			vertex.normal.z() = 6.0f;

			vertex.color.r() = 7.0f;
			vertex.color.g() = 8.0f;
			vertex.color.b() = 9.0f;
			vertex.color.a() = 10.0f;

			Vector3D position_increment(10.3f, 11.0f, 12.1f);
			Vector3D normal_increment(13.0f, 41.2f, 442.32f);
			Vector4D color_increment(3.0f, 4.25f, 9.32f, 0.3f);

			VertexBuffer::Interface<PositionColorNormalVertexDeclaration> interface = vertex_buffer.interface<PositionColorNormalVertexDeclaration>();

			std::vector<PositionColorNormalVertexDeclaration>::size_type max = 2048;

			for (std::vector<PositionColorNormalVertexDeclaration>::size_type i = 0; i != max; ++i)
			{
				vertex_vector.push_back(vertex);
				interface.add(vertex);

				vertex.position += position_increment;
				vertex.normal += normal_increment;
				vertex.color += color_increment;

				UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), VertexBuffer::SizeType( i + 1 ) );
			}

			for (std::vector<PositionColorNormalVertexDeclaration>::size_type i = 0; i != max; ++i)
			{
				PositionColorNormalVertexDeclaration match = interface[i];
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].position, match.position);
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].normal, match.normal);
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].color, match.color);
			}

			for (std::vector<PositionColorNormalVertexDeclaration>::size_type i = 0; i != max; ++i)
			{
				PositionColorNormalVertexDeclaration const& match = interface[i];
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].position, match.position);
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].normal, match.normal);
				UNITT_FAIL_NOT_EQUAL(vertex_vector[i].color, match.color);
			}
		}

		{
				VertexBuffer vertex_buffer;
				PositionColorNormalVertexDeclaration::configure(vertex_buffer);

				std::vector<PositionColorNormalVertexDeclaration> vertex_vector;

				PositionColorNormalVertexDeclaration vertex;
				vertex.position.x() = 1.0f;
				vertex.position.y() = 2.0f;
				vertex.position.z() = 3.0f;

				vertex.normal.x() = 4.0f;
				vertex.normal.y() = 5.0f;
				vertex.normal.z() = 6.0f;

				vertex.color.r() = 7.0f;
				vertex.color.g() = 8.0f;
				vertex.color.b() = 9.0f;
				vertex.color.a() = 10.0f;

				Vector3D position_increment(10.3f, 11.0f, 12.1f);
				Vector3D normal_increment(13.0f, 41.2f, 442.32f);
				Vector4D color_increment(3.0f, 4.25f, 9.32f, 0.3f);

				VertexBuffer::Interface<PositionColorNormalVertexDeclaration> interface = vertex_buffer.interface<PositionColorNormalVertexDeclaration>();
				VertexBuffer::Interface<PositionColorNormalVertexDeclaration> const const_interface = vertex_buffer.interface<PositionColorNormalVertexDeclaration>();

				std::vector<PositionColorNormalVertexDeclaration>::size_type max = 2048;

				for (std::vector<PositionColorNormalVertexDeclaration>::size_type i = 0; i != max; ++i)
				{
					vertex_vector.push_back(vertex);
					interface.add(vertex);

					vertex.position += position_increment;
					vertex.normal += normal_increment;
					vertex.color += color_increment;

					UNITT_FAIL_NOT_EQUAL(vertex_buffer.size(), VertexBuffer::SizeType( i + 1 ));
				}

				for (std::vector<PositionColorNormalVertexDeclaration>::size_type i = 0; i != max; ++i)
				{
					PositionColorNormalVertexDeclaration match = const_interface[i];
					UNITT_FAIL_NOT_EQUAL(vertex_vector[i].position, match.position);
					UNITT_FAIL_NOT_EQUAL(vertex_vector[i].normal, match.normal);
					UNITT_FAIL_NOT_EQUAL(vertex_vector[i].color, match.color);
				}

				for (std::vector<PositionColorNormalVertexDeclaration>::size_type i = 0; i != max; ++i)
				{
					PositionColorNormalVertexDeclaration const& match = const_interface[i];
					UNITT_FAIL_NOT_EQUAL(vertex_vector[i].position, match.position);
					UNITT_FAIL_NOT_EQUAL(vertex_vector[i].normal, match.normal);
					UNITT_FAIL_NOT_EQUAL(vertex_vector[i].color, match.color);
				}
			}
	}

UNITT_TEST_END_CLASS(UnitTestVertexBuffer)
