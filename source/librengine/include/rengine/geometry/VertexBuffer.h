// __!!rengine_copyright!!__ //
#ifndef __RENGINE_VERTEX_BUFFER_H__
#define __RENGINE_VERTEX_BUFFER_H__

#include <rengine/lang/Lang.h>
#include <rengine/lang/Idioms.h>
#include <rengine/lang/debug/Debug.h>
#include <vector>
#include <string>


namespace rengine
{
	class VertexBuffer : public NonCopyable
	{
	public:
		enum Semantic
		{
			None				= 0,
			GenericFloat		= 1,
			GenericVector2		= 2,
			GenericVector3		= 3,
			GenericVector4		= 4,
			Position			= 5,
			Color				= 6,
			Normal				= 7,
			Binormal			= 8,
			Tangent				= 9,
			TexCoords			= 10
		};

		typedef Uint32 SizeType;
		typedef Uchar DataType;
		typedef DataType * DataPointer;
		typedef DataType const * const ConstDataPointer;


		struct Channel
		{
			Semantic semantic;
			SizeType offset;
			SizeType component_size;
			SizeType number_of_components;
		};
		typedef std::vector<Channel> Channels;

		template <typename T>
		class Interface
		{
			public:
				typedef T VertexDeclaration;

				void add(VertexDeclaration const& vertex);

				VertexDeclaration& operator[] (SizeType const i);
				VertexDeclaration const& operator[] (SizeType const i) const;
			private:
				Interface(VertexBuffer* buffer);
				VertexBuffer* buffer_;
				friend class VertexBuffer;
		};

		template <typename T>
		class ConstInterface
		{
			public:
				typedef T VertexDeclaration;
				VertexDeclaration const& operator[] (SizeType const i) const;
			private:
				ConstInterface(VertexBuffer const * const buffer);
				VertexBuffer const * const buffer_;
				friend class VertexBuffer;
		};

		VertexBuffer();

		void construct();
		void construct(SizeType const& initial_size);
		void construct(SizeType const& initial_size, SizeType const& minimum_capacity);

		void addChannel(Semantic const& semantic);
		void addChannel(Semantic const& semantic, SizeType const& component_size, SizeType const& number_of_components);

		~VertexBuffer();

		//
		// Reserves minimum_capacity elements
		// Does not affect the size, it only affects capacity
		//
		void reserve(SizeType const& minimum_capacity);

		//
		// Fills size elements on the array
		// It needed expands size and capacity
		void fill(SizeType const& size, Uint const value = 0);

		SizeType const& size() const;
		SizeType const& capacity() const;
		void push_back(ConstDataPointer element);
		void pop_back();
		void clear();
		Bool empty() const;

		DataPointer data();
		ConstDataPointer data() const;

		Channels& channels();
		Channels const& channels() const;
		Uint channelsEnabled() const;

		Bool isValid();

		SizeType const& vertexSize() const;


		// interface methods
		template <typename T>
		Interface<T> interface();

		template <typename T>
		ConstInterface<T> const constInterface() const;

		static Semantic semanticFromString(std::string value);
		static std::string semanticToString(Semantic const& value);
	private:

		DataPointer data_;
		SizeType capacity_;
		SizeType size_;
		SizeType vertex_size_;
		Bool constructed_;
		Channels channels_;
	};

	//
	// Implementation
	//

	RENGINE_INLINE VertexBuffer::SizeType const& VertexBuffer::size() const
	{
		return size_;
	}

	RENGINE_INLINE VertexBuffer::SizeType const& VertexBuffer::capacity() const
	{
		return capacity_;
	}

	RENGINE_INLINE VertexBuffer::DataPointer VertexBuffer::data()
	{
		return data_;
	}

	RENGINE_INLINE VertexBuffer::ConstDataPointer VertexBuffer::data() const
	{
		return data_;
	}

	RENGINE_INLINE VertexBuffer::Channels& VertexBuffer::channels()
	{
		return channels_;
	}

	RENGINE_INLINE VertexBuffer::Channels const& VertexBuffer::channels() const
	{
		return channels_;
	}

	RENGINE_INLINE Bool VertexBuffer::isValid()
	{
		return constructed_;
	}

	RENGINE_INLINE Uint VertexBuffer::channelsEnabled() const
	{
		return Uint(channels_.size() );
	}

	RENGINE_INLINE VertexBuffer::SizeType const& VertexBuffer::vertexSize() const
	{
		return vertex_size_;
	}

	//
	// VertexBuffer Interface
	//
	template <typename T>
	RENGINE_INLINE VertexBuffer::Interface<T> VertexBuffer::interface()
	{
		return VertexBuffer::Interface<T>(this);
	}

	template <typename T>
	RENGINE_INLINE VertexBuffer::ConstInterface<T> const VertexBuffer::constInterface() const
	{
		return VertexBuffer::ConstInterface<T>(this);
	}



	template <typename T>
	RENGINE_INLINE VertexBuffer::Interface<T>::Interface(VertexBuffer* buffer)
	: buffer_(buffer)
	{

	}

	template <typename T>
	RENGINE_INLINE void VertexBuffer::Interface<T>::add(T const& vertex)
	{
		RENGINE_ASSERT(buffer_);
		RENGINE_ASSERT(sizeof(T) >= buffer_->vertexSize());

		buffer_->push_back( reinterpret_cast<ConstDataPointer>(&vertex) );
	}

	template <typename T>
	RENGINE_INLINE typename VertexBuffer::Interface<T>::VertexDeclaration& VertexBuffer::Interface<T>::operator[] (SizeType const i)
	{
		RENGINE_ASSERT(buffer_);
		RENGINE_ASSERT(i < buffer_->size());

		VertexBuffer::DataPointer data_pointer = buffer_->data();
		return *reinterpret_cast<VertexDeclaration*>(data_pointer + buffer_->vertexSize() * i);
	}

	template <typename T>
	RENGINE_INLINE typename VertexBuffer::Interface<T>::VertexDeclaration const& VertexBuffer::Interface<T>::operator[] (SizeType const i) const
	{
		RENGINE_ASSERT(buffer_);
		RENGINE_ASSERT(i < buffer_->size());

		VertexBuffer::ConstDataPointer data_pointer = buffer_->data();
		return *reinterpret_cast<VertexDeclaration const*>(data_pointer + buffer_->vertexSize() * i);
	}


	template <typename T>
	RENGINE_INLINE VertexBuffer::ConstInterface<T>::ConstInterface(VertexBuffer const * const buffer)
	: buffer_(buffer)
	{
	}

	template <typename T>
	RENGINE_INLINE typename VertexBuffer::ConstInterface<T>::VertexDeclaration const& VertexBuffer::ConstInterface<T>::operator[] (SizeType const i) const
	{
		RENGINE_ASSERT(buffer_);
		RENGINE_ASSERT(i < buffer_->size());

		VertexBuffer::ConstDataPointer data_pointer = buffer_->data();
		return *reinterpret_cast<VertexDeclaration const*>(data_pointer + buffer_->vertexSize() * i);
	}


} // end of namespace

#endif // __RENGINE_VERTEX_BUFFER_H__
