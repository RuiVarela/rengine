// __!!rengine_copyright!!__ //

#ifndef __RENGINE_BASE_SHAPES_H__
#define __RENGINE_BASE_SHAPES_H__

#include <rengine/geometry/Mesh.h>

namespace rengine
{
	//
	// Quadrilateral
	//
	class Quadrilateral : public Mesh
	{
	public:
		Quadrilateral();
		Quadrilateral(Vector3D bottom_left, Vector3D top_right);
		Quadrilateral(Vector3D bottom_left_vertex, Vector3D top_right_vertex,
				      Vector2D bottom_left_texture_coordinate, Vector2D top_right_texture_coordinate);
		~Quadrilateral();

		void setCornersVertex(Vector3D bottom_left, Vector3D top_right);
		Vector3D bottomLeftVertex() const;
		Vector3D topRightVertex() const;

		void setCornersTextureCoordinates(Vector2D bottom_left, Vector2D top_right);
		Vector2D bottomLeftTextureCoordinate() const;
		Vector2D topRightTextureCoordinate() const;
	};

	//
	// Box
	//
	class Box : public Mesh
	{
	public:
		Box();
		Box(Vector3D const& center, Real const length);
		Box(Vector3D const& center, Vector3D const& length);
		Box(Vector3D const& length);
		~Box();

		void computeGeometry();

		void setCenter(Vector3D const& center);
		Vector3D const& getCenter() const;

		void setLenght(Vector3D const& length);
		Vector3D const getLength() const;
	private:
		Vector3D center_;
		Vector3D length_;
	};

	//
	// Sphere
	//
	class Sphere : public Mesh
	{
	public:
		static const Uint default_slices = 8;
		static const Uint default_stacks = 5;

		Sphere();
		Sphere(Vector3D const& center, Real const radius);
		Sphere(Real const radius);
		Sphere(Vector3D const& center, Real const radius, Uint const slices, Uint const stacks);
		Sphere(Real const radius, Uint const slices, Uint const stacks);
		~Sphere();

		void computeGeometry();

		void setRadius(Real const radius);
		Real const& getRadius() const;

		void setSlices(Uint const slices);
		Uint const& getSlices() const;

		void setStacks(Uint const stacks);
		Uint const& getStacks() const;

		void setCenter(Vector3D const& center);
		Vector3D const& getCenter() const;
	private:
		Real radius_;
		Uint slices_;
		Uint stacks_;
		Vector3D center_;
	};

	//
	// Cylinder
	//
	class Cylinder : public Mesh
	{
	public:
		static const Uint default_slices = 10;
		static const Uint default_stacks = 3;

		Cylinder();
		Cylinder(Vector3D const& center, Real const radius, Real const height);
		Cylinder(Real const radius, Real const height);
		Cylinder(Real const top_radius, Real const bottom_radius, Real const height);
		Cylinder(Vector3D const& center, Real const top_radius, Real const bottom_radius, Real const height);
		Cylinder(Vector3D const& center, Real const top_radius, Real const bottom_radius, Real const height, Uint const slices, Uint const stacks);
		~Cylinder();

		void computeGeometry();

		void setTopRadius(Real const radius);
		Real const& getTopRadius() const;

		void setBottomRadius(Real const radius);
		Real const& getBottomRadius() const;

		void setHeight(Real const height);
		Real const& getHeight() const;

		void setSlices(Uint const slices);
		Uint const& getSlices() const;

		void setStacks(Uint const stacks);
		Uint const& getStacks() const;

		void setCenter(Vector3D const& center);
		Vector3D const& getCenter() const;
	private:
		Real bottom_radius_;
		Real top_radius_;
		Real height_;
		Uint slices_;
		Uint stacks_;
		Vector3D center_;
	};

	//
	// Cone
	//
	class Cone : public Mesh
	{
	public:
		static const Uint default_slices = 10;
		static const Uint default_stacks = 3;

		Cone();
		Cone(Vector3D const& center, Real const radius, Real const height);
		Cone(Real const radius, Real const height);
		Cone(Vector3D const& center, Real const radius, Real const height, Uint const slices, Uint const stacks);
		~Cone();

		void computeGeometry();

		void setRadius(Real const radius);
		Real const& getRadius() const;

		void setHeight(Real const height);
		Real const& getHeight() const;

		void setSlices(Uint const slices);
		Uint const& getSlices() const;

		void setStacks(Uint const stacks);
		Uint const& getStacks() const;

		void setCenter(Vector3D const& center);
		Vector3D const& getCenter() const;
	private:
		Real radius_;
		Real height_;
		Uint slices_;
		Uint stacks_;
		Vector3D center_;
	};

	//
	// Torus
	//

	class Torus : public Mesh
	{
	public:
		static const Uint default_rings = 10;
		static const Uint default_sides = 8;

		Torus();
		Torus(Vector3D const& center, Real const inner_radius, Real const outer_radius);
		Torus(Real const inner_radius, Real const outer_radius);
		Torus(Vector3D const& center, Real const inner_radius, Real const outer_radius, Uint const rings, Uint const sides);
		~Torus();

		void computeGeometry();

		void setInnerRadius(Real const radius);
		Real const& getInnerRadius() const;

		void setOuterRadius(Real const radius);
		Real const& getOuterRadius() const;

		void setSides(Uint const sides);
		Uint const& getSides() const;

		void setRings(Uint const rings);
		Uint const& getRings() const;

		void setCenter(Vector3D const& center);
		Vector3D const& getCenter() const;
	private:
		Real inner_radius_;
		Real outer_radius_;
		Uint sides_;
		Uint rings_;
		Vector3D center_;
	};

	//
	// Capsule
	//

	class Capsule : public Mesh
	{
	public:
		static const Uint default_slices = 10;
		static const Uint default_stacks = 8;

		Capsule();
		Capsule(Vector3D const& center, Real const radius, Real const height);
		Capsule(Real const radius, Real const height);
		Capsule(Vector3D const& center, Real const radius, Real const height, Uint const slices, Uint const stacks);
		~Capsule();

		void computeGeometry();

		void setRadius(Real const radius);
		Real const& getRadius() const;

		void setHeight(Real const height);
		Real const& getHeight() const;

		void setSlices(Uint const slices);
		Uint const& getSlices() const;

		void setStacks(Uint const stacks);
		Uint const& getStacks() const;

		void setCenter(Vector3D const& center);
		Vector3D const& getCenter() const;
	private:
		Real radius_;
		Real height_;
		Uint slices_;
		Uint stacks_;
		Vector3D center_;
	};


	//
	// Implementation
	//

	//
	// Box
	//
	RENGINE_INLINE void Box::setCenter(Vector3D const& center)
	{
		center_ = center;
	}

	RENGINE_INLINE Vector3D const& Box::getCenter() const
	{
		return center_;
	}

	RENGINE_INLINE void Box::setLenght(Vector3D const& length)
	{
		length_ = length;
	}

	RENGINE_INLINE Vector3D const Box::getLength() const
	{
		return length_;
	}

	//
	// Sphere
	//
	RENGINE_INLINE void Sphere::setRadius(Real const radius)
	{
		radius_ = radius;
	}

	RENGINE_INLINE Real const& Sphere::getRadius() const
	{
		return radius_;
	}

	RENGINE_INLINE void Sphere::setSlices(Uint const slices)
	{
		slices_ = slices;
	}

	RENGINE_INLINE Uint const& Sphere::getSlices() const
	{
		return slices_;
	}

	RENGINE_INLINE void Sphere::setStacks(Uint const stacks)
	{
		stacks_ = stacks;
	}

	RENGINE_INLINE Uint const& Sphere::getStacks() const
	{
		return stacks_;
	}

	RENGINE_INLINE void Sphere::setCenter(Vector3D const& center)
	{
		center_ = center;
	}

	RENGINE_INLINE Vector3D const& Sphere::getCenter() const
	{
		return center_;
	}

	//
	// Cylinder
	//
	RENGINE_INLINE void Cylinder::setTopRadius(Real const radius)
	{
		top_radius_ = radius;
	}

	RENGINE_INLINE Real const& Cylinder::getTopRadius() const
	{
		return top_radius_;
	}

	RENGINE_INLINE void Cylinder::setBottomRadius(Real const radius)
	{
		bottom_radius_ = radius;
	}

	RENGINE_INLINE Real const& Cylinder::getBottomRadius() const
	{
		return bottom_radius_;
	}

	RENGINE_INLINE void Cylinder::setHeight(Real const height)
	{
		height_ = height;
	}

	RENGINE_INLINE Real const& Cylinder::getHeight() const
	{
		return height_;
	}

	RENGINE_INLINE void Cylinder::setSlices(Uint const slices)
	{
		slices_ = slices;
	}

	RENGINE_INLINE Uint const& Cylinder::getSlices() const
	{
		return slices_;
	}

	RENGINE_INLINE void Cylinder::setStacks(Uint const stacks)
	{
		stacks_ = stacks;
	}

	RENGINE_INLINE Uint const& Cylinder::getStacks() const
	{
		return stacks_;
	}

	RENGINE_INLINE void Cylinder::setCenter(Vector3D const& center)
	{
		center_ = center;
	}

	RENGINE_INLINE Vector3D const& Cylinder::getCenter() const
	{
		return center_;
	}

	//
	// Cone
	//
	RENGINE_INLINE void Cone::setRadius(Real const radius)
	{
		radius_ = radius;
	}

	RENGINE_INLINE Real const& Cone::getRadius() const
	{
		return radius_;
	}

	RENGINE_INLINE void Cone::setHeight(Real const height)
	{
		height_ = height;
	}

	RENGINE_INLINE Real const& Cone::getHeight() const
	{
		return height_;
	}

	RENGINE_INLINE void Cone::setSlices(Uint const slices)
	{
		slices_ = slices;
	}

	RENGINE_INLINE Uint const& Cone::getSlices() const
	{
		return slices_;
	}

	RENGINE_INLINE void Cone::setStacks(Uint const stacks)
	{
		stacks_ = stacks;
	}

	RENGINE_INLINE Uint const& Cone::getStacks() const
	{
		return stacks_;
	}

	RENGINE_INLINE void Cone::setCenter(Vector3D const& center)
	{
		center_ = center;
	}

	RENGINE_INLINE Vector3D const& Cone::getCenter() const
	{
		return center_;
	}

	//
	// Torus
	//
	RENGINE_INLINE void Torus::setInnerRadius(Real const radius)
	{
		inner_radius_ = radius;
	}

	RENGINE_INLINE Real const& Torus::getInnerRadius() const
	{
		return inner_radius_;
	}

	RENGINE_INLINE void Torus::setOuterRadius(Real const radius)
	{
		outer_radius_ = radius;
	}

	RENGINE_INLINE Real const& Torus::getOuterRadius() const
	{
		return outer_radius_;
	}

	RENGINE_INLINE void Torus::setSides(Uint const sides)
	{
		sides_ = sides;
	}

	RENGINE_INLINE Uint const& Torus::getSides() const
	{
		return sides_;
	}

	RENGINE_INLINE void Torus::setRings(Uint const rings)
	{
		rings_ = rings;
	}

	RENGINE_INLINE Uint const& Torus::getRings() const
	{
		 return rings_;
	}

	RENGINE_INLINE void Torus::setCenter(Vector3D const& center)
	{
		center_ = center;
	}

	RENGINE_INLINE Vector3D const& Torus::getCenter() const
	{
		return center_;
	}

	//
	// Capsule
	//
	RENGINE_INLINE void Capsule::setRadius(Real const radius)
	{
		radius_ = radius;
	}

	RENGINE_INLINE Real const& Capsule::getRadius() const
	{
		return radius_;
	}

	RENGINE_INLINE void Capsule::setHeight(Real const height)
	{
		height_ = height;
	}

	RENGINE_INLINE Real const& Capsule::getHeight() const
	{
		return height_;
	}

	RENGINE_INLINE void Capsule::setSlices(Uint const slices)
	{
		slices_ = slices;
	}

	RENGINE_INLINE Uint const& Capsule::getSlices() const
	{
		return slices_;
	}

	RENGINE_INLINE void Capsule::setStacks(Uint const stacks)
	{
		stacks_ = stacks;
	}

	RENGINE_INLINE Uint const& Capsule::getStacks() const
	{
		return stacks_;
	}

	RENGINE_INLINE void Capsule::setCenter(Vector3D const& center)
	{
		center_ = center;
	}

	RENGINE_INLINE Vector3D const& Capsule::getCenter() const
	{
		return center_;
	}
}

#endif //__RENGINE_BASE_SHAPES_H__
