// __!!rengine_copyright!!__ //

#ifndef __RENGINE_PLATFORM_H__
#define __RENGINE_PLATFORM_H__

#define RENGINE_OFF 0
#define RENGINE_ON 1

#define RENGINE_PLATFORM_WIN32 1
#define RENGINE_PLATFORM_LINUX 2
#define RENGINE_PLATFORM_APPLE 3

#define RENGINE_COMPILER_MSVC 1
#define RENGINE_COMPILER_GNUC 2

#define RENGINE_ARCHITECTURE_32 1
#define RENGINE_ARCHITECTURE_64 2

#if defined( _MSC_VER )
	#define RENGINE_COMPILER RENGINE_COMPILER_MSVC
	#define RENGINE_COMPILER_VERSION _MSC_VER
#elif defined( __GNUC__ )
	#define RENGINE_COMPILER RENGINE_COMPILER_GNUC
	#define RENGINE_COMPILER_VERSION ( ((__GNUC__) * 100) + (__GNUC_MINOR__ * 10) + __GNUC_PATCHLEVEL__)
#else
	#pragma error "Compiler not Supported"
#endif


#if defined( __WIN32__ ) || defined( _WIN32 )
	#define RENGINE_PLATFORM RENGINE_PLATFORM_WIN32
#elif defined( __APPLE_CC__)
	#define RENGINE_PLATFORM RENGINE_PLATFORM_APPLE
#else
	#define RENGINE_PLATFORM RENGINE_PLATFORM_LINUX
#endif

#if defined(__x86_64__) || defined(_M_X64) || defined(__powerpc64__) || defined(__alpha__) || defined(__ia64__) || defined(__s390__) || defined(__s390x__)
	#define RENGINE_ARCH_TYPE RENGINE_ARCHITECTURE_64
#else
	#define RENGINE_ARCH_TYPE RENGINE_ARCHITECTURE_32
#endif


#if RENGINE_COMPILER == RENGINE_COMPILER_MSVC
	#if defined(_DEBUG) || !defined(NDEBUG)
		#define RENGINE_DEBUG_MODE RENGINE_ON
	#else
		#define RENGINE_DEBUG_MODE RENGINE_OFF
	#endif
#else
	#ifdef NDEBUG
		#define RENGINE_DEBUG_MODE RENGINE_OFF
	#else
		#define RENGINE_DEBUG_MODE RENGINE_ON
	#endif
#endif


#if RENGINE_COMPILER == RENGINE_COMPILER_MSVC
	#define RENGINE_INLINE __forceinline
#elif RENGINE_COMPILER == RENGINE_COMPILER_GNUC	
	#define RENGINE_INLINE inline __attribute__((always_inline))
#else
	#define RENGINE_INLINE inline
#endif

#if RENGINE_COMPILER == RENGINE_COMPILER_MSVC
	#define RENGINE_FUNCTION __FUNCSIG__
	//#define RENGINE_FUNCTION __FUNCDNAME__
#elif RENGINE_COMPILER == RENGINE_COMPILER_GNUC
	#define RENGINE_FUNCTION __PRETTY_FUNCTION__
#endif


#define RENGINE_LITTLE_ENDIAN 0
#define RENGINE_BIG_ENDIAN 1

#define RENGINE_ENDIAN RENGINE_LITTLE_ENDIAN


//
// types
//

#define RG_REAL32 float
#define RG_REAL64 double
#define RG_REAL RG_REAL32

#define RG_INT8 signed char
#define RG_INT16 signed short
#define RG_INT32 signed int

#define RG_UINT8 unsigned char
#define RG_UINT16 unsigned short
#define RG_UINT32 unsigned int

#if RENGINE_COMPILER == RENGINE_COMPILER_MSVC
	#define RG_INT64 signed __int64
	#define RG_UINT64  unsigned __int64
	#define RENGINE_INT64_MASK "I64"
#else
	#define RG_INT64 long long int
	#define RG_UINT64 unsigned long long int
	#define RENGINE_INT64_MASK "ll"
#endif



#endif // __RENGINE_PLATFORM_H__
