// __!!rengine_copyright!!__ //

#include <rengine/thread/Synchronization.h>

namespace rengine
{

	//
	// ReadWriteMutex
	//
	ReadWriteMutex::ReadWriteMutex()
		:m_readers(0),
		 m_writers(0),
		 m_read_waiters(0),
		 m_write_waiters(0)
	{}

	ReadWriteMutex::~ReadWriteMutex()
	{}

	Bool ReadWriteMutex::readLock()
	{
		ScopedLock lock(m_mutex);

	    if ((m_writers > 0) || (m_write_waiters > 0))
	    {
	    	m_read_waiters++;

	    	while((m_writers > 0) || (m_write_waiters > 0))
	    	{
	    		m_read.wait(&m_mutex);
	    	}

	    	m_read_waiters--;
	    }

	    m_readers++;

        return true;
	}

	Bool ReadWriteMutex::readUnlock()
	{
		ScopedLock lock(m_mutex);

		if (m_readers > 0)
		{
			m_readers--;
		}

		m_write.signal();
        return true;
	}

	Bool ReadWriteMutex::writeLock()
	{
		ScopedLock lock(m_mutex);

	    if ((m_readers > 0) || (m_writers > 0))
	    {
	    	m_write_waiters++;

	    	while((m_readers > 0) || (m_writers > 0))
	    	{
	    		m_write.wait(&m_mutex);
	    	}

	        m_write_waiters--;
	    }

	    m_writers = 1;


        return true;
	}

	Bool ReadWriteMutex::writeUnlock()
	{
		ScopedLock lock(m_mutex);

		m_writers = 0;
	    if (m_write_waiters > 0)
	    {
	    	m_write.signal();
	    }
	    else if (m_read_waiters)
	    {
	    	m_read.broadcast();
	    }

        return true;
	}

	//
	// Block
	//
	Block::Block()
		:m_released(false)
	{}

	Block::~Block()
	{
		release();
	}

	Bool Block::block()
	{
		ScopedLock lock(m_mutex);
		if (!m_released)
		{
			return m_condition.wait(&m_mutex);
		}
		return true;
	}

	Bool Block::block(Uint const milliseconds)
	{
		ScopedLock lock(m_mutex);
		if (!m_released)
		{
			return m_condition.wait(&m_mutex, milliseconds);
		}
		return true;
	}

	void Block::release()
	{
		ScopedLock lock(m_mutex);
		if (!m_released)
		{
			m_released = true;
			m_condition.broadcast();
		}
	}

	void Block::reset()
	{
		ScopedLock lock(m_mutex);
		m_released = false;
	}


    //
    // Barrier
    //
    Barrier::Barrier(Uint const threads)
    {
		m_threads_to_block = threads;
    	m_count = 0;
    	m_round = 0;
    	m_valid = true;
    }

    Barrier::~Barrier()
    {
		{
			ScopedLock lock(m_mutex);
			m_valid = false;
		}
    	release();
    }

	void Barrier::block()
	{
		m_mutex.lock();

		if (m_valid)
		{
			Uint current_round = m_round;
			++m_count;

			if (m_count == m_threads_to_block)
			{
				m_count = 0;
				++m_round;

				m_condition.broadcast();
			}

			while ((current_round == m_round) && (m_valid))
			{
				m_condition.wait(&m_mutex);
			}
		}

		m_mutex.unlock();
	}

	void Barrier::release()
	{
		ScopedLock lock(m_mutex);

		m_count = 0;
		++m_round;
		m_condition.broadcast();
	}

	void Barrier::reset()
	{
		ScopedLock lock(m_mutex);

		m_count = 0;
		m_round = 0;
	}

} // end of namespace
