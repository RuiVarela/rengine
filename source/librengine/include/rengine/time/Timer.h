// __!!rengine_copyright!!__ //

#ifndef _RENGINE_TIMER_H__
#define _RENGINE_TIMER_H__

#include <rengine/math/Math.h>
#include <rengine/lang/Lang.h>
#include <limits.h>

namespace rengine
{
	class Timer
	{
	public:
		typedef Uint64 TickType;
		typedef Uint64 OperationCount;

		Timer() 
			:start_tick(0), seconds_per_tick(0.0)
		{
			start();
		}

		~Timer(){ }

		// Starts the timer
		void start();

		// total time elapse since start();
		Real64 elapsedTime() const
		{
			return (Real64) (getTick() - start_tick) * seconds_per_tick;
		}

		Real64 elapsedTimeMilliseconds() const
		{
			return elapsedTime() * 1000.0;
		}

		// Operation time
		Real64 operationTime() const
		{
			return operation_time_;
		}

		// advances one operation
		Real64 advanceOperation()
		{
			Real64 current_time = elapsedTime();
			operation_time_ = current_time - last_time_;
			++operation_;
			last_time_ = current_time;
			return current_time;
		}

		// time elapsed since operation start
		Real64 elapsedSinceOperationStart() const
		{
			return elapsedTime() - last_time_;
		}

		// current operation number
		OperationCount operation() const { return operation_; }

		// calculates the current fps
		Int getFps() const
		{
			if (equivalent(operation_time_, 0.0))
			{
				return INT_MAX;
			}
			return Int(1.0 / operation_time_);
		}

	private:
		TickType getTick() const;

		void initialize()
		{
			last_time_= 0.0;
			operation_time_ = 0.0;
			operation_ = 0;
		}

		Real64 last_time_;
		Real64 operation_time_;
		OperationCount operation_;
		TickType start_tick;
		Real64 seconds_per_tick;
	};

} // end of namespace

#endif //_RENGINE_TIMER_H__
