// __!!rengine_copyright!!__ //

#include <rengine/time/Timer.h>
#include <rengine/lang/debug/Debug.h>

#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32

#include <sys/types.h>
#include <fcntl.h>
#include <windows.h>
#include <winbase.h>

namespace rengine
{
	void Timer::start()
	{
		seconds_per_tick = 1.0;

		LARGE_INTEGER frequency;
		if(QueryPerformanceFrequency(&frequency))
		{
			seconds_per_tick /= Real64(frequency.QuadPart);
		}

		start_tick = getTick();
		initialize();
	}

	Timer::TickType Timer::getTick() const
	{
		LARGE_INTEGER qpc;
		if (QueryPerformanceCounter(&qpc))
		{
			return qpc.QuadPart;
		}
		return 0;
	}

} // namespace rengine

#else

#include <sys/time.h>

namespace rengine
{
	void Timer::start()
	{
		seconds_per_tick = (1.0 / 1000000.0);
		start_tick = getTick();
		initialize();
	}

	Timer::TickType Timer::getTick() const
	{
		struct timeval time_val;
		gettimeofday(&time_val, 0);

		return ((Timer::TickType)time_val.tv_sec) * 1000000 + (Timer::TickType) time_val.tv_usec;
	}

} // namespace rengine

#endif //WIN32

