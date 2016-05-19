// __!!rengine_copyright!!__ //

#include <rengine/thread/Thread.h>
#include <rengine/thread/Synchronization.h>

#include <windows.h>
#include <process.h>
#include <cstdio>

namespace rengine
{
	struct TlsKey
	{
		TlsKey()
		{
			tls_key = 0;
			initialized = 0;
		}	
		
		~TlsKey()
		{
			if (initialized)
			{
				TlsFree(tls_key);
			}
		}

		DWORD get()
		{
			if (!initialized) 
			{
				tls_key = TlsAlloc();
				initialized = true;
			}
			return tls_key;
		}
	private:
		DWORD tls_key;
		bool initialized;
	};

	static int threadCounter = 0;
	static TlsKey tls_key;

	struct Thread::PrivateImplementation
	{
		PrivateImplementation()
		{
			keep_running = false;
			is_running = false;
			id = ++threadCounter;
			process_id = -1;
			thread_id = -1;
			processor_afinity = -1;
			thread_ended = false;

			thread_handle = 0;
		}

		volatile Bool thread_ended;
		volatile Bool is_running;
		volatile Bool keep_running;
		Int id;
		Int process_id;
		HANDLE thread_handle;
		unsigned int thread_id;

		int processor_afinity;

		Mutex mutex;
		Block block;
	};

	static unsigned __stdcall ThreadFunction(void* arguments)
	{
		{
			Thread* thread = (Thread*)(arguments);
			Thread::PrivateImplementation* pimpl = thread->privateImplementation();

			TlsSetValue(tls_key.get(), thread);

			if (pimpl->processor_afinity != -1)
			{
				DWORD mask  = 0x1 << pimpl->processor_afinity ; // thread affinity mask
				DWORD_PTR result = SetThreadAffinityMask(pimpl->thread_handle, mask);

				if (result == 0)
				{
					pimpl->processor_afinity = -1;
				}
			}


			pimpl->keep_running = true;

			thread->preRun();

			pimpl->is_running = true;

			pimpl->block.release();

			thread->run();

			pimpl->thread_ended = true;

			TlsSetValue(tls_key.get(), 0);
		}

		_endthreadex( 0 );
		return 0;
	} 


	Thread::Thread()
		:m_pimpl(new PrivateImplementation())
	{
		m_pimpl->process_id = Int( GetCurrentProcessId() );
	}

	Thread::~Thread()
	{
	    if(m_pimpl->is_running)
	    {
	    	stop();
	    }

		delete(m_pimpl);
		m_pimpl = 0;
	}

	Bool Thread::isRunning()
	{
		return m_pimpl->is_running && !m_pimpl->thread_ended;
	}

	Bool Thread::keepRunning()
	{
		return m_pimpl->keep_running;
	}

	void Thread::setProcessorAfinity(Int cpu)
	{
		m_pimpl->processor_afinity = cpu;
		if ((cpu < 0) || 
			(m_pimpl->thread_handle == 0) ||  
			(m_pimpl->thread_handle == INVALID_HANDLE_VALUE) )
		{
			return;
		}

		DWORD mask  = 0x1 << m_pimpl->processor_afinity ; // thread affinity mask
		DWORD_PTR result = SetThreadAffinityMask(m_pimpl->thread_handle, mask);
	
		if (result == 0)
		{
			m_pimpl->processor_afinity = -1;
		}
	}

	Int Thread::getProcessorAffinity() const
	{
		return m_pimpl->processor_afinity;
	}

	void Thread::start()
	{
		ScopedLock lock(m_pimpl->mutex);

		stop();

		m_pimpl->block.reset();
		m_pimpl->thread_ended = false;
		m_pimpl->thread_handle = (HANDLE)_beginthreadex(NULL, 0, &ThreadFunction, this, 0, &m_pimpl->thread_id);

		if ((m_pimpl->thread_handle == 0) || (m_pimpl->thread_handle == INVALID_HANDLE_VALUE))
		{
			printf("Thread::start unable to start thread");
			m_pimpl->thread_handle = 0;
		}
		else
		{
			m_pimpl->block.block();
		}
	}

	void Thread::stop()
	{
		ScopedLock lock(m_pimpl->mutex);

		if ((m_pimpl->thread_handle != 0) && 
			(m_pimpl->thread_handle != INVALID_HANDLE_VALUE))
		{
			m_pimpl->keep_running = false;

			if (WaitForSingleObject(m_pimpl->thread_handle, INFINITE) != WAIT_OBJECT_0)
			{
				printf("Thread::stop %p WaitForSingleObject failed", (void*)this);
			}

			CloseHandle(m_pimpl->thread_handle);

			m_pimpl->thread_handle = 0;
			m_pimpl->is_running = false;
		}
	}

	void Thread::signalShouldStop()
	{
		m_pimpl->keep_running = false;
	}

	Int Thread::microSleep(Uint const& microseconds)
	{
		HANDLE sleeper = CreateWaitableTimer(NULL, TRUE, NULL);

		if(!sleeper)
		{
			CloseHandle(sleeper);
			return -1;
		}

		LARGE_INTEGER t;
		t.QuadPart= -(LONGLONG)microseconds * 10; // in 100ns units
												  // negative sign means relative,

		if (!SetWaitableTimer(sleeper, &t, 0, NULL, NULL, 0))
		{
			CloseHandle(sleeper);
			return -1;
		}

		// Wait for the timer.
		if (WaitForSingleObject(sleeper, INFINITE) != WAIT_OBJECT_0)
		{
			CloseHandle(sleeper);
			return -1;
		}

		CloseHandle(sleeper);
		return 0;
	}

	Int Thread::numberOfProcessors()
	{
		SYSTEM_INFO info;
		GetSystemInfo(&info);
		return Int(info.dwNumberOfProcessors);
	}

	Thread* Thread::currentThread()
	{
		return (Thread*) TlsGetValue(tls_key.get());
	}

	Int Thread::getThreadId() const
	{
		return m_pimpl->id;
	}

	Int Thread::getProccessId() const
	{
		return m_pimpl->process_id;
	}


} // end of namespace
