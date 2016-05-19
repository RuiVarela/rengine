	// __!!rengine_copyright!!__ //

#ifndef __RENGINE_DEBUG_H__
#define __RENGINE_DEBUG_H__

#include <rengine/lang/Types.h>
#include <rengine/lang/SourceCodeLocation.h>

namespace rengine
{
	void enableApplicationDebugger();

	//
	// Stack Trace
	//
	typedef void const* StackMemoryAddress;
	void createStackTrace(StackMemoryAddress* stack, Uint const size, Uint const skip = 0);


	//
	// Symbol Resolver
	//
	void resolverSetup();
	void resolverTeardown();
	SourceCodeLocation resolveAddress(void* address);

} // end of namespace


#if RENGINE_DEBUG_MODE == RENGINE_ON
	
	#include <cassert>
	#define RENGINE_ASSERT(expression) assert(expression)

#else

	#define RENGINE_ASSERT(expression)

#endif //RENGINE_DEBUG_MODE == RENGINE_ON




#endif // __RENGINE_DEBUG_H__
