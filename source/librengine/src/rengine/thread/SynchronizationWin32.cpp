// __!!rengine_copyright!!__ //

#include <rengine/thread/Synchronization.h>

#include <windows.h>
#include <cstdio>

#define CHECK_HANDLE(handle) if (!handle || (handle == INVALID_HANDLE_VALUE)) { printf("Invalid Handle at [%d, %s]\n", __LINE__, __FILE__ ); }

namespace rengine
{
	struct MutexImplementation
	{
		CRITICAL_SECTION critical_section;
	};

	struct ConditionImplementation
	{
		// Semaphore used to queue up threads waiting for the condition 
		// to become signaled. 
		HANDLE semaphore;

		// Number of waiting threads.
		unsigned int waiters_count;
		
		 // Serialize access to <waiters_count>.
		CRITICAL_SECTION waiters_count_lock;

		// An auto-reset event used by the broadcast/signal thread to wait
		// for all the waiting thread(s) to wake up and be released from the
		// semaphore. 
		HANDLE waiters_done;

		// Keeps track of whether we were broadcasting or signaling.  This
		// allows us to optimize the code if we're just signaling.
		size_t was_broadcast;
	};

	struct BarrierImplementation
	{
		Condition condition;
	};

	//
	// Mutex
	//
	Mutex::Mutex()
	{
		m_pimpl = new MutexImplementation();
		InitializeCriticalSection(&m_pimpl->critical_section);
	}

	Mutex::~Mutex()
	{
		DeleteCriticalSection(&m_pimpl->critical_section);
		delete(m_pimpl);
	}

	Bool Mutex::lock()
	{
		EnterCriticalSection(&m_pimpl->critical_section);
	    return true;
	}

	Bool Mutex::unlock()
	{
		LeaveCriticalSection(&m_pimpl->critical_section);
	    return true;
	}

	Bool Mutex::tryLock()
	{
		 return (TryEnterCriticalSection(&m_pimpl->critical_section) != 0);
	}


	//
	// Condition
	//
	Condition::Condition()
		:m_pimpl(new ConditionImplementation())
	{
		m_pimpl->waiters_count = 0;
		m_pimpl->was_broadcast = 0;
		m_pimpl->semaphore = CreateSemaphore(NULL,       // no security
											 0,          // initially 0
											 0x7fffffff, // max count
											 NULL);      // unnamed 
		CHECK_HANDLE(m_pimpl->semaphore)

		InitializeCriticalSection(&m_pimpl->waiters_count_lock);
		m_pimpl->waiters_done = CreateEvent(NULL,  // no security
											FALSE, // auto-reset
											FALSE, // non-signaled initially
											NULL); // unnamed
		CHECK_HANDLE(m_pimpl->waiters_done)
	}

	Condition::~Condition()
	{
		DeleteCriticalSection(&m_pimpl->waiters_count_lock);
		CloseHandle(m_pimpl->waiters_done);
		CloseHandle(m_pimpl->semaphore);
		delete(m_pimpl);
	}

	Bool Condition::wait(Mutex* mutex)
	{
		return wait(mutex, INFINITE);
	}

    Bool Condition::wait(Mutex* mutex, Uint const milliseconds)
    {
		EnterCriticalSection(&m_pimpl->waiters_count_lock);
		m_pimpl->waiters_count++;
		LeaveCriticalSection(&m_pimpl->waiters_count_lock);


		ReversedScopedLock lock(*mutex);
		DWORD result = WaitForSingleObject(m_pimpl->semaphore, milliseconds);


		EnterCriticalSection(&m_pimpl->waiters_count_lock);
		m_pimpl->waiters_count--;
		// Check to see if we're the last waiter after <_broadcast>.
		int last_waiter = m_pimpl->was_broadcast && (m_pimpl->waiters_count == 0);
		LeaveCriticalSection(&m_pimpl->waiters_count_lock);


		// If we're the last waiter thread during this particular broadcast then let all the other threads proceed.
		if (last_waiter && (result != -1))
		{
			SetEvent(m_pimpl->waiters_done);
		}

		return (result == WAIT_OBJECT_0);
    }

    Bool Condition::signal()
    {
		EnterCriticalSection(&m_pimpl->waiters_count_lock);
		int have_waiters = (m_pimpl->waiters_count > 0);
		LeaveCriticalSection(&m_pimpl->waiters_count_lock);

		BOOL ok = TRUE;

		// If there aren't any waiters, then this is a no-op.  
		if (have_waiters)
		{
			ok = ReleaseSemaphore(m_pimpl->semaphore, 1, 0);
		}

        return (ok == TRUE);
    }

    Bool Condition::broadcast()
    {
		// This is needed to ensure that <waiters_count> and <was_broadcast> are consistent relative to each other.
		EnterCriticalSection(&m_pimpl->waiters_count_lock);
		int have_waiters = 0;

		if (m_pimpl->waiters_count > 0)
		{
			// We are broadcasting, even if there is just one waiter...
			// Record that we are broadcasting, which helps optimize
			// <pthread_cond_wait> for the non-broadcast case.
			m_pimpl->was_broadcast = 1;
			have_waiters = 1;
		}

		if (have_waiters) 
		{
			// Wake up all the waiters atomically.
			ReleaseSemaphore(m_pimpl->semaphore, m_pimpl->waiters_count, 0);
			LeaveCriticalSection(&m_pimpl->waiters_count_lock);


			// Wait for all the awakened threads to acquire the counting semaphore. 
			WaitForSingleObject(m_pimpl->waiters_done, INFINITE);

			// This assignment is okay, even without the <waiters_count_lock_> held  because no other waiter threads can wake up to access it.
			m_pimpl->was_broadcast = 0;
		}
		else
		{
			LeaveCriticalSection(&m_pimpl->waiters_count_lock);
		}


        return true;
    }


	//
	// Atomic
	//
    Atomic::Atomic(AtomicValue value)
    	:m_value(value)
    {}

    Atomic::~Atomic()
	{}

	Atomic::AtomicValue Atomic::exchange(AtomicValue const& value)
	{
		return InterlockedExchange64(&m_value, value);
	}

	Atomic::AtomicValue Atomic::operator +=(AtomicValue const& value)
	{
		return InterlockedExchangeAdd64(&m_value, value) + value;
	}

	Atomic::AtomicValue Atomic::operator -=(AtomicValue const& value)
	{
		return InterlockedExchangeAdd64(&m_value, -value) - value;
	}

	Atomic::AtomicValue Atomic::operator ++()
	{
		return InterlockedIncrement64(&m_value);
	}

	Atomic::AtomicValue Atomic::operator --()
	{
		return InterlockedDecrement64(&m_value);
	}

	Atomic::AtomicValue Atomic::operator ++(int)
	{
		return InterlockedIncrement64(&m_value) - 1;
	}

	Atomic::AtomicValue Atomic::operator --(int)
	{
		return InterlockedDecrement64(&m_value) + 1;
	}

	Atomic::AtomicValue Atomic::operator |=(AtomicValue const& value)
	{
		return InterlockedOr64(&m_value, value) | value;
	}

	Atomic::AtomicValue Atomic::operator &=(AtomicValue const& value)
	{
		return InterlockedAnd64(&m_value, value) & value;
	}

	Atomic::AtomicValue Atomic::operator ^=(AtomicValue const& value)
	{
		return InterlockedXor64(&m_value, value) ^ value;
	}
} // end namespace
