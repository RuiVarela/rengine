// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SYNCHRONIZATION_H__
#define __RENGINE_SYNCHRONIZATION_H__

#include <rengine/lang/Types.h>

namespace rengine
{
	struct MutexImplementation;
	struct ConditionImplementation;
	class Condition;

	//
	// Recursive Mutex
	//
	// A recursive mutex can be locked repeatedly by the owner.
	// The mutex doesn't become unlocked until the owner has called unlock() for each successful
	// lock request that it has outstanding on the mutex.
	//
	class Mutex
	{
	public:
		Mutex();
		~Mutex();

		Bool lock();
		Bool unlock();
		Bool tryLock();

	private:
		Mutex(Mutex const& non_copyable) {}
		Mutex& operator=(Mutex const& non_copyable) { return *this; }
		
		MutexImplementation* m_pimpl;

		friend class Condition;
	};

	//
	// Condition
	//
	// Condition variables provide yet another way for threads to synchronize.
	// While mutexes implement synchronization by controlling thread access to data,
	// condition variables allow threads to synchronize based upon the actual value of data
	//
	class Condition
	{
	public:
		Condition();
		~Condition();

		Bool wait(Mutex* mutex);
	    Bool wait(Mutex* mutex, Uint const milliseconds);

	    // is used to signal (or wake up) another thread which is waiting on the condition variable.
	    Bool signal();

	    //should be used instead of signal() if more than one thread is in a blocking wait state
	    Bool broadcast();
	private:
		Condition(Condition const& non_copyable) {}
		Condition& operator=(Condition const& non_copyable) { return *this; }
		ConditionImplementation* m_pimpl;
	};

	//
	// Atomic
	//
	class Atomic
	{
	public:
		typedef Int64 AtomicValue;

		Atomic(AtomicValue value = 0);
		~Atomic();

		AtomicValue exchange(AtomicValue const& value);
		AtomicValue operator =(AtomicValue const& value) { exchange(value); return value; }

		AtomicValue operator +=(AtomicValue const& value);
		AtomicValue operator -=(AtomicValue const& value);

		AtomicValue operator ++();
		AtomicValue operator --();
		AtomicValue operator ++(int);
		AtomicValue operator --(int);

		AtomicValue operator |=(AtomicValue const& value);
		AtomicValue operator &=(AtomicValue const& value);
		AtomicValue operator ^=(AtomicValue const& value);


		AtomicValue value() { return m_value; }
		operator Int64() const { return m_value; }
	private:
		Atomic(Atomic const& non_copyable) {}
		AtomicValue m_value;
	};

	//
	// ReadWriteMutex
	//
	// Support for multiple reads
	// Blocks on the following situations:
	//	- Multiple writes
	//	- Write when someone is reading
	//	- Read when someone is writing
	//
	class ReadWriteMutex
	{
	public:
		ReadWriteMutex();
		~ReadWriteMutex();

		Bool readLock();
		Bool readUnlock();

		Bool writeLock();
		Bool writeUnlock();
	private:
		ReadWriteMutex(ReadWriteMutex const& non_copyable) {}
		ReadWriteMutex& operator=(ReadWriteMutex const& non_copyable) { return *this; }

		Mutex m_mutex;
		Condition m_read;
		Condition m_write;
		Int m_readers;
		Int m_writers;
		Int m_read_waiters;
		Int m_write_waiters;
	};


	//
	// Block
	//
	class Block
	{
	public:
		Block();
		~Block();

		Bool block();
		Bool block(Uint const milliseconds);
		void release();
		void reset();
	private:
		Block(Block const& non_copyable) {}
		Block& operator=(Block const &non_copyable) { return *this; }

		Mutex m_mutex;
		Condition m_condition;
		bool m_released;
	};

	//
	// Barrier
	//
	// You should guarantee the barrier has no blocked threads, before deleting this object
	//
	class Barrier
	{
	public:
		Barrier(Uint const threads); //number of threads to block
		~Barrier();

		void block(); // blocks until all threads called called this method
		void release();
		void reset();
	private:
		Barrier() {}
		Barrier(Barrier const& non_copyable) {}
		Barrier& operator=(Barrier const& non_copyable) { return *this; }

		Uint m_threads_to_block;
		Condition m_condition;
		Mutex m_mutex;
		volatile Uint m_count;
		volatile Uint m_round;
		volatile Bool m_valid;
	};

	//
	// Scoped Lock
	//
	class ScopedLock
	{
	    public:
	        explicit ScopedLock(Mutex& locker) :m_locker(locker) { m_locker.lock(); }
	        ~ScopedLock(){ m_locker.unlock(); }
	    private:
			ScopedLock(ScopedLock const& non_copyable) :m_locker(non_copyable.m_locker) {}
	        ScopedLock& operator=(ScopedLock const &non_copyable) { return *this; }
	        Mutex& m_locker;
	};

	//
	// Read Scoped Lock
	//
	class ReadScopedLock
	{
	    public:
	        explicit ReadScopedLock(ReadWriteMutex& locker) :m_locker(locker) { m_locker.readLock(); }
	        ~ReadScopedLock(){ m_locker.readUnlock(); }
	    private:
	        ReadScopedLock(ReadScopedLock const& non_copyable) :m_locker(non_copyable.m_locker) {}
	        ReadScopedLock& operator=(ReadScopedLock const &non_copyable) { return *this; }
	        ReadWriteMutex& m_locker;
	};

	//
	// Write Scoped Lock
	//
	class WriteScopedLock
	{
	    public:
	        explicit WriteScopedLock(ReadWriteMutex& locker) :m_locker(locker) { m_locker.writeLock(); }
	        ~WriteScopedLock(){ m_locker.writeUnlock(); }
	    private:
	        WriteScopedLock(WriteScopedLock const& non_copyable) :m_locker(non_copyable.m_locker) {}
	        WriteScopedLock& operator=(WriteScopedLock const &non_copyable) { return *this; }
	        ReadWriteMutex& m_locker;
	};

	//
	// Reversed Scoped Lock
	//
	class ReversedScopedLock
	{
	    public:
	        explicit ReversedScopedLock(Mutex& locker) :m_locker(locker) { m_locker.unlock(); }
	        ~ReversedScopedLock(){ m_locker.lock(); }
	    private:
	        ReversedScopedLock(ReversedScopedLock const& non_copyable):m_locker(non_copyable.m_locker) {}
	        ReversedScopedLock& operator=(ReversedScopedLock const &non_copyable) { return *this; }
	        Mutex& m_locker;
	};

	//
	// Read Reversed Scoped Lock
	//
	class ReadReversedScopedLock
	{
	    public:
	        explicit ReadReversedScopedLock(ReadWriteMutex& locker) :m_locker(locker) { m_locker.readUnlock(); }
	        ~ReadReversedScopedLock(){ m_locker.readLock(); }
	    private:
	        ReadReversedScopedLock(ReadReversedScopedLock const& non_copyable) :m_locker(non_copyable.m_locker) {}
	        ReadReversedScopedLock& operator=(ReadReversedScopedLock const &non_copyable) { return *this; }
	        ReadWriteMutex& m_locker;
	};

	//
	// Write Reversed Scoped Lock
	//
	class WriteReversedScopedLock
	{
	    public:
	        explicit WriteReversedScopedLock(ReadWriteMutex& locker) :m_locker(locker) { m_locker.writeUnlock(); }
	        ~WriteReversedScopedLock(){ m_locker.writeLock(); }
	    private:
	        WriteReversedScopedLock(WriteReversedScopedLock const& non_copyable) :m_locker(non_copyable.m_locker) {}
	        WriteReversedScopedLock& operator=(WriteReversedScopedLock const &non_copyable) { return *this; }
	        ReadWriteMutex& m_locker;
	};

} // end of namespace

#endif // __RENGINE_SYNCHRONIZATION_H__
