// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SHARED_POINTER__
#define __RENGINE_SHARED_POINTER__

#include <rengine/lang/Types.h>
#include <rengine/lang/Platform.h>
#include <rengine/thread/Synchronization.h>

namespace rengine
{
	//
	// SharedPointer
	//
	struct PointerReferenceCounter
	{
		typedef Atomic CounterType;

		PointerReferenceCounter() :reference_count(0) {}
		~PointerReferenceCounter() {}

		void reference()
		{
			reference_count++;
		}

		void unreference(bool& do_delete)
		{
			do_delete = (--reference_count == 0);
		}

		Atomic reference_count;
	};

	template <typename T>
	class SharedPointer
	{
	public:
		typedef SharedPointer<T> ThisType;
		typedef T Type;
		typedef Type* PointerType;
		typedef Type const* ConstPointerType;
		typedef Type& ReferenceType;
		typedef Type const& ConstReferenceType;
		typedef Uint CounterType;

		//
		// Constructors
		//
		SharedPointer(PointerType data_pointer = 0)
		{
			reference_counter = new PointerReferenceCounter();
            reference(data_pointer);
		}

		SharedPointer(SharedPointer const& shared_pointer)
		{
			reference_counter = shared_pointer.reference_counter;
		    reference(shared_pointer.pointer);
		}

		~SharedPointer()
		{
			unreference();
		}

		ReferenceType operator*() const { return *pointer; }
        PointerType operator->() const { return pointer; }
        PointerType get() const { return pointer; }
		Bool operator!() const { return pointer == 0; }

		Bool operator == (SharedPointer const& shared_pointer) const { return (pointer == shared_pointer.pointer); }
        Bool operator == (ConstPointerType data_pointer) const { return (pointer == data_pointer); }
		friend Bool operator == (ConstPointerType data_pointer, SharedPointer const& shared_pointer) { return (shared_pointer == data_pointer); }

		Bool operator != (SharedPointer const& shared_pointer) const { return (pointer != shared_pointer.pointer); }
        Bool operator != (ConstPointerType data_pointer) const { return (pointer != data_pointer); }
		friend Bool operator != (ConstPointerType data_pointer, SharedPointer const& shared_pointer) { return (shared_pointer != data_pointer); }

		// safe bool idiom
		typedef void (ThisType::*bool_type)() const;
		void safe_bool_function() const {}
		operator bool_type() const { return pointer == 0 ? 0 : &ThisType::safe_bool_function; }

		void swap(SharedPointer& shared_pointer)
		{
			PointerReferenceCounter* temporary_reference_counter = reference_counter;
			reference_counter = shared_pointer.reference_counter;
			shared_pointer.reference_counter = temporary_reference_counter;

			PointerType temporary_pointer = pointer;
			pointer = shared_pointer.pointer;
			shared_pointer.pointer = temporary_pointer;
		}

	    void reset(PointerType data_pointer = 0) { ThisType(data_pointer).swap(*this); }

		SharedPointer& operator = (SharedPointer const& shared_pointer) { ThisType(shared_pointer).swap(*this); return *this; }
        SharedPointer& operator = (PointerType data_pointer) { ThisType(data_pointer).swap(*this); return *this; }


		Int64 referenceCount() const { return Int64(reference_counter->reference_count); }

		//
		// Inheritance Support
		//

		template<class ConversionType>
		operator SharedPointer<ConversionType>() // implicit conversion operators.
		{
			return SharedPointer<ConversionType>(pointer, reference_counter);
		}

		//
		// This should never be called explicitly
		//
		SharedPointer(PointerType data_pointer, PointerReferenceCounter* counter)
		{
			reference_counter = counter;
			reference(data_pointer);
		}

		PointerReferenceCounter* referenceCounter() const { return reference_counter; }

	private:
		PointerReferenceCounter* reference_counter;
		PointerType pointer;

		void reference(PointerType data_pointer)
		{
            reference_counter->reference();
            pointer = data_pointer;
		}

		void unreference()
		{
			bool do_delete = false;
            reference_counter->unreference(do_delete);

            if (do_delete)
            {
            	if (pointer)
            	{
    				delete(pointer);
    				pointer = 0;
            	}

    			delete(reference_counter);
    			reference_counter = 0;
            }
		}
	};


	template<class T>
	RENGINE_INLINE void swap(SharedPointer<T> &shared_pointer, SharedPointer<T> &another_shared_pointer)
	{
		shared_pointer.swap(another_shared_pointer);
	}

	template<class T>
	RENGINE_INLINE T* get_pointer(SharedPointer<T> const& shared_pointer)
	{
		return shared_pointer.get();
	}

	template<class T, class Y>
	RENGINE_INLINE SharedPointer<T> const& static_pointer_cast(SharedPointer<Y> const& shared_pointer)
	{
		return SharedPointer<T>( static_cast<T*>(shared_pointer.get()), shared_pointer.referenceCounter());
	}

	template<class T, class Y>
	RENGINE_INLINE SharedPointer<T> dynamic_pointer_cast(SharedPointer<Y> const& shared_pointer)
	{
		T* pointer = dynamic_cast<T*>(shared_pointer.get());
		return (pointer ? SharedPointer<T>(pointer, shared_pointer.referenceCounter()) : SharedPointer<T>() );
	}

	template<class T, class Y>
	RENGINE_INLINE SharedPointer<T> const_pointer_cast(SharedPointer<Y> const& shared_pointer)
	{
		return SharedPointer<T>( const_cast<T*>(shared_pointer.get()), shared_pointer.referenceCounter());
	}

	//
	// Shared Array
	//

	template <typename T>
	class SharedArray
	{
	public:
		typedef SharedArray<T> ThisType;
		typedef T Type;
		typedef Type* PointerType;
		typedef Type const* ConstPointerType;
		typedef Type& ReferenceType;
		typedef Type const& ConstReferenceType;

		SharedArray(PointerType data_pointer = 0){ m_array.reset( new Holder(data_pointer) ); }
		SharedArray(SharedArray const& shared_array) { m_array = shared_array.m_array; }
		~SharedArray() {}

		ReferenceType operator[](PointerDiff i) const { return get()[i]; }

        PointerType get() const { return (m_array ? m_array->holder : 0); }
		Bool operator!() const { return get() == 0; }

		Bool operator == (SharedArray const& shared) const { return (m_array == shared.m_array); }
		Bool operator == (ConstPointerType pointer) const { return (get() == pointer); }
		friend Bool operator == (ConstPointerType data_pointer, SharedArray const& shared) { return (shared == data_pointer); }

		Bool operator != (SharedArray const& shared) const { return (m_array != shared.m_array); }
        Bool operator != (ConstPointerType data_pointer) const { return (get() != data_pointer); }
		friend Bool operator != (ConstPointerType data_pointer, SharedArray const& shared) { return (shared != data_pointer); }

		// safe bool idiom
		typedef void (ThisType::*bool_type)() const;
		void safe_bool_function() const {}
		operator bool_type() const { return (get() == 0) ? 0 : &ThisType::safe_bool_function; }

		void swap(SharedArray& shared) { SharedPointerType(shared.m_array).swap(m_array); }

	    void reset(PointerType data_pointer = 0) { m_array.reset( new Holder(data_pointer) ); }

		SharedArray& operator = (SharedArray const& shared) { SharedPointerType(shared.m_array).swap(m_array); return *this; }
        SharedArray& operator = (PointerType data_pointer) { reset(data_pointer); return *this; }

	private:
		struct Holder
		{
			Holder(PointerType object)	{ holder = object;}
			~Holder()	{ if (holder) { delete[](holder); } }
			PointerType holder;
		};

		typedef SharedPointer<Holder> SharedPointerType;
		SharedPointerType m_array;
	};

} //namespace rengine


#endif //__RENGINE_SHARED_POINTER__
