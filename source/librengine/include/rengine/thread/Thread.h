// __!!rengine_copyright!!__ //

#ifndef __RENGINE_THREAD_H__
#define __RENGINE_THREAD_H__

#include <rengine/thread/Synchronization.h>
#include <rengine/lang/Idioms.h>

namespace rengine
{
	class Thread : public NonCopyable
	{
	public:
		Thread();
		virtual ~Thread();

		// run on thread context before release of launching thread
		virtual void preRun() {} ;

		// working method
		virtual void run() = 0;

		Bool isRunning();

		void start();
		void stop(); //signals stop and waits for thread to end work

		Int getThreadId() const;
		Int getProccessId() const;
		void setProcessorAfinity(Int cpu);
		Int getProcessorAffinity() const;

		//
		// will inform thread that should stop,
		// next call to keepRunning will return false
		//
		void signalShouldStop();

		static Int microSleep(Uint const& microseconds);
		static Int numberOfProcessors();
		static Thread* currentThread();

		struct PrivateImplementation;
		PrivateImplementation* privateImplementation() { return m_pimpl; }

	protected:
		Bool keepRunning();
		PrivateImplementation* m_pimpl;
	};

}  // end of namespace

#endif //__RENGINE_THREAD_H__
