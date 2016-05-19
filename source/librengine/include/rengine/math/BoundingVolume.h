// __!!rengine_copyright!!__ //

#ifndef __RENGINE_BOUNDING_VOLUME_H__
#define __RENGINE_BOUNDING_VOLUME_H__

#include <rengine/math/Vector.h>
#include <rengine/lang/debug/Debug.h>
#include <limits>

namespace rengine
{
	class BoundingBox
	{
	public:
		typedef Real ValueType;
		typedef Vector3<ValueType> VectorType;

		//    2-----3
		//   /|    /|
		//  / |   / |
		// 6-----7  |
		// |  0--|--1
		// | /   | /
		// |/    |/
		// 4-----5

		enum BoxCorner           //ZYX
		{
			FarLeftBottom   = 0, //000
			FarRightBottom  = 1, //001
			FarLeftTop      = 2, //010
			FarRightTop     = 3, //011

			NearLeftBottom  = 4, //100
			NearRightBottom = 5, //101
			NearLeftTop     = 6, //110
			NearRightTop    = 7  //111
		};

		BoundingBox()
		{
			reset();
		}

		BoundingBox(
			ValueType const min_x, ValueType const min_y, ValueType const min_z,
			ValueType const max_x, ValueType const max_y, ValueType const max_z)
			:min_(min_x, min_y, min_z), max_(max_x, max_y, max_z)
		{}

		BoundingBox(VectorType const& min, VectorType const& max)
			:min_(min), max_(max)
		{}

		void reset()
		{
			min_.set(
				std::numeric_limits<ValueType>::max(),
				std::numeric_limits<ValueType>::max(),
				std::numeric_limits<ValueType>::max()
				);

			max_.set(
				std::numeric_limits<ValueType>::min(),
				std::numeric_limits<ValueType>::min(),
				std::numeric_limits<ValueType>::min()
				);
		}

		void set(
			ValueType const min_x, ValueType const min_y, ValueType const min_z,
			ValueType const max_x, ValueType const max_y, ValueType const max_z)
		{
			min_.set(min_x, min_y, min_z);
			max_.set(max_x, max_y, max_z);
		}

		void set(VectorType const& min, VectorType const& max)
		{
			min_ = min;
			max_ = max;
		}

		Bool isValid() const
		{
			return ( (min_.x() <= max_.x()) && (min_.y() <= max_.y()) && (min_.z() <= max_.z()) );
		}

		VectorType minimum() const
		{
			return min_;
		}

		VectorType maximum() const
		{
			return max_;
		}

		ValueType xMin() const
		{
			return min_.x();
		}

		ValueType yMin() const
		{
			return min_.y();
		}

		ValueType zMin() const
		{
			return min_.z();
		}

		ValueType xMax() const
		{
			return max_.x();
		}

		ValueType yMax() const
		{
			return max_.y();
		}

		ValueType zMax() const
		{
			return max_.z();
		}

		VectorType extent() const
		{
			return max_ - min_;
		}

		VectorType center() const
		{
			return (min_ + max_) * ValueType(0.5);
		}

		ValueType radius() const
		{
			return (extent() * ValueType(0.5)).length();
		}

		ValueType area() const
		{
			VectorType diagonal( extent() );
			return ValueType(2.0) * (diagonal.x() * diagonal.y() + diagonal.y() * diagonal.z() + diagonal.x() * diagonal.z());
		}

		ValueType volume() const
		{
			VectorType diagonal( extent() );
			return diagonal.x() * diagonal.y() * diagonal.z();
		}

		VectorType corner(Uint corner_code) const
        {
            return VectorType(
					corner_code & 1 ? max_.x() : min_.x(),
					corner_code & 2 ? max_.y() : min_.y(),
					corner_code & 4 ? max_.z() : min_.z()
					);
        }

		void merge(ValueType const x, ValueType const y, ValueType const z)
		{
			if(x < min_.x()) { min_.x() = x; }
			if(x > max_.x()) { max_.x() = x; }

            if(y < min_.y()) { min_.y() = y; }
            if(y > max_.y()) { max_.y() = y; }

			if(z < min_.z()) { min_.z() = z; }
            if(z > max_.z()) { max_.z() = z; }
		}

		void merge(VectorType const& v)
		{
			merge(v.x(), v.y(), v.z());
		}

		void merge(BoundingBox const& bounding_box)
		{
			RENGINE_ASSERT(isValid());
			RENGINE_ASSERT(bounding_box.isValid());

			if(bounding_box.xMin() < min_.x()) { min_.x() = bounding_box.xMin(); }
			if(bounding_box.xMax() > max_.x()) { max_.x() = bounding_box.xMax(); }

			if(bounding_box.yMin() < min_.y()) { min_.y() = bounding_box.yMin(); }
			if(bounding_box.yMax() > max_.y()) { max_.y() = bounding_box.yMax(); }

			if(bounding_box.zMin() < min_.z()) { min_.z() = bounding_box.zMin(); }
			if(bounding_box.zMax() > max_.z()) { max_.z() = bounding_box.zMax(); }
		}

		Bool contains(VectorType const& v) const
		{
			RENGINE_ASSERT(isValid());

			return (
				(v.x() >= min_.x() && v.x() <= max_.x()) &&
				(v.y() >= min_.y() && v.y() <= max_.y()) &&
				(v.z() >= min_.z() && v.z() <= max_.z())
				);
		}

		Bool contains(BoundingBox const& bounding_box) const
		{
			RENGINE_ASSERT(isValid());
			RENGINE_ASSERT(bounding_box.isValid());

			return (
				(bounding_box.xMin() >= min_.x() && bounding_box.xMax() <= max_.x()) &&
				(bounding_box.yMin() >= min_.y() && bounding_box.yMax() <= max_.y()) &&
				(bounding_box.zMin() >= min_.z() && bounding_box.zMax() <= max_.z())
				);
		}

		Bool intersects(BoundingBox const& bounding_box) const
		{
			return (
				(rengine::maximum(xMin(), bounding_box.xMin()) <= rengine::minimum(xMax(), bounding_box.xMax())) &&
				(rengine::maximum(yMin(), bounding_box.yMin()) <= rengine::minimum(yMax(), bounding_box.yMax())) &&
				(rengine::maximum(zMin(), bounding_box.zMin()) <= rengine::minimum(zMax(), bounding_box.zMax()))
			);
		}

		BoundingBox intersection(BoundingBox const& bounding_box) const
		{
			return BoundingBox(
				rengine::maximum(xMin(), bounding_box.xMin()),
				rengine::maximum(yMin(), bounding_box.yMin()),
				rengine::maximum(zMin(), bounding_box.zMin()),

				rengine::minimum(xMax(), bounding_box.xMax()),
				rengine::minimum(yMax(), bounding_box.yMax()),
				rengine::minimum(zMax(), bounding_box.zMax())
			 );
		}

	private:
		VectorType min_;
		VectorType max_;
	};

} //namespace rengine


#endif //__RENGINE_BOUNDING_VOLUME_H__
