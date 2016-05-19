// __!!rengine_copyright!!__ //

#ifndef __RENGINE_BASE_EXCEPTIONS_H__
#define __RENGINE_BASE_EXCEPTIONS_H__

#include <rengine/lang/exception/Exception.h>

namespace rengine
{
	class AllocationException : public Exception
	{
	public:
		MetaException(AllocationException);
		virtual ~AllocationException() {}
	};

	class MemoryFullException : public Exception
	{
	public:
		MetaException(MemoryFullException);
		virtual ~MemoryFullException() {}
	};

	class GraphicsException : public Exception
	{
	public:
		MetaException(GraphicsException);
		virtual ~GraphicsException() {}
	};

	class FileNotFoundException : public Exception
	{
	public:
		MetaException(FileNotFoundException);
		virtual ~FileNotFoundException() {}
	};

	class NullPointerException : public Exception
	{
	public:
		MetaException(NullPointerException);
		virtual ~NullPointerException() {}
	};

} // namespace rengine

#endif // __RENGINE_BASE_EXCEPTIONS_H__
