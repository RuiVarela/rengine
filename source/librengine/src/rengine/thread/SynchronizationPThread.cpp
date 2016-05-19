// __!!rengine_copyright!!__ //

#include <rengine/thread/Synchronization.h>

#include <string>
#include <pthread.h>
#include <unistd.h>
#include <cstdio>
#include <cerrno>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#define RENGINE_PREVENT_PRIORITY_INVERSION

std::string getError(int status)
{
	if      (status == EBUSY) { return "EBUSY"; }
	else if (status == EINVAL) { return "EINVAL"; }
	else if (status == EAGAIN) { return "EAGAIN"; }
	else if (status == ENOMEM) { return "ENOMEM"; }
	else if (status == EPERM) { return "EPERM"; }

	return "";
}

#define CHECK_PTHREAD_ERR(status) if (status != 0) { printf("Synchronization> Unexpected error [code=%d, message=%s] at [%d, %s]\n", status, getError(status).c_str(), __LINE__, __FILE__ ); }

namespace rengine
{
	struct MutexImplementation
	{
		pthread_mutex_t mutex;
	};

	struct ConditionImplementation
	{
	    pthread_cond_t condition;
	};

	//
	// Mutex
	//
	Mutex::Mutex()
	{
		int status = 0;

		m_pimpl = new MutexImplementation();

	    pthread_mutexattr_t mutex_attribute;
	    status = pthread_mutexattr_init(&mutex_attribute);
	    CHECK_PTHREAD_ERR(status)

	    status = pthread_mutexattr_settype(&mutex_attribute, PTHREAD_MUTEX_RECURSIVE);
	    CHECK_PTHREAD_ERR(status)


#ifdef RENGINE_PREVENT_PRIORITY_INVERSION

	    //
    	//Prevent priority inversion
    	//
	    if (sysconf(_POSIX_THREAD_PRIO_INHERIT))
		{
	        // When a thread is blocking higher priority threads because of owning one
	        // or more mutexes with the PTHREAD_PRIO_INHERIT  protocol  attribute,  it
	        // shall  execute  at  the  higher  of its priority or the priority of the
	        // highest priority thread waiting on any of the  mutexes  owned  by  this
	        // thread and initialized with this protocol.

	    	status = pthread_mutexattr_setprotocol(&mutex_attribute, PTHREAD_PRIO_INHERIT);
		    CHECK_PTHREAD_ERR(status)
		}
		else if (sysconf(_POSIX_THREAD_PRIO_PROTECT))
		{
			//  When   a   thread  owns  one  or  more  mutexes  initialized  with  the
		    //  PTHREAD_PRIO_PROTECT protocol, it shall execute at the  higher  of  its
		    //  priority  or  the  highest  of the priority ceilings of all the mutexes
		    //  owned by this thread and initialized with this attribute, regardless of
		    //  whether other threads are blocked on any of these mutexes or not.

			int th_policy;
			struct sched_param th_param;
			status = pthread_getschedparam(pthread_self(), &th_policy, &th_param);
		    CHECK_PTHREAD_ERR(status)

			status = pthread_mutexattr_setprotocol(&mutex_attribute, PTHREAD_PRIO_PROTECT);
		    CHECK_PTHREAD_ERR(status)

		    status = pthread_mutexattr_setprioceiling(&mutex_attribute, sched_get_priority_max(th_policy));
		    CHECK_PTHREAD_ERR(status)

		}
#endif //RENGINE_PREVENT_PRIORITY_INVERSION

	    status = pthread_mutex_init(&m_pimpl->mutex, &mutex_attribute);
	    CHECK_PTHREAD_ERR(status)
	}

	Mutex::~Mutex()
	{
		int status = 0;

		status = pthread_mutex_destroy(&m_pimpl->mutex);
	    CHECK_PTHREAD_ERR(status)

		delete(m_pimpl);
	}

	Bool Mutex::lock()
	{
	    return (pthread_mutex_lock(&m_pimpl->mutex) == 0);
	}

	Bool Mutex::unlock()
	{
	    return (pthread_mutex_unlock(&m_pimpl->mutex) == 0);
	}

	Bool Mutex::tryLock()
	{
	    return (pthread_mutex_trylock(&m_pimpl->mutex) == 0);
	}


	//
	// Condition
	//

	// unlock on cancel
	void conditionCleanup(void *m)
	{
	    pthread_mutex_t *mutex = (pthread_mutex_t*)(m);
	    pthread_mutex_unlock(mutex);
	}

	Condition::Condition()
	:m_pimpl(new ConditionImplementation())
	{
		int status = 0;
		status = pthread_cond_init(&m_pimpl->condition, 0);
	    CHECK_PTHREAD_ERR(status)
	}

	Condition::~Condition()
	{
		int status = 0;
		status = pthread_cond_destroy(&m_pimpl->condition);
	    CHECK_PTHREAD_ERR(status)

		delete(m_pimpl);
	}

	Bool Condition::wait(Mutex* mutex)
	{
		int status = 0;

		pthread_cleanup_push(conditionCleanup, &mutex->m_pimpl->mutex);
		status = pthread_cond_wait(&m_pimpl->condition, &mutex->m_pimpl->mutex);
		pthread_cleanup_pop(0);

		return (status == 0);
	}

    Bool Condition::wait(Mutex* mutex, Uint const milliseconds)
    {
        struct ::timeval now;
        ::gettimeofday(&now, 0);

        struct timespec targettime;
        targettime.tv_sec = milliseconds / 1000;
        targettime.tv_nsec = (milliseconds % 1000) * 1000000;

        targettime.tv_sec += now.tv_sec;
        targettime.tv_nsec += now.tv_usec * 1000;

        targettime.tv_sec += targettime.tv_nsec / 1000000000;
        targettime.tv_sec  = targettime.tv_sec  % 1000000000;


		int status = 0;

        pthread_cleanup_push(conditionCleanup, &mutex->m_pimpl->mutex);
        status = pthread_cond_timedwait(&m_pimpl->condition, &mutex->m_pimpl->mutex, &targettime);
        pthread_cleanup_pop(0);

		return (status == 0);
    }

    Bool Condition::signal()
    {
        return (pthread_cond_signal(&m_pimpl->condition) == 0);
    }

    Bool Condition::broadcast()
    {
        return (pthread_cond_broadcast(&m_pimpl->condition) == 0);
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
		return __sync_lock_test_and_set(&m_value, value);
	}

	Atomic::AtomicValue Atomic::operator +=(AtomicValue const& value)
	{
		return __sync_add_and_fetch(&m_value, value);
	}

	Atomic::AtomicValue Atomic::operator -=(AtomicValue const& value)
	{
		return __sync_sub_and_fetch(&m_value, value);
	}

	Atomic::AtomicValue Atomic::operator ++()
	{
		return __sync_add_and_fetch(&m_value, 1);
	}

	Atomic::AtomicValue Atomic::operator --()
	{
		return __sync_sub_and_fetch(&m_value, 1);
	}

	Atomic::AtomicValue Atomic::operator ++(int)
	{
		return __sync_fetch_and_add(&m_value, 1);
	}

	Atomic::AtomicValue Atomic::operator --(int)
	{
		return __sync_fetch_and_sub(&m_value, 1);
	}

	Atomic::AtomicValue Atomic::operator |=(AtomicValue const& value)
	{
	    return __sync_or_and_fetch(&m_value, value);
	}

	Atomic::AtomicValue Atomic::operator &=(AtomicValue const& value)
	{
	    return __sync_and_and_fetch(&m_value, value);
	}

	Atomic::AtomicValue Atomic::operator ^=(AtomicValue const& value)
	{
	    return __sync_xor_and_fetch(&m_value, value);
	}


} // end namespace
