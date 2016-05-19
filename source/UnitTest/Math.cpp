#include "UnitTest/UnitTest.h"

#include <rengine/math/Math.h>
#include <rengine/math/Vector.h>
#include <rengine/math/Matrix.h>
#include <rengine/math/Quaternion.h>
#include <rengine/math/BoundingVolume.h>
#include <rengine/math/Streams.h>

using namespace rengine;

template<typename T>
bool isVectorEquivalent(Vector3<T> const left,
						Vector3<T> const right,
						typename Vector3<T>::ValueType const epsilon = 1e-5)
{
	typename Vector3<T>::ValueType delta = (right - left).length();

	return (delta < typename Vector3<T>::ValueType(0.0f)) ? (delta >= -epsilon) : (delta <= epsilon);
}


//
// UnitTestMath
//

UNITT_TEST_BEGIN_CLASS(UnitTestMath)

virtual void run()
{
	UNITT_ASSERT( absolute(-1.5f) == 1.5f );
	UNITT_ASSERT( absolute(-4.2) == 4.2 );

	UNITT_ASSERT( equivalent(0.0000000001, 0.0) );
	UNITT_ASSERT( equivalent(0.0, 0.0000000001) );

	UNITT_ASSERT( !equivalent(0.0001, 0.0) );
	UNITT_ASSERT( !equivalent(0.0f, 0.0001f) );

	UNITT_ASSERT( minimum(10.2, 5.0) == 5.0 );
	UNITT_ASSERT( minimum(5.0f, 10.2f) == 5.0f );

	UNITT_ASSERT( maximum(10.2, 5.0) == 10.2 );
	UNITT_ASSERT( maximum(5.0f, 10.2f) == 10.2f );


	UNITT_ASSERT( 0.0000000001 != 0.0 );
	UNITT_ASSERT( clampNear(0.0000000001, 0.0) == 0.0 );

	UNITT_ASSERT( clampTo(12.0f, 5.0f, 10.2f) == 10.2f );
	UNITT_ASSERT( clampTo(4.0f, 5.0f, 10.2f) == 5.0f );
	UNITT_ASSERT( clampTo(7.0f, 5.0f, 10.2f) == 7.0f );

	UNITT_ASSERT( clampBelow(12.0f, 5.0f) == 5.0f );
	UNITT_ASSERT( clampBelow(1.0f, 5.0f) == 1.0f );

	UNITT_ASSERT( clampAbove(4.0f, 5.0f) == 5.0f );
	UNITT_ASSERT( clampAbove(6.0f, 5.0f) == 6.0f );

	UNITT_ASSERT( sign(7) == 1 );
	UNITT_ASSERT( sign(-2) == -1 );
	UNITT_ASSERT( sign(0) == 0 );

	UNITT_ASSERT( square(2.5) == 2.5 * 2.5 );
	UNITT_ASSERT( signedSquare(2.5) == 2.5 * 2.5 );
	UNITT_ASSERT( signedSquare(-2.5) == -2.5 * 2.5 );

	UNITT_ASSERT( equivalent( radiansToDegrees(pi_angle) , Real(180.0)) );
	UNITT_ASSERT( equivalent( degreesToRadians(Real(180.0)) , pi_angle) );

	UNITT_ASSERT( equivalent( round(1.5) , 2.0) );
	UNITT_ASSERT( equivalent( round(1.2) , 1.0) );
	UNITT_ASSERT( equivalent( round(-1.5) , -2.0) );
	UNITT_ASSERT( equivalent( round(-1.2) , -1.0) );
	UNITT_ASSERT( equivalent( round(1.5f) , 2.0f) );
	UNITT_ASSERT( equivalent( round(1.2f) , 1.0f) );

}

UNITT_TEST_END_CLASS(UnitTestMath)

//
// UnitTestVector
//

UNITT_TEST_BEGIN_CLASS(UnitTestVector)

virtual void run()
{

	UNITT_ASSERT(Vector2D(1.0f, 1.0f) == Vector2D(1.0, 1.0));
	UNITT_ASSERT(Vector2D(1.0f, 1.0f) != Vector2D(1.0, 2.0));
	UNITT_ASSERT(Vector2D(1.1f, 3.0f).x() == 1.1f);
	UNITT_ASSERT(Vector2D(1.1f, 3.0f).y() == 3.0f);

	Vector2<float> as_float2;
	Vector2<double> as_double2(as_float2);
	Vector2<double> as_double2_ = as_float2;
	as_float2 = as_double2;

	as_float2.set(1.1f, 2.2f);

	UNITT_ASSERT(Vector3D(1.0f, 1.0f, 1.0) == Vector3D(1.0, 1.0, 1.0));
	UNITT_ASSERT(Vector3D(1.0f, 1.0f, 1.0) != Vector3D(1.0, 2.0, 1.0));
	UNITT_ASSERT(Vector3D(1.1f, 3.0f, 1.0).x() == 1.1f);
	UNITT_ASSERT(Vector3D(1.1f, 3.0f, 1.0).y() == 3.0f);
	UNITT_ASSERT(Vector3D(1.1f, 3.0f, 5.0).z() == 5.0f);

	Vector3<float> as_float3;
	Vector3<double> as_double3(as_float3);
	Vector3<double> as_double3_ = as_float3;
	as_float3 = as_double3;

	as_float3.set(1.1f, 2.2f, 3.3f);

	as_double3 = 2.0 * as_double3;
	as_double3 = as_double3 * 2.0;

	Vector2<float> vec2d_conversion;
	Vector2<double> vec2dd_conversion;
	Vector3D conversion(vec2d_conversion, 1.0f);
	Vector3D conversion_(vec2dd_conversion, 1.0f);

	conversion = conversion ^ conversion_;

	Vector3<double> vec3dd_0;
	Vector3<double> vec3dd_1;
	Vector3<double> vec3dd_2;

	vec3dd_2 = vec3dd_0 ^ vec3dd_1;

	UNITT_ASSERT(Vector4D(1.0f, 1.0f, 1.0, 1.0f) == Vector4D(1.0, 1.0, 1.0, 1.0f));
	UNITT_ASSERT(Vector4D(1.0f, 1.0f, 1.0, 1.0f) != Vector4D(1.0, 2.0, 1.0, 1.0f));
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 1.0, 4.0f).x() == 1.1f);
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 1.0, 5.0f).y() == 3.0f);
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 5.0, 6.0f).z() == 5.0f);
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 5.0, 7.0f).w() == 7.0f);
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 1.0, 4.0f).r() == 1.1f);
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 1.0, 5.0f).g() == 3.0f);
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 5.0, 6.0f).b() == 5.0f);
	UNITT_ASSERT(Vector4D(1.1f, 3.0f, 5.0, 7.0f).a() == 7.0f);

	Vector4D as_float4(1.1f, 2.2f, 3.3f, 4.4f);
}

UNITT_TEST_END_CLASS(UnitTestVector)


//
// UnitTestMatrix
//

UNITT_TEST_BEGIN_CLASS(UnitTestMatrix)

virtual void run()
{
	using namespace rengine;

	float myf_matrix[16] =
	{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
	};

	Matrix matrix;
	UNITT_ASSERT( matrix.isIdentity() );

	matrix.set(myf_matrix);
	UNITT_ASSERT( matrix.isIdentity() );

	myf_matrix[0] = 12.0f;
	matrix.set(myf_matrix);
	UNITT_ASSERT( !matrix.isIdentity() );

	matrix.set(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);
	UNITT_ASSERT( matrix.isIdentity() );

	Matrix second;
	UNITT_ASSERT(matrix == second);

	matrix.set(
		1.0, 2.0, 3.0, 4.0,
		5.0, 6.0, 7.0, 8.0,
		9.0, 10.0, 11.0, 12.0,
		13.0, 14.0, 15.0, 16.0
		);

	second = matrix;
	UNITT_ASSERT(matrix == second);

	matrix.set(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0
		);

	second = matrix;

	second(2, 3) = 14.5f;
	UNITT_ASSERT(matrix != second);

	second[2 * 4 + 3] = matrix[2 * 4 + 3];
	UNITT_ASSERT(matrix == second);

	second.setAsIdentity();
	matrix(2, 3) = 14.5f;
	second = matrix;
	UNITT_ASSERT(matrix == second);
//	std::cout << "Matrix : " << std::endl << matrix;
//	std::cout << "Second : " << std::endl << second;

	// testar isto
	matrix.setAsIdentity();
	Vector3D x_axis(1.0f, 0.0f, 0.0f);
	Vector3D y_axis(1.0f, 0.0f, 0.0f);
	Vector3D z_axis(1.0f, 0.0f, 0.0f);

	Vector3D result;

	result = matrix.preMult(x_axis);
	result = matrix.postMult(x_axis);
	result = x_axis * matrix;
	result = matrix * x_axis;

	Matrix result_matrix;
	result_matrix += matrix;
	result_matrix = matrix + second;
	result_matrix -= matrix;
	result_matrix += matrix - second;


	//
	// Matrix Multiplication
	//

	Matrix one, two, three, four;

	one.postMult(two);
	one.preMult(two);
	one.mult(two, three);

	one(0, 0) = 1.0;
	one(1, 0) = 2.0;
	one(2, 0) = 3.0;
	one(3, 0) = 1.0;

	two(0, 0) = 5.0;
	two(1, 0) = 6.0;
	two(2, 0) = 7.0;
	two(3, 0) = 9.0;

	three(0, 1) = 15.0;
	three(1, 2) = 63.0;
	three(0, 3) = 37.0;
	three(1, 2) = 79.0;

	matrix = one * two;
	three = two * one;
	UNITT_ASSERT(matrix != three);

	four = one;
	four *= two;
	UNITT_ASSERT(four == matrix);

	four = one;
	four.postMult(two);
	UNITT_ASSERT(four == matrix);

	four = two;
	four.preMult(one);
	UNITT_ASSERT(four == matrix);

	matrix = one * two * three;
	four = one;
	four.postMult(two);
	four.postMult(three);
	UNITT_ASSERT(four == matrix);

	one.setAsIdentity();
	two.setAsIdentity();

	one.setTranslation( Vector3D(2.0f, 3.0f, 4.0f) );
	two.setTranslation(2.0f, 3.0f, 4.0f);
	UNITT_ASSERT(one == two);
	result = one.getTranslation();
	UNITT_ASSERT(result == Vector3D(2.0f, 3.0f, 4.0f));


	one(0, 0) =  1.0; one(0, 1) = 02.0; one(0, 2) =  3.0; one(0, 3) =  4.0;
	one(1, 0) =  5.0; one(1, 1) =  6.0; one(1, 2) =  7.0; one(1, 3) =  8.0;
	one(2, 0) =  9.0; one(2, 1) = 10.0; one(2, 2) = 11.0; one(2, 3) = 12.0;
	one(3, 0) = 13.0; one(3, 1) = 14.0; one(3, 2) = 15.0; one(3, 3) = 16.0;

	matrix.setAsTransposeOf(one);
//	std::cout << "Transpose : " << std::endl << one << std::endl << matrix << std::endl;


	matrix.setAsIdentity();
	UNITT_ASSERT(matrix.isAffine());
	matrix(0, 3) = 2.0;
	UNITT_ASSERT(!matrix.isAffine());

	one.setAsInverseOf(matrix);
	one.setAsInverseOf(two);
	one.invert();
	matrix = one.inverse();

	// translation
	matrix.setAsIdentity();
	Vector3D translation(12.0f, 10.20f, 123.0f);

	matrix.setAsTranslation(translation);
	one = Matrix::translate(translation);
	UNITT_ASSERT(matrix == one);

	Vector3D point(5.0f, 10.0f, 2.0f);

	result = point * matrix;
	//std::cout << "Translation : " << std::endl << matrix << std::endl;
	UNITT_ASSERT(result == (translation + point));
	matrix.invert();
	UNITT_ASSERT((point * matrix) == (-translation + point));
//	std::cout << "Translation : " << std::endl << matrix << std::endl;

	// scale
	Vector3D scale(2.0f, 4.0f, 5.0f);
	point.set(10.0f, 10.0f, 10.0f);
	matrix.setAsScale(scale);
	UNITT_ASSERT(matrix == Matrix::scale(scale));
//	std::cout << "Scale : " << std::endl << matrix << std::endl;

	result = point * matrix;
	UNITT_ASSERT(result == Vector3D(scale.x() * point.x(), scale.y() * point.y(), scale.z() * point.z()));

	matrix = Matrix::translate(translation) * Matrix::scale(scale);
	one = Matrix::scale(scale) * Matrix::translate(translation);
//	std::cout << "T * S : " << std::endl << matrix << std::endl;
//	std::cout << "S * T : " << std::endl << one << std::endl;
}

UNITT_TEST_END_CLASS(UnitTestMatrix)


//
// UnitTestQuaternion
//

UNITT_TEST_BEGIN_CLASS(UnitTestQuaternion)

virtual void run()
{
	using namespace rengine;

	Quaternion quaternion;
	Quaternion quaternion0(1.0f, 2.0f, 3.0f, 4.0f);
	Quaternion quaternion1(6.0f, 215.0f, 311.0f, 241.0f);
	Quaternion quaternion2(9.0f, 21.0f, 32.0f, 344.0f);
	Quaternion quaternion3(12.0f, 28.0f, 369.0f, 574.0f);

	UNITT_ASSERT(quaternion0 == Quaternion(1.0f, 2.0f, 3.0f, 4.0f));
	UNITT_ASSERT(quaternion1 != quaternion0);
	UNITT_ASSERT(quaternion0.x() == 1.0f);
	UNITT_ASSERT(quaternion0.y() == 2.0f);
	UNITT_ASSERT(quaternion0.z() == 3.0f);
	UNITT_ASSERT(quaternion0.w() == 4.0f);

	UNITT_ASSERT( equivalent(sqrtf(quaternion0.lengthSquared()), quaternion0.length()) );


	quaternion = 3.0 * quaternion;
	quaternion *= 3.0;

	quaternion = quaternion / 3.0;
	quaternion /= 3.0;

	quaternion.set(6.0f, 215.0f, 311.0f, 241.0f);

	quaternion = quaternion3;
	quaternion = quaternion2 / quaternion3;

	//std::cout << "Quaternion : " << quaternion << std::endl;

	Vector3D point(10.0, 0.0, 0.0);

	quaternion0.setAsRotation(degreesToRadians(90.0), 0.0, 0.0, 1.0);
	quaternion1.setAsRotation(degreesToRadians(90.0), 0.0, 1.0, 0.0);
	quaternion2.setAsRotation(degreesToRadians(90.0), 1.0, 0.0, 0.0);

	Vector3D vector_00;
	Real angle;
	quaternion0.getRotation(angle, vector_00);
	UNITT_ASSERT( equivalent(angle, degreesToRadians(90.0f)) );
	UNITT_ASSERT( isVectorEquivalent( vector_00 , Vector3D(0.0, 0.0, 1.0)) );

	quaternion1.getRotation(angle, vector_00);
	UNITT_ASSERT( equivalent(angle, degreesToRadians(90.0f)) );
	UNITT_ASSERT( isVectorEquivalent( vector_00 , Vector3D(0.0, 1.0, 0.0)) );

	quaternion2.getRotation(angle, vector_00);
	UNITT_ASSERT( equivalent(angle, degreesToRadians(90.0f)) );
	UNITT_ASSERT( isVectorEquivalent( vector_00 , Vector3D(1.0, 0.0, 0.0)) );


	quaternion = quaternion0;
	UNITT_ASSERT( isVectorEquivalent( point * quaternion , quaternion * point) );

	quaternion = quaternion0;
	UNITT_ASSERT( isVectorEquivalent(point * quaternion , Vector3D(0.0f, 10.0f, 0.0f)) );

	quaternion = quaternion1;
	UNITT_ASSERT( isVectorEquivalent(point * quaternion , Vector3D(0.0f, 0.0f, -10.0f)) );

	quaternion = quaternion2;
	UNITT_ASSERT( isVectorEquivalent(point * quaternion , Vector3D(10.0f, 0.0f, 0.0f)) );


	UNITT_ASSERT(equivalent(quaternion0.length(), 1.0f) );
	UNITT_ASSERT(equivalent(quaternion1.length(), 1.0f) );
	UNITT_ASSERT(equivalent(quaternion2.length(), 1.0f) );

	quaternion = quaternion0 * quaternion1;
	UNITT_ASSERT( isVectorEquivalent(point * quaternion , Vector3D(0.0f, 10.0f, 0.0f)) );

	quaternion = quaternion0 * quaternion1 * quaternion2;
	UNITT_ASSERT( isVectorEquivalent(point * quaternion ,  Vector3D(0.0f, 0.0f, 10.0f)) );

	quaternion = quaternion0 * quaternion1 * quaternion2 * quaternion1;
	UNITT_ASSERT( isVectorEquivalent(point * quaternion , point ) );


	quaternion = quaternion0 * quaternion1;
	UNITT_ASSERT( isVectorEquivalent(point * quaternion , Vector3D(0.0f, 10.0f, 0.0f) ) );
	//std::cout << " " << point << " -> " << point * quaternion << std::endl;


	//std::cout << " " << point << " -> " << point * quaternion << std::endl;
}

UNITT_TEST_END_CLASS(UnitTestQuaternion)


//
// UnitTestBoundingVolume
//

UNITT_TEST_BEGIN_CLASS(UnitTestBoundingVolume)

virtual void run()
{
	using namespace rengine;

	float x_min = -1.0f;
	float y_min = -2.0f;
	float z_min = -3.0f;
	float x_max = 1.0f;
	float y_max = 2.0f;
	float z_max = 3.0f;

	BoundingBox bounding_box0(x_min, y_min, z_min, x_max, y_max, z_max);
	UNITT_ASSERT(bounding_box0.xMin() == x_min);
	UNITT_ASSERT(bounding_box0.yMin() == y_min);
	UNITT_ASSERT(bounding_box0.zMin() == z_min);
	UNITT_ASSERT(bounding_box0.xMax() == x_max);
	UNITT_ASSERT(bounding_box0.yMax() == y_max);
	UNITT_ASSERT(bounding_box0.zMax() == z_max);
	UNITT_ASSERT(bounding_box0.minimum() == Vector3D(x_min, y_min, z_min));
	UNITT_ASSERT(bounding_box0.maximum() == Vector3D(x_max, y_max, z_max));

	y_max = 4.0f;

	bounding_box0.set(x_min, y_min, z_min, x_max, y_max, z_max);
	UNITT_ASSERT(bounding_box0.xMin() == x_min);
	UNITT_ASSERT(bounding_box0.yMin() == y_min);
	UNITT_ASSERT(bounding_box0.zMin() == z_min);
	UNITT_ASSERT(bounding_box0.xMax() == x_max);
	UNITT_ASSERT(bounding_box0.yMax() == y_max);
	UNITT_ASSERT(bounding_box0.zMax() == z_max);
	UNITT_ASSERT(bounding_box0.minimum() == Vector3D(x_min, y_min, z_min));
	UNITT_ASSERT(bounding_box0.maximum() == Vector3D(x_max, y_max, z_max));


	x_min = -10.0f;
	y_min = -20.0f;
	z_min = -30.0f;
	x_max = 10.0f;
	y_max = 20.0f;
	z_max = 30.0f;

	bounding_box0.merge(Vector3D(x_min, y_min, z_min));
	bounding_box0.merge(Vector3D(x_max, y_max, z_max));
	UNITT_ASSERT(bounding_box0.xMin() == x_min);
	UNITT_ASSERT(bounding_box0.yMin() == y_min);
	UNITT_ASSERT(bounding_box0.zMin() == z_min);
	UNITT_ASSERT(bounding_box0.xMax() == x_max);
	UNITT_ASSERT(bounding_box0.yMax() == y_max);
	UNITT_ASSERT(bounding_box0.zMax() == z_max);
	UNITT_ASSERT(bounding_box0.minimum() == Vector3D(x_min, y_min, z_min));
	UNITT_ASSERT(bounding_box0.maximum() == Vector3D(x_max, y_max, z_max));

	x_min = -5.0f;
	y_min = -21.0f;
	z_min = 0.0f;
	x_max = 1.0f;
	y_max = 22.0f;
	z_max = 32.0f;

	bounding_box0.merge(Vector3D(x_min, y_min, z_min));
	bounding_box0.merge(Vector3D(x_max, y_max, z_max));

	UNITT_ASSERT(bounding_box0.xMin() != x_min);
	UNITT_ASSERT(bounding_box0.yMin() == y_min);
	UNITT_ASSERT(bounding_box0.zMin() != z_min);
	UNITT_ASSERT(bounding_box0.xMax() != x_max);
	UNITT_ASSERT(bounding_box0.yMax() == y_max);
	UNITT_ASSERT(bounding_box0.zMax() == z_max);
}

UNITT_TEST_END_CLASS(UnitTestBoundingVolume)
