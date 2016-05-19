// __!!rengine_copyright!!__ //

#ifndef __RENGINE_TYPES_H__
#define __RENGINE_TYPES_H__

#include <rengine/lang/Platform.h>

namespace rengine
{
	typedef RG_REAL32 Real32;
	typedef RG_REAL64 Real64;
	typedef RG_REAL Real;

	typedef RG_INT8 Int8;
	typedef RG_INT16 Int16;
	typedef RG_INT32 Int32;
	typedef RG_INT64 Int64;

	typedef RG_UINT8 Uint8;
	typedef RG_UINT16 Uint16;
	typedef RG_UINT32 Uint32;
	typedef RG_UINT64 Uint64;

	typedef unsigned char Uchar;
	typedef char Char;

	typedef Uint16 Ushort;
	typedef Int16 Short;

	typedef Uint32 Uint;
	typedef Int32 Int;

	typedef Uint64 Ulong;
	typedef Int64 Long;

	typedef Uint8 Byte;
	typedef bool Bool;

#if RENGINE_ARCH_TYPE == RENGINE_ARCHITECTURE_32
	typedef Int32 PointerDiff;
#else
	typedef Int64 PointerDiff;
#endif // RENGINE_ARCH_TYPE == RENGINE_ARCHITECTURE_32

} // end of namespace

#endif // __RENGINE_TYPES_H__
