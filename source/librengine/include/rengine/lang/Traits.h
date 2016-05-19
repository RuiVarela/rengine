// __!!rengine_copyright!!__ //

#ifndef __RENGINE_TRAITS_H__
#define __RENGINE_TRAITS_H__

#include <rengine/lang/Lang.h>


namespace rengine
{
	struct TrueType  { static const Bool value = true; };
	struct FalseType { static const Bool value = false; };

	//
	// Is Void
	//
	template <typename T>
	struct IsVoid : public FalseType {};

	template <>
	struct IsVoid<void> : public TrueType {};

	//
	// Is Pointer
	//
	template <typename T>
	struct IsPointer : public FalseType {};

	template<typename T>
	struct IsPointer<T*> : public TrueType {};

	//
	// Is Const
	//
	template<typename >
	struct IsConst : public FalseType {};

	template<typename T>
	struct IsConst<T const> : public TrueType {};


	//
	// Remove Bounds
	//
	// i.e removeBounds< int[1], 6>::Type => int[6]
	//
	template<typename T>
	struct RemoveBounds
	{
		typedef T Type;
	};

	template<typename T, std::size_t N>
	struct RemoveBounds<T[N]>
	{
		typedef T Type;
	};


	//
	// Const Template
	//
	template <typename T>
	struct RemoveConst
	{
	    typedef T Type;
	};

	template <typename T>
	struct RemoveConst<const T>
	{
	    typedef T Type;
	};

	template<typename T>
	struct AddConst
	{
		typedef T const Type;
	};

}

#endif //__RENGINE_TRAITS_H__
