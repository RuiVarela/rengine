#include "UnitTest/UnitTest.h"

#include <rengine/thread/Thread.h>
#include <rengine/lang/Lang.h>
#include <rengine/math/Math.h>

#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <limits>

using namespace rengine;

#define DISABLE_THREAD_TEST 1
#define DISABLE_POINTER_TEST 1
#define DISABLE_READ_WRITE_TEST 1

//#define TEST_LOG(msg) std::cout << msg << std::endl;
#define TEST_LOG(msg)

unsigned int const max_counter = 10;
unsigned int const max_threads = 10;

class TestThread : public Thread
{
public:
	TestThread(Barrier& threads_barrier, int thread_index)
		:barrier(threads_barrier), index(thread_index )
	{
		counter = 0;
	}

	virtual void preRun()
	{
		TEST_LOG("[" << index << "] " << TestThread::currentThread() << " PreRun!");
	}

	virtual void run()
	{
		for (unsigned int i = 0; i != max_counter; ++i)
		{
			++counter;

			TEST_LOG("[" << index << "] " << TestThread::currentThread() << " [" << i << "]")

			if ( (i % 5) == 0)
			{
				TEST_LOG("[" << index << "] " << TestThread::currentThread() << " [" << i << "] Block ")
				barrier.block();
			}

			//
			// spin lock, test cpu affinity
			//
//			int const spin_value = 10000;
//			for (int s_q = 0; s_q != spin_value; ++s_q)
//			{
//				for (int s_w = 0; s_w != spin_value; ++s_w)
//				{
//					for (int s_e = 0; s_e != spin_value; ++s_e)
//					{
//						int z = s_q;
//						int x = s_w;
//						int c = s_e;
//
//						x = z * c;
//						z = x * c;
//						c = z * x;
//
//					}
//				}
//			}
			Thread::microSleep((rand() % ( (index + 1) * 10)) * 1000);
		}
	}

	int counter;

	Barrier& barrier;
	int index;
};


//
// UnitTestThread
//

UNITT_TEST_BEGIN_CLASS(UnitTestThread)

	virtual void run()
	{
		{
			Mutex mutex;
			ScopedLock lock00(mutex);

			UNITT_FAIL_NOT_EQUAL(true, mutex.tryLock());
			mutex.unlock();

		}

		{
			Mutex mutex;

			UNITT_FAIL_NOT_EQUAL(true, mutex.tryLock());
			UNITT_FAIL_NOT_EQUAL(true, mutex.tryLock());

			mutex.unlock();
			mutex.unlock();
		}


#ifdef DISABLE_THREAD_TEST
	return;
#endif //DISABLE_THREAD_TEST

		UNITT_FAIL_NOT_EQUAL(4, Thread::numberOfProcessors());

		{
			srand(time(0));

			Barrier barrier(max_threads);
			std::vector< SharedPointer<TestThread> > threads;

			for (unsigned int i = 0; i != max_threads; ++i)
			{
				threads.push_back(new TestThread(barrier, i));
				UNITT_FAIL_NOT_EQUAL(0, threads.back()->counter);

				//threads.back()->setProcessorAfinity(0);
				//threads.back()->setProcessorAfinity(1);
				//threads.back()->setProcessorAfinity(2);
				//threads.back()->setProcessorAfinity(3);
			}


			for (unsigned int i = 0; i != max_threads; ++i)
			{
				TEST_LOG("PID: " << threads[i]->getProccessId() << " TID: " << threads[i]->getThreadId());
				TEST_LOG("[" << i << "]" << " Before start!");
				threads[i]->start();
			}


			for (unsigned int i = 0; i != max_threads; ++i)
			{
				threads[i]->stop();
				TEST_LOG("[" << i << "]" << " After Stop!");
			}


			for (unsigned int i = 0; i != max_threads; ++i)
			{
				UNITT_FAIL_NOT_EQUAL(max_counter, threads.back()->counter);
			}
		}

}

UNITT_TEST_END_CLASS(UnitTestThread)


//
// UnitTestAtomic
//

UNITT_TEST_BEGIN_CLASS(UnitTestAtomic)

	virtual void run()
	{
		Atomic atomic;
		Atomic::AtomicValue value = 0;

		Atomic::AtomicValue const or_mask = 123;
		Atomic::AtomicValue const and_mask = 321;
		Atomic::AtomicValue const xor_mask = 1928;
		Atomic::AtomicValue const exchange = 12345;

		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic++, value++);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(++atomic, ++value);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic--, value--);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(--atomic, --value);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic += 5, value += 5);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic -= 50, value -= 50);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic |= or_mask, value |= or_mask);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic &= and_mask, value &= and_mask);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic ^= xor_mask, value ^= xor_mask);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);

		UNITT_FAIL_NOT_EQUAL(atomic = exchange, value = exchange);
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);


		UNITT_FAIL_NOT_EQUAL(atomic.exchange(0), exchange);
		value = 0;
		UNITT_FAIL_NOT_EQUAL((Atomic::AtomicValue) atomic, value);
	}

UNITT_TEST_END_CLASS(UnitTestAtomic)


//
// UnitTestThreadSharedPointer
//

unsigned int const max_pointers = 10;
unsigned int const max_recursive_level = 2;
unsigned int const max_runs = 3;

struct PointerData
{
	PointerData() :i(0), d(0.0) {}

	int i;
	double d;
};

typedef SharedPointer<PointerData> SharedData;

class TestPointerThread : public Thread
{
public:
	TestPointerThread(SharedData** dataArray, SharedData data, unsigned int index, unsigned int recursiveLevel)
		:m_data(data), m_dataArray(dataArray), m_index(index), m_recursiveLevel(recursiveLevel)
	{}

	virtual void run()
	{
		std::vector< SharedPointer<TestPointerThread> > threads;

		if (m_recursiveLevel < max_recursive_level)
		{
			for (unsigned int i = 0; i != max_threads; ++i)
			{
				unsigned int dataIndex = i % max_pointers;
				threads.push_back(new TestPointerThread(m_dataArray, *(m_dataArray[dataIndex]), dataIndex, m_recursiveLevel + 1));
			}
		}
		else
		{
			m_data->i++;
			m_data->d += 1.0;

			SharedData data = m_data;

			for (unsigned int i = 0; i != 10; ++i)
			{

#ifdef RENGINE_SHARED_POINTER_THREAD_SAFE

				SharedData new_data = new PointerData();
				data = new_data;
				m_data = data;
				data->i++;
				data->d += 1.0;
				m_data = new PointerData();
				*(m_dataArray[m_index]) = m_data;

#else
				SharedData data0 = m_data;
				SharedData data1 = *(m_dataArray[m_index]);

				data0->i++;
				data0->d += 1.0;

				data1->i++;
				data1->d += 1.0;

#endif //RENGINE_SHARED_POINTER_THREAD_SAFE

			}



		}

		for (unsigned int i = 0; i != (unsigned int) threads.size(); ++i)
		{
			TEST_LOG("[" << m_recursiveLevel << ":" <<  i << "] PID: " << threads[i]->getProccessId() << " TID: " << threads[i]->getThreadId());
			threads[i]->start();
		}


		//Thread::microSleep(10 * 1000);


		for (unsigned int i = 0; i != (unsigned int) threads.size(); ++i)
		{
			threads[i]->stop();
			TEST_LOG("[" << m_recursiveLevel << ":" <<  i << "]" << " After Stop!");
		}

		m_data = new PointerData();
	}

	SharedData m_data;
	SharedData** m_dataArray;
	unsigned int m_index;
	unsigned int m_recursiveLevel;
};

UNITT_TEST_BEGIN_CLASS(UnitTestThreadSharedPointer)

	virtual void run()
	{

#ifdef DISABLE_POINTER_TEST
	return;
#endif //DISABLE_POINTER_TEST

		SharedData* sharedData[max_pointers];
		memset(sharedData, 0, sizeof(SharedData*));

		for (unsigned int i = 0; i != max_pointers; ++i)
		{
			sharedData[i] = new SharedData();
		}

		for (unsigned int i = 0; i != max_pointers; ++i)
		{
			(*sharedData[i]) = new PointerData();
		}


		for (unsigned int i = 0; i != max_runs; ++i)
		{
			TestPointerThread thread(sharedData, *(sharedData[0]), 0, 0);
			thread.start();

			//Thread::microSleep(1000 * 1000);
			//TEST_LOG("Loop: " <<  i);
			thread.stop();
		}


		for (unsigned int i = 0; i != max_pointers; ++i)
		{
			delete( sharedData[i] );
		}
	}

UNITT_TEST_END_CLASS(UnitTestThreadSharedPointer)




//
// UnitTestReadWriteMutex
//

SharedData rw_data[max_pointers];
ReadWriteMutex rw_mutex[max_pointers];
int const read_threads = 100;
int const write_threads = 10;


int write_min = std::numeric_limits<Int>::max();
int write_max = std::numeric_limits<Int>::min();
Mutex writers_min_max_lock;

class TestReadWriteThread : public Thread
{
public:
	TestReadWriteThread(Int index, Bool type) :m_index(index % max_pointers), m_type(type)
	{
		m_max = std::numeric_limits<Int>::min();
		m_min = std::numeric_limits<Int>::max();

		m_reads = 0;
		m_writes = 0;
	}

	void writerRun()
	{
		Int value = -100;

		while (keepRunning())
		{
			{
				WriteScopedLock lock(rw_mutex[m_index]);
				rw_data[m_index]->i = value;
			}

			m_writes++;
			m_max = maximum(m_max, value);
			m_min = minimum(m_min, value);

			Uint sleep_time = (rand() % (m_index + 10)) * 1000;
			//TEST_LOG("[" << m_index << "]" "Writer sleeping for " << sleep_time << " milliseconds");
			Thread::microSleep(sleep_time);

			value++;


		}


		{
			ScopedLock lock(writers_min_max_lock);

			write_max = maximum(m_max, write_max);
			write_min = minimum(m_min, write_min);
		}
	}

	void readerRun()
	{
		while (keepRunning())
		{
			Int value = 0;

			{
				ReadScopedLock lock( rw_mutex[m_index] );
				value = rw_data[m_index]->i;
			}

			m_reads++;

			m_max = maximum(m_max, value);
			m_min = minimum(m_min, value);

			Uint sleep_time = (rand() % (m_index + 5)) * 1000;
			//TEST_LOG("[" << m_index << "]" "Reader sleeping for " << sleep_time << " milliseconds");
			Thread::microSleep(sleep_time);
		}
	}

	virtual void run()
	{
		if (m_type) // writer
		{
			writerRun();
		}
		else
		{
			readerRun();
		}
	}

	Int m_max;
	Int m_min;

	Int m_index;
	Bool m_type;

	Uint m_reads;
	Uint m_writes;
};


UNITT_TEST_BEGIN_CLASS(UnitTestReadWriteMutex)

	virtual void run()
	{

#ifdef DISABLE_READ_WRITE_TEST
	return;
#endif //DISABLE_READ_WRITE_TEST


		for (int stress = 0; stress != 1; ++stress)
		{
			for (unsigned int i = 0; i != max_pointers; ++i)
			{
				rw_data[i] = new PointerData();
			}

			std::vector< SharedPointer<TestReadWriteThread> > threads;

			for (int i = 0; i != read_threads; ++i)
			{
				threads.push_back(new TestReadWriteThread(i, false) );
			}

			for (int i = 0; i != write_threads; ++i)
			{
				threads.push_back(new TestReadWriteThread(i, true) );
			}

			for (unsigned int i = 0; i != threads.size(); ++i)
			{
				//TEST_LOG("[" <<  i << "] PID: " << threads[i]->getProccessId() << " TID: " << threads[i]->getThreadId());
				threads[i]->start();
			}



			for (unsigned int i = 0; i != 1; ++i)
			{
				Uint const run_time = 3;
				TEST_LOG("["<< stress << " " << i << "] Running for " << run_time << " seconds");
				Thread::microSleep(run_time * 1000 * 1000);
			}

			TEST_LOG("Done running");

			for (unsigned int i = 0; i != threads.size(); ++i)
			{
				threads[i]->stop();
				//TEST_LOG("["<<  i << "]" << " After Stop!");
			}

			for (unsigned int i = 0; i != threads.size(); ++i)
			{
				TEST_LOG("[" << write_min << " | " << write_max << "] ["<< i << "] [" << threads[i]->m_min << " | " << threads[i]->m_max << "] [" << threads[i]->m_reads << " | " << threads[i]->m_writes << "]");

				UNITT_ASSERT(threads[i]->m_min <= write_max);
				UNITT_ASSERT(threads[i]->m_min >= write_min);

				UNITT_ASSERT(threads[i]->m_max <= write_max);
				UNITT_ASSERT(threads[i]->m_max >= write_min);

				UNITT_ASSERT(threads[i]->m_max > threads[i]->m_min);

				if (threads[i]->m_type)
				{
					UNITT_ASSERT(threads[i]->m_reads == 0);
					UNITT_ASSERT(threads[i]->m_writes > 0);
				}
				else
				{
					UNITT_ASSERT(threads[i]->m_reads > 0);
					UNITT_ASSERT(threads[i]->m_writes == 0);
				}

			}
		}

	}

UNITT_TEST_END_CLASS(UnitTestReadWriteMutex)
