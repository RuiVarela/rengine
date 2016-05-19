#include <rengine\lang\Lang.h>
#include <rengine\time\Timer.h>
#include "OS.h"

static rengine::Timer m_timer;

extern "C"
{
	void OS_Initialize()
	{
		m_timer = rengine::Timer();
		m_timer.start();
	}

	void OS_Finalize()
	{
	}

	double OS_ElapsedTime()
	{
		return m_timer.elapsedTime();
	}

	char* OS_GetFilePath(const char* filename)
	{
		int size = 13;
		int filename_size = strlen(filename);

		char* output = (char*)OS_Malloc(size + filename_size + 1);

		if (output)
		{
			memcpy(output, "data\\squared\\", size);
			memcpy(output + size, filename, filename_size);
			output[size + filename_size] = 0;
		}

		return output;
	}
}