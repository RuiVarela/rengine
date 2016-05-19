// __!!rengine_copyright!!__ //

#include <rengine/thread/Thread.h>

#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

namespace rengine
{
	struct TlsKey
	{
		TlsKey()
		{
			tls_key = 0;
			initialized = false;
		}

		~TlsKey()
		{
			if (initialized)
			{

			}
		}

		pthread_key_t& get()
		{
			if (!initialized)
			{
			    int status = pthread_key_create(&tls_key, 0);
			    if (status)
			    {
			        printf("Thread::Thread() unable to create tls key [%d]\n", status);
			    }
				initialized = true;
			}

			return tls_key;
		}
	private:
		pthread_key_t tls_key;
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
			pthread_id = 0;
			processor_afinity = -1;
			thread_ended = false;
		}

		volatile Bool thread_ended;
		volatile Bool is_running;
		volatile Bool keep_running;
		volatile Int id;
		volatile Int process_id;

		pthread_t pthread_id;
		int processor_afinity;

		Mutex mutex;
		Block block;
	};


	static void* ThreadFunction(void *data)
	{
		{
			Thread* thread = (Thread*)(data);
			Thread::PrivateImplementation* pimpl = thread->privateImplementation();

			if (pimpl->processor_afinity != -1)
			{
				cpu_set_t cpumask;
				CPU_ZERO(&cpumask);
				CPU_SET(pimpl->processor_afinity, &cpumask);
				if (pthread_setaffinity_np(pthread_self(), sizeof(cpumask), &cpumask) != 0)
				{
					pimpl->processor_afinity = -1;
				}
			}


			int status = pthread_setspecific(tls_key.get(), thread);
			if (status)
			{
				printf("Thread::ThreadFunction unable to pthread_setspecific [%d]\n", status);
			}


			pimpl->keep_running = true;

			thread->preRun();

			pimpl->is_running = true;

			pimpl->block.release();

			thread->run();

			pimpl->thread_ended = true;

			pthread_setspecific(tls_key.get(), 0);
		}

		return 0;
	}


	Thread::Thread()
		:m_pimpl(new PrivateImplementation())
	{
		m_pimpl->process_id = Int( getpid() );
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
		if (cpu < 0)
		{
			return;
		}

	    if (m_pimpl->is_running && Thread::currentThread() == this)
	    {
	        cpu_set_t cpumask;
	        CPU_ZERO(&cpumask);
	        CPU_SET(m_pimpl->processor_afinity, &cpumask);

	        if (pthread_setaffinity_np(pthread_self(), sizeof(cpumask), &cpumask) != 0)
	        {
	        	m_pimpl->processor_afinity = -1;
	        }
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
		pthread_create(&(m_pimpl->pthread_id), 0, ThreadFunction, (void*)this);


		m_pimpl->block.block();

	}

	void Thread::stop()
	{
		ScopedLock lock(m_pimpl->mutex);

		//if (m_pimpl->is_running)
		if (m_pimpl->pthread_id != 0)
		{
			m_pimpl->keep_running = false;

			 void *result = 0;
			 int status = pthread_join(m_pimpl->pthread_id, &result);

			 if (status == EDEADLK)
			 {
				 printf("Thread::stop %p has deadlock state", (void*)this);
			 }

			 m_pimpl->pthread_id = 0;
			 m_pimpl->is_running = false;
		}
	}

	void Thread::signalShouldStop()
	{
		m_pimpl->keep_running = false;
	}

	Int Thread::microSleep(Uint const& microseconds)
	{
	    return ::usleep(microseconds);
	}

	Int Thread::numberOfProcessors()
	{
		Int64 value = sysconf(_SC_NPROCESSORS_ONLN);
		if (value == -1)
		{
			return 0;
		}
		return value;
	}

	Thread* Thread::currentThread()
	{
	    return (Thread*)(pthread_getspecific(tls_key.get()));
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
