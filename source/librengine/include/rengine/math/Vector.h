// __!!rengine_copyright!!__ //

#ifndef __RENGINE_VECTOR_H__
#define __RENGINE_VECTOR_H__

#include <rengine/math/Math.h>

namespace rengine
{
	//
	// 2D Vector
	//

	template<typename T>
	class Vector2
	{
	public:
		static Uint const number_of_elements = 2;
		typedef T ValueType;

		Vector2()
		{
			_v[0] = ValueType(0.0);
			_v[1] = ValueType(0.0);
		}

		Vector2(ValueType const x, ValueType const y)
		{
			_v[0] = x;
			_v[1] = y;
		}

		Vector2(Vector2 const& v)
		{
			_v[0] = v[0];
			_v[1] = v[1];
		}

		template<typename VT>
		Vector2(Vector2<VT> const& v)
		{
			_v[0] = ValueType(v[0]);
			_v[1] = ValueType(v[1]);
		}


		ValueType * ptr()
		{
			return _v;
		}

		ValueType const * ptr() const
		{
			return _v;
		}

		void set(ValueType const x, ValueType const y)
		{
			_v[0] = x;
			_v[1] = y;
		}

		ValueType & x()
		{
			return _v[0];
		}

		ValueType & y()
		{
			return _v[1];
		}

		ValueType x() const
		{
			return _v[0];
		}

		ValueType y() const
		{
			return _v[1];
		}

		ValueType & operator [] (Uint const i)
		{
			return _v[i];
		}

		ValueType operator [] (Uint const i) const
		{
			return _v[i];
		}

		Bool operator == (Vector2 const& v) const
		{
			return _v[0] == v._v[0] && _v[1] == v._v[1];
		}

		Bool operator != (Vector2 const& v) const
		{
			return _v[0] != v._v[0] || _v[1] != v._v[1];
		}

		Bool operator < (Vector2 const& v) const
		{
			if (_v[0] < v._v[0]) return true;
			else if (_v[0] > v._v[0]) return false;
			else return (_v[1] < v._v[1]);
		}

		ValueType operator * (Vector2 const& rhs) const
		{
			return _v[0] * rhs._v[0] + _v[1] * rhs._v[1];
		}

		Vector2 operator * (ValueType const rhs) const
		{
			return Vector2(_v[0] * rhs, _v[1] * rhs);
		}

		Vector2& operator *= (ValueType const rhs)
		{
			_v[0] *= rhs;
			_v[1] *= rhs;
			return *this;
		}

		Vector2 operator / (ValueType const rhs) const
		{
			return Vector2(_v[0] / rhs, _v[1] / rhs);
		}

		Vector2& operator /= (ValueType const rhs)
		{
			_v[0] /= rhs;
			_v[1] /= rhs;
			return *this;
		}

		Vector2 operator + (Vector2 const& rhs) const
		{
			return Vector2(_v[0] + rhs._v[0], _v[1] + rhs._v[1]);
		}

		Vector2& operator += (Vector2 const& rhs)
		{
			_v[0] += rhs._v[0];
			_v[1] += rhs._v[1];
			return *this;
		}

		Vector2 operator - (Vector2 const& rhs) const
		{
			return Vector2(_v[0] - rhs._v[0], _v[1] - rhs._v[1]);
		}

		Vector2& operator -= (Vector2 const& rhs)
		{
			_v[0] -= rhs._v[0];
			_v[1] -= rhs._v[1];
			return *this;
		}

		Vector2 operator - () const
		{
			return Vector2 (-_v[0], -_v[1]);
		}

		ValueType length() const
		{
			return sqrtf(_v[0] * _v[0] + _v[1] *_v[1] );
		}

		ValueType lengthSquared() const
		{
			return _v[0] * _v[0] + _v[1] * _v[1];
		}

		ValueType normalize()
		{
			ValueType norm = Vector2::length();
			if (norm > 0.0)
			{
				ValueType inv = ValueType(1.0) / norm;
				_v[0] *= inv;
				_v[1] *= inv;
			}
			return norm;
		}
	private:
		ValueType _v[2];
	};

	template<typename T>
	RENGINE_INLINE Vector2<T> operator * (T const lhs, Vector2<T> rhs)
	{
		rhs *= lhs;
		return rhs;
	}

	//
	// 3D Vector
	//

	template<typename T>
	class Vector3
	{
	public:
		static Uint const number_of_elements = 3;
		typedef T ValueType;

		Vector3()
		{
			_v[0] = ValueType(0.0);
			_v[1] = ValueType(0.0);
			_v[2] = ValueType(0.0);
		}

		Vector3(ValueType const x, ValueType const y, ValueType const z)
		{
			_v[0] = x;
			_v[1] = y;
			_v[2] = z;
		}

		Vector3(Vector3 const& v)
		{
			_v[0] = v[0];
			_v[1] = v[1];
			_v[2] = v[2];
		}

		template<typename VT>
		Vector3(Vector3<VT> const& v)
		{
			_v[0] = ValueType(v[0]);
			_v[1] = ValueType(v[1]);
			_v[2] = ValueType(v[2]);
		}

		Vector3(Vector2<T> const& v, ValueType const z)
		{
			_v[0] = v[0];
			_v[1] = v[1];
			_v[2] = z;
		}

		ValueType* ptr()
		{
			return _v;
		}

		ValueType const * ptr() const
		{
			return _v;
		}

		void set(ValueType const x, ValueType const y, ValueType const z)
		{
			_v[0] = x;
			_v[1] = y;
			_v[2] = z;
		}

		ValueType& x()
		{
			return _v[0];
		}

		ValueType& y()
		{
			return _v[1];
		}

		ValueType& z()
		{
			return _v[2];
		}

		ValueType x() const
		{
			return _v[0];
		}

		ValueType y() const
		{
			return _v[1];
		}

		ValueType z() const
		{
			return _v[2];
		}

		ValueType& operator [] (Uint const i)
		{
			return _v[i];
		}

		ValueType operator [] (Uint const i) const
		{
			return _v[i];
		}

		Bool operator == (Vector3 const& v) const
		{
			return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2];
		}

		Bool operator != (Vector3 const& v) const
		{
			return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2];
		}

		Bool operator < (Vector3 const& v) const
		{
			if (_v[0] < v._v[0]) return true;
			else if (_v[0] > v._v[0]) return false;
			else if (_v[1] < v._v[1]) return true;
			else if (_v[1] > v._v[1]) return false;
			else return (_v[2] < v._v[2]);
		}

		ValueType operator * (Vector3 const& rhs) const
		{
			return _v[0] * rhs._v[0] +_v[1] * rhs._v[1] +_v[2] * rhs._v[2];
		}

		Vector3 const operator ^ (Vector3 const& rhs) const
		{
			return Vector3(
				_v[1] * rhs._v[2] - _v[2] * rhs._v[1],
				_v[2] * rhs._v[0] - _v[0] * rhs._v[2],
				_v[0] * rhs._v[1] - _v[1] * rhs._v[0]
			);
		}

		Vector3 operator * (ValueType const rhs) const
		{
			return Vector3(_v[0] * rhs, _v[1] * rhs, _v[2] * rhs);
		}

		Vector3& operator *= (ValueType const rhs)
		{
			_v[0] *= rhs;
			_v[1] *= rhs;
			_v[2] *= rhs;
			return *this;
		}

		Vector3 operator / (ValueType const rhs) const
		{
			return Vector3(_v[0] / rhs, _v[1] / rhs, _v[2] / rhs);
		}

		Vector3& operator /= (ValueType const rhs)
		{
			_v[0] /= rhs;
			_v[1] /= rhs;
			_v[2] /= rhs;
			return *this;
		}

		Vector3 operator + (Vector3 const& rhs) const
		{
			return Vector3(_v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2]);
		}

		Vector3& operator += (Vector3 const& rhs)
		{
			_v[0] += rhs._v[0];
			_v[1] += rhs._v[1];
			_v[2] += rhs._v[2];
			return *this;
		}

		Vector3 operator - (Vector3 const& rhs) const
		{
			return Vector3(_v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2]);
		}

		Vector3& operator -= (Vector3 const& rhs)
		{
			_v[0] -= rhs._v[0];
			_v[1] -= rhs._v[1];
			_v[2] -= rhs._v[2];
			return *this;
		}

		Vector3 operator - () const
		{
			return Vector3 (-_v[0], -_v[1], -_v[2]);
		}


		ValueType length() const
		{
			return sqrt( _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] );
		}


		ValueType lengthSquared() const
		{
			return _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2];
		}

		ValueType normalize()
		{
			ValueType norm = Vector3::length();
			if (norm > 0.0)
			{
				ValueType inv = ValueType(1.0) / norm;
				_v[0] *= inv;
				_v[1] *= inv;
				_v[2] *= inv;
			}

			return norm;
		}
	private:
		ValueType _v[3];
	};

	template<typename T>
	RENGINE_INLINE Vector3<T> operator * (T const lhs, Vector3<T> rhs)
	{
		rhs *= lhs;
		return rhs;
	}

	//
	// 4D Vector
	//

	template<typename T>
	class Vector4
	{
	public:
		static Uint const number_of_elements = 4;
		typedef T ValueType;

		Vector4()
		{
			_v[0] = ValueType(0.0);
			_v[1] = ValueType(0.0);
			_v[2] = ValueType(0.0);
			_v[3] = ValueType(0.0);
		}

		Vector4(ValueType const x, ValueType const y, ValueType const z, ValueType const w)
		{
			_v[0] = x;
			_v[1] = y;
			_v[2] = z;
			_v[3] = w;
		}

		Vector4(Vector4 const& v)
		{
			_v[0] = v[0];
			_v[1] = v[1];
			_v[2] = v[2];
			_v[3] = v[3];
		}

		template<typename VT>
		Vector4(Vector4<VT> const& v)
		{
			_v[0] = ValueType(v[0]);
			_v[1] = ValueType(v[1]);
			_v[2] = ValueType(v[2]);
			_v[3] = ValueType(v[3]);
		}

		Vector4(Vector2<T> const& v, ValueType const z, ValueType const w)
		{
			_v[0] = v[0];
			_v[1] = v[1];
			_v[2] = z;
			_v[3] = w;
		}

		Vector4(Vector3<T> const& v, ValueType const w)
		{
			_v[0] = v[0];
			_v[1] = v[1];
			_v[2] = v[2];
			_v[3] = w;
		}

		ValueType* ptr()
		{
			return _v;
		}

		ValueType const * ptr() const
		{
			return _v;
		}

		void set(ValueType const x, ValueType const y, ValueType const z, ValueType const w)
		{
			_v[0] = x;
			_v[1] = y;
			_v[2] = z;
			_v[3] = w;
		}

		ValueType& x()
		{
			return _v[0];
		}

		ValueType& y()
		{
			return _v[1];
		}

		ValueType& z()
		{
			return _v[2];
		}

		ValueType& w()
		{
			return _v[3];
		}

		ValueType x() const
		{
			return _v[0];
		}

		ValueType y() const
		{
			return _v[1];
		}

		ValueType z() const
		{
			return _v[2];
		}

		ValueType w() const
		{
			return _v[3];
		}

		ValueType& r()
		{
			return _v[0];
		}

		ValueType& g()
		{
			return _v[1];
		}

		ValueType& b()
		{
			return _v[2];
		}

		ValueType& a()
		{
			return _v[3];
		}

		ValueType r() const
		{
			return _v[0];
		}

		ValueType g() const
		{
			return _v[1];
		}

		ValueType b() const
		{
			return _v[2];
		}

		ValueType a() const
		{
			return _v[3];
		}

		Uint asABGR() const
		{
			return
				Uint(clampTo(_v[0] * ValueType(255.0), ValueType(0.0), ValueType(255.0))) << 24 |
				Uint(clampTo(_v[1] * ValueType(255.0), ValueType(0.0), ValueType(255.0))) << 16 |
				Uint(clampTo(_v[2] * ValueType(255.0), ValueType(0.0), ValueType(255.0))) << 8  |
				Uint(clampTo(_v[3] * ValueType(255.0), ValueType(0.0), ValueType(255.0)));
		}

		Uint asRGBA() const
		{
			return
				Uint(clampTo(_v[3] * ValueType(255.0), ValueType(0.0), ValueType(255.0))) << 24 |
				Uint(clampTo(_v[2] * ValueType(255.0), ValueType(0.0), ValueType(255.0))) << 16 |
				Uint(clampTo(_v[1] * ValueType(255.0), ValueType(0.0), ValueType(255.0))) << 8  |
				Uint(clampTo(_v[0] * ValueType(255.0), ValueType(0.0), ValueType(255.0)));
		}

		Bool operator == (Vector4 const& v) const
		{
			return _v[0] == v._v[0] && _v[1] == v._v[1] && _v[2] == v._v[2] && _v[3] == v._v[3];
		}

		Bool operator != (Vector4 const& v) const
		{
			return _v[0] != v._v[0] || _v[1] != v._v[1] || _v[2] != v._v[2] || _v[3] != v._v[3];
		}

		Bool operator < (Vector4 const& v) const
		{
			if (_v[0] < v._v[0]) return true;
			else if (_v[0] > v._v[0]) return false;
			else if (_v[1] < v._v[1]) return true;
			else if (_v[1] > v._v[1]) return false;
			else if (_v[2] < v._v[2]) return true;
			else if (_v[2] > v._v[2]) return false;
			else return (_v[3] < v._v[3]);
		}

		ValueType& operator [] (Uint const i)
		{
			return _v[i];
		}

		ValueType operator [] (Uint const i) const
		{
			return _v[i];
		}

		ValueType operator * (Vector4 const& rhs) const
		{
			return _v[0] * rhs._v[0] + _v[1] * rhs._v[1] + _v[2] * rhs._v[2] + _v[3] * rhs._v[3] ;
		}

		Vector4 operator * (ValueType const rhs) const
		{
			return Vector4(_v[0] * rhs, _v[1] * rhs, _v[2] * rhs, _v[3] * rhs);
		}

		Vector4& operator *= (ValueType const rhs)
		{
			_v[0] *= rhs;
			_v[1] *= rhs;
			_v[2] *= rhs;
			_v[3] *= rhs;
			return *this;
		}

		Vector4 operator / (ValueType const rhs) const
		{
			return Vector4(_v[0] / rhs, _v[1] / rhs, _v[2] / rhs, _v[3] / rhs);
		}

		Vector4& operator /= (ValueType const rhs)
		{
			_v[0] /= rhs;
			_v[1] /= rhs;
			_v[2] /= rhs;
			_v[3] /= rhs;
			return *this;
		}

		Vector4 operator + (Vector4 const& rhs) const
		{
			return Vector4(_v[0] + rhs._v[0], _v[1] + rhs._v[1], _v[2] + rhs._v[2], _v[3] + rhs._v[3]);
		}

		Vector4& operator += (Vector4 const& rhs)
		{
			_v[0] += rhs._v[0];
			_v[1] += rhs._v[1];
			_v[2] += rhs._v[2];
			_v[3] += rhs._v[3];
			return *this;
		}

		Vector4 operator - (Vector4 const& rhs) const
		{
			return Vector4(_v[0] - rhs._v[0], _v[1] - rhs._v[1], _v[2] - rhs._v[2], _v[3] - rhs._v[3]);
		}

		Vector4& operator -= (Vector4 const& rhs)
		{
			_v[0] -= rhs._v[0];
			_v[1] -= rhs._v[1];
			_v[2] -= rhs._v[2];
			_v[3] -= rhs._v[3];
			return *this;
		}

		Vector4 operator - () const
		{
			return Vector4 (-_v[0], -_v[1], -_v[2], -_v[3]);
		}

		ValueType length() const
		{
			return sqrt( _v[0] * _v[0] + _v[1] * _v[1] + _v[2] * _v[2] + _v[3] * _v[3]);
		}

		ValueType lengthSquared() const
		{
			return _v[0]*_v[0] + _v[1]*_v[1] + _v[2]*_v[2] + _v[3]*_v[3];
		}

		ValueType normalize()
		{
			ValueType norm = Vector4::length();
			if (norm > ValueType(0.0))
			{
				ValueType inv = ValueType(1.0) / norm;
				_v[0] *= inv;
				_v[1] *= inv;
				_v[2] *= inv;
				_v[3] *= inv;
			}
			return norm;
		}
	private:
		ValueType _v[4];

	};

	template<typename T>
	RENGINE_INLINE Vector4<T> operator * (T const lhs, Vector4<T> rhs)
	{
		rhs *= lhs;
		return rhs;
	}


	//
	// Rectangle
	//
	template <typename T = Real>
	class Rectangle
	{
	public:
		typedef T ValueType;
		typedef Vector2<ValueType> VectorType;

		Rectangle()
		{
		}

		Rectangle(VectorType const& bottom_left, VectorType const& top_right)
			:m_top_right(top_right), m_bottom_left(bottom_left)
		{
		}

		Rectangle(ValueType const& left, ValueType const& bottom, ValueType const& right, ValueType const& top)
			:m_bottom_left(VectorType(left, bottom)), m_top_right(VectorType(right, top))
		{
		}

		ValueType& left()
		{
			return m_bottom_left.x();
		}
		
		ValueType left() const
		{
			return m_bottom_left.x();
		}

		ValueType& bottom() 
		{
			return m_bottom_left.y();
		}

		ValueType bottom() const
		{
			return m_bottom_left.y();
		}

		ValueType& right()
		{
			return m_top_right.x();
		}

		ValueType right() const
		{
			return m_top_right.x();
		}

		ValueType& top()
		{
			return m_top_right.y();
		}

		ValueType top() const
		{
			return m_top_right.y();
		}

		ValueType width() const
		{
			return right() - left();
		}

		ValueType height() const
		{
			return top() - bottom();
		}

		VectorType& bottomLeft()
		{
			return m_bottom_left;
		}
		
		VectorType const& bottomLeft() const
		{
			return m_bottom_left;
		}

		VectorType& topRight()
		{
			return m_top_right;
		}
				
		VectorType const& topRight() const
		{
			return m_top_right;
		}

	private:
		VectorType m_bottom_left;
		VectorType m_top_right;
	};

	//
	// Specific Types
	//

	typedef Vector2<Int> Vector2Di;
	typedef Vector2<Uint> Vector2Dui;
	typedef Vector2<Real> Vector2D;

	typedef Vector3<Int> Vector3Di;
	typedef Vector3<Uint> Vector3Dui;
	typedef Vector3<Real> Vector3D;

	typedef Vector4<Int> Vector4Di;
	typedef Vector4<Uint> Vector4Dui;
	typedef Vector4<Real> Vector4D;

	typedef Rectangle<Int> Rectanglei;
	typedef Rectangle<Uint> Rectangleui;

} // namespace rengine

#endif //__RENGINE_VECTOR_H__
