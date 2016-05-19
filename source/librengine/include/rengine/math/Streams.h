// __!!rengine_copyright!!__ //

#ifndef __RENGINE_MATH_STREAMS_H__
#define __RENGINE_MATH_STREAMS_H__

#include <rengine/math/Vector.h>
#include <rengine/math/Matrix.h>
#include <rengine/math/Quaternion.h>

#include <ostream>

namespace rengine
{
	//Vector
	template<typename T>
	RENGINE_INLINE std::ostream& operator << (std::ostream& out, Vector2<T> const& v)
	{
		out << "[ " << v[0] << " , " << v[1] << " ] ";
		return out;
	}

	template<typename T>
	RENGINE_INLINE std::ostream& operator << (std::ostream& out, Vector3<T> const& v)
	{
		out << "[ " << v[0] << " , " << v[1] << " , " << v[2] << " ] ";
		return out;
	}

	template<typename T>
	RENGINE_INLINE std::ostream& operator << (std::ostream& out, Vector4<T> const& v)
	{
		out << "[ " << v[0] << " , " << v[1] << " , " << v[2] << " , " << v[3] << " ] ";
		return out;
	}

	//Matrix
	RENGINE_INLINE std::ostream& operator << (std::ostream& out, Matrix44 const& matrix)
	{
		out << "[ " << matrix(0, 0) << " , " << matrix(0, 1) << " , " << matrix(0, 2) << " , " << matrix(0, 3) << " ] " << std::endl;
		out << "[ " << matrix(1, 0) << " , " << matrix(1, 1) << " , " << matrix(1, 2) << " , " << matrix(1, 3) << " ] " << std::endl;
		out << "[ " << matrix(2, 0) << " , " << matrix(2, 1) << " , " << matrix(2, 2) << " , " << matrix(2, 3) << " ] " << std::endl;
		out << "[ " << matrix(3, 0) << " , " << matrix(3, 1) << " , " << matrix(3, 2) << " , " << matrix(3, 3) << " ] " << std::endl;
		return out;
	}

	//Quaternion
	RENGINE_INLINE std::ostream& operator << (std::ostream& out, Quaternion const& v)
	{
		out << "[ " << v[0] << " , " << v[1] << " , " << v[2] << " , " << v[3] << " ] ";
		return out;
	}
}

#endif //MATH_USE_STREAMS
