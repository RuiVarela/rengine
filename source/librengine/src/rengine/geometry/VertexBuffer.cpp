// __!!rengine_copyright!!__ //

#include <rengine/geometry/VertexBuffer.h>
#include <rengine/math/Vector.h>
#include <rengine/math/Math.h>
#include <rengine/string/String.h>
#include <rengine/lang/debug/Debug.h>

#include <cstdlib>
#include <cstring>

namespace rengine
{
	static const VertexBuffer::SizeType default_initial_capacity = 2;
	static const VertexBuffer::SizeType grow_factor = 2;


	VertexBuffer::VertexBuffer()
		:data_(0), capacity_(0), size_(0), vertex_size_(0), constructed_(false)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
		if (data_)
		{
			delete[](data_);
			data_ = 0;
		}
	}

	void VertexBuffer::construct()
	{
		construct(0, 2);
	}

	void VertexBuffer::construct(SizeType const& initial_size)
	{
		construct(initial_size, 0);
	}

	void VertexBuffer::construct(SizeType const& initial_size, SizeType const& minimum_capacity)
	{
		RENGINE_ASSERT(!constructed_);
		RENGINE_ASSERT(channelsEnabled() > 0);

		if (data_)
		{
			delete[](data_);
			data_ = 0;
		}

		size_ = initial_size;
		capacity_ = default_initial_capacity;

		//calculate vertex size
		vertex_size_ = 0;
		for (Channels::size_type i = 0; i != channels_.size(); ++i)
		{
			vertex_size_ += channels_[i].component_size * channels_[i].number_of_components;
		}

		RENGINE_ASSERT(vertex_size_ > 0);

		reserve( maximum(size_, maximum(minimum_capacity, capacity_)) );

		constructed_ = (data_ != 0);
	}

	void VertexBuffer::addChannel(Semantic const& semantic, SizeType const& component_size, SizeType const& number_of_components)
	{
		RENGINE_ASSERT(!constructed_);

		Channel channel;

		channel.semantic = semantic;
		channel.component_size = component_size;
		channel.offset = 0;
		channel.number_of_components = number_of_components;

		for (Channels::size_type i = 0; i != channels_.size(); ++i)
		{
			channel.offset += channels_[i].component_size * channels_[i].number_of_components;
		}

		channels_.push_back(channel);
	}

	void VertexBuffer::push_back(ConstDataPointer element)
	{
		RENGINE_ASSERT(constructed_);

		if (size_ == capacity_)
		{
			reserve(capacity_ + 1);

			if (data_ == 0)
			{
				constructed_ = false;
				return;
			}
		}

		memcpy(data_ + (size_ * vertex_size_), element, vertex_size_);
		size_++;
	}

	void VertexBuffer::pop_back()
	{
		RENGINE_ASSERT(constructed_);
		RENGINE_ASSERT(size_ > 0);

		size_--;
	}

	void VertexBuffer::clear()
	{
		RENGINE_ASSERT(constructed_);
		size_ = 0;
	}

	Bool VertexBuffer::empty() const
	{
		RENGINE_ASSERT(constructed_);
		return (size_ == 0);
	}

	void VertexBuffer::reserve(SizeType const& minimum_capacity)
	{
		if (minimum_capacity >= capacity_)
		{
			SizeType next_capacity = capacity_;
			while (next_capacity < minimum_capacity)
			{
				next_capacity *= grow_factor;
			}

			DataPointer data_block = new DataType[next_capacity * vertex_size_];
			if (data_block)
			{
				//memset(data_block, 0, next_capacity * vertex_size_);

				if (data_)
				{
					memcpy(data_block, data_, capacity_ * vertex_size_);
					delete[](data_);
					data_ = 0;
				}
				else if (size_ > 0)
				{
					RENGINE_ASSERT(size_ <= next_capacity);
					memset(data_block, 0, size_ * vertex_size_);
				}

				data_ = data_block;
				capacity_ = next_capacity;
			}
		}
	}

	void VertexBuffer::fill(SizeType const& size, Uint const value)
	{
		RENGINE_ASSERT(constructed_);

		reserve(size);
		size_ = size;

		memset(data_, value, size_ * vertex_size_);
	}


	void VertexBuffer::addChannel(Semantic const& semantic)
	{
		SizeType component_size = 0;
		SizeType number_of_components = 0;

		switch (semantic)
		{
			case GenericVector3:
			case Position:
			case Normal:
			case Binormal:
			case Tangent:
			{
				component_size = sizeof(Vector3D::ValueType);
				number_of_components = 3;
			}
				break;

			case GenericVector4:
			case Color:
			{
				component_size = sizeof(Vector4D::ValueType);
				number_of_components = 4;
			}
				break;

			case GenericVector2:
			case TexCoords:
			{
				component_size = sizeof(Vector2D::ValueType);
				number_of_components = 2;
			}
				break;

			case GenericFloat:
			{
				component_size = sizeof(Real);
				number_of_components = 1;
			}
				break;


			default:
				break;
		}

		addChannel(semantic, component_size, number_of_components);
	}

#define RENGINE_SEMANTIC_TEST(type) else if (equalCaseInsensitive(value, #type)) { semantic = type; }
#define RENGINE_SEMANTIC_TEST_STR(type) else if (value == type) { semantic = #type; }
	VertexBuffer::Semantic VertexBuffer::semanticFromString(std::string value)
	{
		Semantic semantic = None;

		if (false) {}
		RENGINE_SEMANTIC_TEST(GenericFloat)
		RENGINE_SEMANTIC_TEST(GenericVector2)
		RENGINE_SEMANTIC_TEST(GenericVector3)
		RENGINE_SEMANTIC_TEST(GenericVector4)
		RENGINE_SEMANTIC_TEST(Position)
		RENGINE_SEMANTIC_TEST(Color)
		RENGINE_SEMANTIC_TEST(Normal)
		RENGINE_SEMANTIC_TEST(Binormal)
		RENGINE_SEMANTIC_TEST(Tangent)
		RENGINE_SEMANTIC_TEST(TexCoords)

		return semantic;
	}

	std::string VertexBuffer::semanticToString(Semantic const& value)
	{
		std::string semantic = "None";

		if (false) {}
		RENGINE_SEMANTIC_TEST_STR(GenericFloat)
		RENGINE_SEMANTIC_TEST_STR(GenericVector2)
		RENGINE_SEMANTIC_TEST_STR(GenericVector3)
		RENGINE_SEMANTIC_TEST_STR(GenericVector4)
		RENGINE_SEMANTIC_TEST_STR(Position)
		RENGINE_SEMANTIC_TEST_STR(Color)
		RENGINE_SEMANTIC_TEST_STR(Normal)
		RENGINE_SEMANTIC_TEST_STR(Binormal)
		RENGINE_SEMANTIC_TEST_STR(Tangent)
		RENGINE_SEMANTIC_TEST_STR(TexCoords)

		return semantic;
	}
} // namespace rengine

