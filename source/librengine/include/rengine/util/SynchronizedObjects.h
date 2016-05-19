// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SYNCHRONIZED_OBJECTS_H__
#define __RENGINE_SYNCHRONIZED_OBJECTS_H__

#include <rengine/thread/Synchronization.h>
#include <rengine/lang/Idioms.h>
#include <list>

namespace rengine
{
	//
	// SynchronizedData
	//
	template <typename T>
	class SynchronizedData : private NonCopyable
	{
	public:
		typedef T ValueType;

		SynchronizedData()
		{

		}

		SynchronizedData(ValueType const& initial_value)
		{
			set(initial_value);
		}

		void set(ValueType const& value)
		{
			ScopedLock lock(m_lock);
			m_value = value;
		}

		ValueType get() const
		{
			ScopedLock lock(m_lock);
			return m_value;
		}

		operator ValueType() const
		{
			return get();
		}

		void operator = (ValueType const& value)
		{
			set(value);
		}

	protected:
		Mutex m_lock;
		ValueType m_value;
	};


	//
	// SynchronizedCounter
	//
	template <typename T>
	class SynchronizedCounter : public SynchronizedData<T>
	{
	public:
		typedef SynchronizedData<T> Base;
		typedef typename Base::ValueType ValueType;

		SynchronizedCounter(ValueType const& initial_value = 0)
			:SynchronizedData<T>(initial_value)
		{
		}

		ValueType operator++()
		{
			ScopedLock lock(Base::m_lock);
			return ++Base::m_value;
		}

		ValueType operator++(int)
		{
			ScopedLock lock(Base::m_lock);
			return Base::m_value++;
		}

		ValueType operator--()
		{
			ScopedLock lock(Base::m_lock);
			return --Base::m_value;
		}

		ValueType operator--(int)
		{
			ScopedLock lock(Base::m_lock);
			return Base::m_value--;
		}
	private:
	};


	//
	// Max == 0 no max size
	//	With MaxSize > 0, when size > MaxSize older elements are dropped on push
	//
	template <typename T, typename C = std::list<T> >
	class SynchronizedQueue
	{
	public:
		typedef T ValueType;
		typedef C DataContainer;
		typedef typename DataContainer::size_type SizeType;

		SynchronizedQueue()
		{
			m_maxSize = 0;
			m_drops = 0;
		}

		void push(ValueType const& element)
		{
			ScopedLock lock(m_mutex);
			m_container.insert(m_container.end(), element);

			if (m_maxSize)
			{
				while(Int( m_container.size() ) > m_maxSize)
				{
					m_container.erase(m_container.begin());
					++m_drops;
				}
			}
		}

		//
		// Pops if not empty
		//
		// ReturnValue:
		//	true if pop, false otherwise
		//
		bool tryPop(ValueType& element)
		{
			ScopedLock lock(m_mutex);
			if (!m_container.empty())
			{
				element = *m_container.begin();
				m_container.erase(m_container.begin());
				return true;
			}

			return false;
		}

		bool empty()
		{
			ScopedLock lock(m_mutex);
			return m_container.empty();
		}

		SizeType size()
		{
			ScopedLock lock(m_mutex);
			return m_container.size();
		}

		void setMaxSize(Int const size)
		{
			ScopedLock lock(m_mutex);
			m_maxSize = size;
		}

		Int getMaxSize() const
		{
			ScopedLock lock(m_mutex);
			return m_maxSize;
		}

		Uint64 droppedElements() const
		{
			return m_drops;
		}

		void resetDroppedElements()
		{
			m_drops = 0;
		}
	private:

		DataContainer m_container;
		Mutex m_mutex;
		Int m_maxSize;
		Uint64 m_drops;
	};




}

#endif //__RENGINE_SYNCHRONIZED_OBJECTS_H__
