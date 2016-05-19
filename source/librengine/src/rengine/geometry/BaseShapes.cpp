// __!!rengine_copyright!!__ //

#include <rengine/geometry/BaseShapes.h>
#include <rengine/geometry/VertexDeclaration.h>
#include <rengine/math/Math.h>


#define SHAPE_RENGINE_ASSERT_VECTOR_POS(vector, index) RENGINE_ASSERT((index) >= 0); RENGINE_ASSERT((index) < vector.size());
namespace rengine
{
	Quadrilateral::Quadrilateral()
	{
		addChannel(VertexBuffer::Position);
		addChannel(VertexBuffer::Normal);
		addChannel(VertexBuffer::TexCoords);
		construct(4);

		setCornersVertex(Vector3D(-0.5f, -0.5f, 0.0f), Vector3D(0.5f, 0.5f, 0.0f));
		setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
	}

	Quadrilateral::Quadrilateral(Vector3D bottom_left, Vector3D top_right)
	{
		addChannel(VertexBuffer::Position);
		addChannel(VertexBuffer::Normal);
		addChannel(VertexBuffer::TexCoords);
		construct(4);

		setCornersVertex(bottom_left, top_right);
		setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
	}

	Quadrilateral::Quadrilateral(Vector3D bottom_left_vertex, Vector3D top_right_vertex,
								 Vector2D bottom_left_texture_coordinate, Vector2D top_right_texture_coordinate)
	{
		addChannel(VertexBuffer::Position);
		addChannel(VertexBuffer::Normal);
		addChannel(VertexBuffer::TexCoords);
		construct(4);

		setCornersVertex(bottom_left_vertex, top_right_vertex);
		setCornersTextureCoordinates(bottom_left_texture_coordinate, top_right_texture_coordinate);
	}

	Quadrilateral::~Quadrilateral()
	{

	}

	void Quadrilateral::setCornersVertex(Vector3D bottom_left, Vector3D top_right)
	{
		index().clear();

		VertexBuffer::Interface<PositionNormalTextureVertexDeclaration> vertex_stream = interface<PositionNormalTextureVertexDeclaration>();

		Vector3D bottom_to_top = top_right - bottom_left;
		Vector3D plane_normal(0.0f, 1.0f, 0.0f);
		Vector3D projection = bottom_to_top - (bottom_to_top * plane_normal) * plane_normal;

		Vector3D bottom_right = bottom_left + projection;
		Vector3D top_left = top_right - projection;

		vertex_stream[0].position = bottom_left;
		vertex_stream[1].position = bottom_right;
		vertex_stream[2].position = top_right;
		vertex_stream[3].position = top_left;

		index().push_back(0);
		index().push_back(1);
		index().push_back(2);

		index().push_back(2);
		index().push_back(3);
		index().push_back(0);

		Vector3D bottom_vector = bottom_right - bottom_left;
		Vector3D top_vector = top_right - bottom_right;
		Vector3D normal_vector = bottom_vector ^ top_vector;

		normal_vector = Vector3D(0.0f, 0.0f,1.0f);

		vertex_stream[0].normal = normal_vector;
		vertex_stream[1].normal = normal_vector;
		vertex_stream[2].normal = normal_vector;
		vertex_stream[3].normal = normal_vector;

		setNeedsDataRefresh();
	}

	Vector3D Quadrilateral::bottomLeftVertex() const
	{
		RENGINE_ASSERT(size() == 4);
		VertexBuffer::ConstInterface<PositionNormalTextureVertexDeclaration> vertex_stream = constInterface<PositionNormalTextureVertexDeclaration>();
		return vertex_stream[0].position;
	}

	Vector3D Quadrilateral::topRightVertex() const
	{
		RENGINE_ASSERT(size() == 4);
		VertexBuffer::ConstInterface<PositionNormalTextureVertexDeclaration> vertex_stream = constInterface<PositionNormalTextureVertexDeclaration>();
		return vertex_stream[2].position;
	}

	void Quadrilateral::setCornersTextureCoordinates(Vector2D bottom_left, Vector2D top_right)
	{
		VertexBuffer::Interface<PositionNormalTextureVertexDeclaration> vertex_stream = interface<PositionNormalTextureVertexDeclaration>();

		Vector2D bottom_right(top_right.x(), bottom_left.y());
		Vector2D top_left(bottom_left.x(), top_right.y());

		vertex_stream[0].tex_coord = bottom_left;
		vertex_stream[1].tex_coord = bottom_right;
		vertex_stream[2].tex_coord = top_right;
		vertex_stream[3].tex_coord = top_left;

		setNeedsDataRefresh();
	}

	Vector2D Quadrilateral::bottomLeftTextureCoordinate() const
	{
		RENGINE_ASSERT(size() == 4);
		VertexBuffer::ConstInterface<PositionNormalTextureVertexDeclaration> vertex_stream = constInterface<PositionNormalTextureVertexDeclaration>();
		return vertex_stream[0].tex_coord;
	}

	Vector2D Quadrilateral::topRightTextureCoordinate() const
	{
		RENGINE_ASSERT(size() == 4);
		VertexBuffer::ConstInterface<PositionNormalTextureVertexDeclaration> vertex_stream = constInterface<PositionNormalTextureVertexDeclaration>();
		return vertex_stream[2].tex_coord;
	}

	//
	// Box
	//

	Box::Box() :
		length_(1.0f, 1.0f, 1.0f)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Box::Box(Vector3D const& center, Real const length) :
		center_(center), length_(length, length, length)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Box::Box(Vector3D const& center, Vector3D const& length) :
		center_(center), length_(length)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Box::Box(Vector3D const& length) :
		length_(length)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Box::~Box()
	{
	}

	void Box::computeGeometry()
	{
	    Real const dx = length_.x() * 0.5f;
	    Real const dy = length_.y() * 0.5f;
	    Real const dz = length_.z() * 0.5f;
	    Real const cx = center_.x();
	    Real const cy = center_.y();
	    Real const cz = center_.z();
	    Real const n = 0.577350259f;

		clear();
		index().clear();

		PositionNormalVertexDeclaration vertex;
		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();

		Vector3D normal_vector(-1.0f, -1.0f, 1.0f);

		vertex.position = Vector3D(cx - dx, cy - dy, cz + dz);
		vertex.normal = Vector3D(-n, -n, n);
		vertex_stream.add(vertex);

		vertex.position = Vector3D(cx + dx, cy - dy, cz + dz);
		vertex.normal = Vector3D(n, -n, n);
		vertex_stream.add(vertex);

		vertex.position = Vector3D(cx + dx, cy + dy, cz + dz);
		vertex.normal = Vector3D(n, n, n);
		vertex_stream.add(vertex);

		vertex.position = Vector3D(cx - dx, cy + dy, cz + dz);
		vertex.normal = Vector3D(-n, n, n);
		vertex_stream.add(vertex);

		vertex.position = Vector3D(cx - dx, cy - dy, cz - dz);
		vertex.normal = Vector3D(-n, -n, -n);
		vertex_stream.add(vertex);

		vertex.position = Vector3D(cx - dx, cy + dy, cz - dz);
		vertex.normal = Vector3D(-n, n, -n);
		vertex_stream.add(vertex);

		vertex.position = Vector3D(cx + dx, cy + dy, cz - dz);
		vertex.normal = Vector3D(n, n, -n);
		vertex_stream.add(vertex);

		vertex.position = Vector3D(cx + dx, cy - dy, cz - dz);
		vertex.normal = Vector3D(n, -n, -n);
		vertex_stream.add(vertex);

		//front (0, 1, 2 ,3)
		index().push_back(0);
		index().push_back(1);
		index().push_back(2);
		index().push_back(2);
		index().push_back(3);
		index().push_back(0);

		//right (1, 7, 6, 2)
		index().push_back(1);
		index().push_back(7);
		index().push_back(6);
		index().push_back(6);
		index().push_back(2);
		index().push_back(1);

		//back (4, 5, 6, 7)
		index().push_back(4);
		index().push_back(5);
		index().push_back(6);
		index().push_back(6);
		index().push_back(7);
		index().push_back(4);

		//left (0, 3, 5, 4)
		index().push_back(0);
		index().push_back(3);
		index().push_back(5);
		index().push_back(5);
		index().push_back(4);
		index().push_back(0);

		//top (3, 2, 6, 5)
		index().push_back(3);
		index().push_back(2);
		index().push_back(6);
		index().push_back(6);
		index().push_back(5);
		index().push_back(3);

		//bottom (4, 7, 1, 0)
		index().push_back(4);
		index().push_back(7);
		index().push_back(1);
		index().push_back(1);
		index().push_back(0);
		index().push_back(4);
	}

	//
	// Shere
	//
	Sphere::Sphere() :
		radius_(1.0f), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalTextureVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Sphere::Sphere(Vector3D const& center, Real const radius) :
		radius_(radius), slices_(default_slices), stacks_(default_stacks), center_(center)
	{
		PositionNormalTextureVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Sphere::Sphere(Real const radius) :
		radius_(radius), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalTextureVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Sphere::Sphere(Vector3D const& center, Real const radius, Uint const slices, Uint const stacks) :
		radius_(radius), slices_(slices), stacks_(stacks), center_(center)
	{
		PositionNormalTextureVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Sphere::Sphere(Real const radius, Uint const slices, Uint const stacks) :
		radius_(radius), slices_(slices), stacks_(stacks)
	{
		PositionNormalTextureVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Sphere::~Sphere()
	{
	}

	void Sphere::computeGeometry()
	{
		RENGINE_ASSERT(slices_ > 2);
		RENGINE_ASSERT(stacks_ > 2);

		clear();
		index().clear();

		PositionNormalTextureVertexDeclaration vertex;
		VertexBuffer::Interface<PositionNormalTextureVertexDeclaration> vertex_stream = interface<PositionNormalTextureVertexDeclaration>();

	    Real const cx = center_.x();
	    Real const cy = center_.y();
	    Real const cz = center_.z();

		Real x = 0.0f;
		Real y = 0.0f;
		Real z = 0.0f;

		Real theta = 0.0f;
		Real rho = 0.0f;

		Real dtheta = pi_mul_2_angle / Real(slices_);
		Real drho = pi_angle / Real(stacks_);

		Uint base_index = 0;

		Real t = 0.0f;
		Real s = 0.0f;

		Real dt = 1.0f / stacks_;
		Real ds = 1.0f / slices_;


		//
		// Sphere Z+ Section
		//
		vertex.position = Vector3D(cx, cy, cz + radius_);
		vertex.normal = Vector3D(0.0f, 0.0f, 1.0f);

		for (Uint slice = 0; slice != slices_ + 1; ++slice)
		{
			vertex.tex_coord = Vector2D(1.0f - s, t);
			vertex_stream.add(vertex);
			s += ds;
		}

		base_index = Uint (size());

		rho = 1 * drho;
		t += dt;
		s = 0.0f;
		for (Uint slice = 0; slice != slices_; ++slice)
		{
			theta = slice * dtheta;

			x = -sin(theta) * sin(rho);
			y = cos(theta) * sin(rho);
			z = cos(rho);

			vertex.position = Vector3D(cx + x * radius_, cy + y * radius_, cz + z * radius_);
			vertex.normal = Vector3D(x, y, z );
			vertex.tex_coord = Vector2D(1.0f - s, t);
			vertex_stream.add(vertex);

			if (slice > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

				index().push_back(base_index - (slices_ + 1) + (slice - 1));
				index().push_back(base_index + slice - 1);
				index().push_back(base_index + slice);

				index().push_back(base_index + slice);
				index().push_back(base_index - (slices_ + 1) + slice);
				index().push_back(base_index - (slices_ + 1) + (slice - 1));
			}
			s += ds;
		}

		// Repeat the first slice to avoid texturing problems
		vertex = vertex_stream[base_index];
		vertex.tex_coord = Vector2D(1.0f - s, t);
		vertex_stream.add(vertex);

		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 2);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - (slices_ + 1) + slices_);

		index().push_back(base_index - 2);
		index().push_back(base_index + slices_ - 1);
		index().push_back(base_index + slices_);

		index().push_back(base_index + slices_);
		index().push_back(base_index - (slices_ + 1) + slices_);
		index().push_back(base_index - 2);



		//
		// Draw intermediate Section
		//
		Uint middle_stacks = stacks_ - 1;
		//t = 0.0f;
		for (Uint stack = 0; stack != middle_stacks; ++stack)
		{
			rho = (stack + 1) * drho;
			base_index = Uint (size());
			s = 0.0f;

			for (Uint slice = 0; slice != slices_; ++slice)
			{
				theta = slice * dtheta;

				x = -sin(theta) * sin(rho);
				y = cos(theta) * sin(rho);
				z = cos(rho);

				vertex.position = Vector3D(cx + x * radius_, cy + y * radius_, cz + z * radius_);
				vertex.normal = Vector3D(x, y, z);
				vertex.tex_coord = Vector2D(1.0f - s, t);
				vertex_stream.add(vertex);

				if ((slice > 0) && (stack > 0))
				{
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - (slices_ + 1) + (slice - 1));
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - (slices_ + 1) + slice);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

					index().push_back(base_index - (slices_ + 1) + (slice - 1));
					index().push_back(base_index + slice - 1);
					index().push_back(base_index + slice);

					index().push_back(base_index + slice);
					index().push_back(base_index - (slices_ + 1) + slice);
					index().push_back(base_index - (slices_ + 1) + (slice - 1));
				}
				s += ds;
			}

			// Repeat the first slice to avoid texturing problems
			vertex = vertex_stream[base_index];
			vertex.tex_coord = Vector2D(1.0f - s, t);
			vertex_stream.add(vertex);
			if (stack > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 2);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - (slices_ + 1) + slices_);

				index().push_back(base_index - 2);
				index().push_back(base_index + slices_ - 1);
				index().push_back(base_index + slices_);

				index().push_back(base_index + slices_);
				index().push_back(base_index - (slices_ + 1) + slices_);
				index().push_back(base_index - 2);
			}

			t += dt;
		}



		t = 1.0f;
		s = 0.0f;
		//
		// Sphere Z- Section
		//
		base_index = Uint (size());
		vertex.position = Vector3D(cx, cy, cz - radius_);
		vertex.normal = Vector3D(0.0f, 0.0f, -1.0f);

		for (Uint slice = 0; slice != slices_ + 1; ++slice)
		{
			vertex.tex_coord = Vector2D(1.0f - s, t);
			vertex_stream.add(vertex);
			s += ds;
		}


		base_index = Uint (size());
		t = 1.0f - dt;
		s = 0.0f;
		rho = pi_angle - drho;

		for (Uint slice = 0; slice != slices_; ++slice)
		{
			theta = slice * dtheta;

			x = -sin(theta) * sin(rho);
			y = cos(theta) * sin(rho);
			z = cos(rho);

			vertex.position = Vector3D(cx + x * radius_, cy + y * radius_, cz + z * radius_);
			vertex.normal = Vector3D(x, y, z );
			vertex.tex_coord = Vector2D(1.0f - s, t);
			vertex_stream.add(vertex);

			if (slice > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

				index().push_back(base_index + slice);
				index().push_back(base_index + slice - 1);
				index().push_back(base_index - (slices_ + 1) + (slice - 1));

				index().push_back(base_index - (slices_ + 1) + (slice - 1));
				index().push_back(base_index - (slices_ + 1) + slice);
				index().push_back(base_index + slice);
			}
			s += ds;
		}

		// Repeat the first slice to avoid texturing problems
		vertex = vertex_stream[base_index];
		vertex.tex_coord = Vector2D(1.0f - s, t);
		vertex_stream.add(vertex);

		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 2);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - (slices_ + 1) + slices_);

		index().push_back(base_index + slices_);
		index().push_back(base_index + slices_ - 1);
		index().push_back(base_index - 2);

		index().push_back(base_index - 2);
		index().push_back(base_index - (slices_ + 1) + slices_);
		index().push_back(base_index + slices_);

	}

	//
	// Cylinder
	//

	Cylinder::Cylinder() :
		bottom_radius_(0.2f), top_radius_(0.2f), height_(1.0f), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cylinder::Cylinder(Vector3D const& center, Real const radius, Real const height) :
		bottom_radius_(radius), top_radius_(radius), height_(height), slices_(default_slices), stacks_(default_stacks), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cylinder::Cylinder(Real const radius, Real const height) :
		bottom_radius_(radius), top_radius_(radius), height_(height), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cylinder::Cylinder(Real const top_radius, Real const bottom_radius, Real const height) :
		bottom_radius_(top_radius), top_radius_(bottom_radius), height_(height), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cylinder::Cylinder(Vector3D const& center, Real const top_radius, Real const bottom_radius, Real const height) :
		bottom_radius_(top_radius), top_radius_(bottom_radius), height_(height), slices_(default_slices), stacks_(default_stacks), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cylinder::Cylinder(Vector3D const& center, Real const top_radius, Real const bottom_radius, Real const height, Uint const slices, Uint const stacks) :
		bottom_radius_(top_radius), top_radius_(bottom_radius), height_(height), slices_(slices), stacks_(stacks), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cylinder::~Cylinder()
	{
	}

	void Cylinder::computeGeometry()
	{
		RENGINE_ASSERT(slices_ > 2);
		RENGINE_ASSERT(stacks_ > 0);

		clear();
		index().clear();

		PositionNormalVertexDeclaration vertex;
		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();

	    Real const cx = center_.x();
	    Real const cy = center_.y() - height_ * 0.5f;
	    Real const cz = center_.z();

		Real x = 0.0f;
		Real y = 0.0f;
		Real z = 0.0f;

		Real theta = 0.0f;
		Real dtheta = pi_mul_2_angle / Real(slices_);
		Real dheight = height_ / Real(stacks_);
		Real dradius = (top_radius_ - bottom_radius_)  / Real(stacks_);
		Real radius = bottom_radius_;
		Real y_normal = (bottom_radius_ - top_radius_)  / height_;

		Uint base_index = 0;

		for (Uint stack = 0; stack <= stacks_; ++stack)
		{
			y = Real(stack) * dheight;
			radius = bottom_radius_ + Real(stack) * dradius;

			base_index = Uint (size());

			for (Uint slice = 0; slice != slices_; ++slice)
			{
				theta = Real(slice) * dtheta;
			    x = -sin(theta);
			    z = cos(theta);

			    vertex.position = Vector3D(cx + x * radius, cy + y, cz + z * radius);
			    vertex.normal = Vector3D(x, y_normal, z);
			    vertex_stream.add(vertex);

				if ((stack > 0) && (slice > 0))
				{
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + (slice - 1));
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + slice);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

					index().push_back(base_index - slices_ + (slice - 1));
					index().push_back(base_index + slice - 1);
					index().push_back(base_index + slice);

					index().push_back(base_index + slice);
					index().push_back(base_index - slices_ + slice);
					index().push_back(base_index - slices_ + (slice - 1));
				}
			}

			//join the last slice to the first
			if (stack > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 1);

				index().push_back(base_index - 1);
				index().push_back(base_index + slices_ - 1);
				index().push_back(base_index);

				index().push_back(base_index);
				index().push_back(base_index - slices_);
				index().push_back(base_index - 1);
			}

		}

		//
		// Top
		//

		base_index = Uint (size());
		vertex.position = Vector3D(cx, cy + height_, cz);
		vertex.normal = Vector3D(0.0f, 1.0f, 0.0f);
		vertex_stream.add(vertex);

		for (Uint slice = 0; slice != slices_; ++slice)
		{
			theta = Real(slice) * dtheta;
		    x = -sin(theta);
		    z = cos(theta);

		    vertex.position = Vector3D(cx + x * top_radius_, cy + height_, cz + z * top_radius_);
		    vertex.normal = Vector3D(0.0f, 1.0f, 0.0f);
		    vertex_stream.add(vertex);

			if (slice > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice + 1);

				index().push_back(base_index);
				index().push_back(base_index + slice + 1);
				index().push_back(base_index + slice);

			}
		}

		//join the last slice to the first
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + 1);

		index().push_back(base_index);
		index().push_back(base_index + 1);
		index().push_back(base_index + slices_);

		//
		// Bottom
		//
		base_index = Uint (size());
		vertex.position = Vector3D(cx, cy, cz);
		vertex.normal = Vector3D(0.0f, -1.0f, 0.0f);
		vertex_stream.add(vertex);

		for (Uint slice = 0; slice != slices_; ++slice)
		{
			theta = slice * dtheta;
			x = -sin(theta);
			z = cos(theta);

			vertex.position = Vector3D(cx + x * bottom_radius_, cy, cz + z * bottom_radius_);
			vertex.normal = Vector3D(0.0f, -1.0f, 0.0f);
			vertex_stream.add(vertex);

			if (slice > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice + 1);

				index().push_back(base_index);
				index().push_back(base_index + slice);
				index().push_back(base_index + slice + 1);
			}
		}

		//join the last slice to the first
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + 1);

		index().push_back(base_index);
		index().push_back(base_index + slices_);
		index().push_back(base_index + 1);
	}

	//
	// Cone
	//

	Cone::Cone() :
		radius_(0.2f), height_(0.5f), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cone::Cone(Vector3D const& center, Real const radius, Real const height) :
		radius_(radius), height_(height), slices_(default_slices), stacks_(default_stacks), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cone::Cone(Real const radius, Real const height) :
		radius_(radius), height_(height), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cone::Cone(Vector3D const& center, Real const radius, Real const height, Uint const slices, Uint const stacks) :
		radius_(radius), height_(height), slices_(slices), stacks_(stacks), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Cone::~Cone()
	{
	}

	void Cone::computeGeometry()
	{
		RENGINE_ASSERT(slices_ > 2);
		RENGINE_ASSERT(stacks_ > 1);

		clear();
		index().clear();

		PositionNormalVertexDeclaration vertex;
		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();


		Real const cx = center_.x();
		Real const cy = center_.y() - height_ * 0.5f;
		Real const cz = center_.z();

		Real x = 0.0f;
		Real y = 0.0f;
		Real z = 0.0f;

		Real theta = 0.0f;
		Real dtheta = pi_mul_2_angle / Real(slices_);
		Real dheight = height_ / Real(stacks_);
		Real dradius = radius_ / Real(stacks_);
		Real radius = 0.0f;
		Real y_normal = radius_ / height_;

		Uint base_index = 0;

		//
		//body
		//
		for (Uint stack = 0; stack != stacks_; ++stack)
		{
			y = Real(stack) * dheight;
			radius = Real(stacks_ - stack) * dradius;

			base_index = Uint (size());

			for (Uint slice = 0; slice != slices_; ++slice)
			{
				theta = Real(slice) * dtheta;
				x = -sin(theta);
				z = cos(theta);

				vertex.position = Vector3D(cx + x * radius, cy + y, cz + z * radius);
				vertex.normal = Vector3D(x, y_normal, z);
			    vertex_stream.add(vertex);

				if ((stack > 0) && (slice > 0))
				{
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + (slice - 1));
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + slice);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

					index().push_back(base_index - slices_ + (slice - 1));
					index().push_back(base_index + slice - 1);
					index().push_back(base_index + slice);

					index().push_back(base_index + slice);
					index().push_back(base_index - slices_ + slice);
					index().push_back(base_index - slices_ + (slice - 1));
				}
			}

			//join the last slice to the first
			if (stack > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 1);

				index().push_back(base_index - 1);
				index().push_back(base_index + slices_ - 1);
				index().push_back(base_index);

				index().push_back(base_index);
				index().push_back(base_index - slices_);
				index().push_back(base_index - 1);
			}
		}

		//
		// Cone Tip
		//

		base_index = Uint (size());
		vertex.position = Vector3D(cx, cy + height_, cz);
		vertex.normal = Vector3D(0.0f, 1.0f, 0.0f);
	    vertex_stream.add(vertex);

		for (Uint slice = 1; slice != slices_; ++slice)
		{
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 1);

			index().push_back(base_index - slices_ + (slice - 1));
			index().push_back(base_index);
			index().push_back(base_index - slices_ + slice);
		}

		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 1);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);

		index().push_back(base_index - 1);
		index().push_back(base_index);
		index().push_back(base_index - slices_);


		//
		// Bottom
		//
		base_index = Uint (size());
		vertex.position = Vector3D(cx, cy, cz);
		vertex.normal = Vector3D(0.0f, -1.0f, 0.0f);
	    vertex_stream.add(vertex);

		for (Uint slice = 0; slice != slices_; ++slice)
		{
			theta = slice * dtheta;
			x = -sin(theta);
			z = cos(theta);
			vertex.position = Vector3D(cx + x * radius_, cy, cz + z * radius_);
			vertex.normal = Vector3D(0.0f, -1.0f, 0.0f);
		    vertex_stream.add(vertex);

			if (slice > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice + 1);

				index().push_back(base_index);
				index().push_back(base_index + slice);
				index().push_back(base_index + slice + 1);
			}
		}

		//join the last slice to the first
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + 1);

		index().push_back(base_index);
		index().push_back(base_index + slices_);
		index().push_back(base_index + 1);
	}

	//
	// Torus
	//

	Torus::Torus() :
	inner_radius_(0.1f), outer_radius_(0.3f), sides_(default_sides), rings_(default_rings)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Torus::Torus(Vector3D const& center, Real const inner_radius, Real const outer_radius) :
		inner_radius_(inner_radius), outer_radius_(outer_radius), sides_(default_sides), rings_(default_rings), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Torus::Torus(Real const inner_radius, Real const outer_radius) :
		inner_radius_(inner_radius), outer_radius_(outer_radius), sides_(default_sides), rings_(default_rings)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Torus::Torus(Vector3D const& center, Real const inner_radius, Real const outer_radius, Uint const rings, Uint const sides) :
		inner_radius_(inner_radius), outer_radius_(outer_radius), sides_(sides), rings_(rings), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Torus::~Torus()
	{
	}

	void Torus::computeGeometry()
	{
		RENGINE_ASSERT(sides_ > 2);
		RENGINE_ASSERT(rings_ > 2);

		clear();
		index().clear();

		PositionNormalVertexDeclaration vertex;
		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();


		Real const cx = center_.x();
		Real const cy = center_.y();
		Real const cz = center_.z();

		Real x = 0.0f;
		Real y = 0.0f;
		Real z = 0.0f;

		Real dpsi = pi_mul_2_angle / Real(rings_);
		Real dphi = -pi_mul_2_angle / Real(sides_);

		Uint ring = 0;
		Uint side = 0;

		for (ring = 0; ring != rings_; ++ring)
		{
			Real psi = Real(ring) * dpsi;
			Real cpsi = sin(psi);
			Real spsi = cos(psi);

			for (side = 0; side != sides_; ++side)
			{
				Real phi = Real(side) * dphi;
				Real cphi = cos(phi);
				Real sphi = sin(phi);

				x = cpsi * (outer_radius_ + cphi * inner_radius_);
				y = spsi * (outer_radius_ + cphi * inner_radius_);
				z =							sphi * inner_radius_;
				vertex.position = Vector3D(cx + x, cy + y, cz + z);

				x = cpsi * cphi;
				y = spsi * cphi;
				z =		   sphi;
				vertex.normal = Vector3D(x, y, z);
			    vertex_stream.add(vertex);
			}
		}

		Uint base_index = 0;

		for (ring = 0; ring != rings_ - 1; ++ring)
		{
			for (side = 0; side != sides_ - 1; ++side)
			{
				base_index = ring * sides_ + side;

				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + 1);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + sides_);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + sides_ + 1);

				index().push_back(base_index);
				index().push_back(base_index + sides_);
				index().push_back(base_index + 1);

				index().push_back(base_index + 1);
				index().push_back(base_index + sides_);
				index().push_back(base_index + sides_ + 1);
			}

			// join the last side
			base_index = ring * sides_ + sides_ - 1;

			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - sides_ + 1);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + sides_);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + 1);

			index().push_back(base_index);
			index().push_back(base_index + sides_);
			index().push_back(base_index - sides_ + 1);

			index().push_back(base_index - sides_ + 1);
			index().push_back(base_index + sides_);
			index().push_back(base_index + 1);
		}

		//join the last ring
		for (side = 0; side != sides_ - 1; ++side)
		{
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), (rings_ - 1) * sides_ + side);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), (rings_ - 1) * sides_ + side + 1);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), side);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), side + 1);

			index().push_back((rings_ - 1) * sides_ + side);
			index().push_back(side);
			index().push_back((rings_ - 1) * sides_ + side + 1);

			index().push_back((rings_ - 1) * sides_ + side + 1);
			index().push_back(side);
			index().push_back(side + 1);
		}

		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), size() - 1);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), sides_ - 1);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + sides_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), size() - sides_);

		index().push_back(size() - 1);
		index().push_back(sides_ - 1);
		index().push_back(size() - sides_);

		index().push_back(size() - sides_);
		index().push_back(sides_ - 1);
		index().push_back(0);
	}

	//
	// Capsule
	//

	Capsule::Capsule() :
		radius_(0.2f), height_(0.4f), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Capsule::Capsule(Vector3D const& center, Real const radius, Real const height) :
		radius_(radius), height_(height), slices_(default_slices), stacks_(default_stacks), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Capsule::Capsule(Real const radius, Real const height) :
		radius_(radius), height_(height), slices_(default_slices), stacks_(default_stacks)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Capsule::Capsule(Vector3D const& center, Real const radius, Real const height, Uint const slices, Uint const stacks) :
		radius_(radius), height_(height), slices_(slices), stacks_(stacks), center_(center)
	{
		PositionNormalVertexDeclaration::configure(*this);
		computeGeometry();
	}

	Capsule::~Capsule()
	{
	}

	void Capsule::computeGeometry()
	{
		RENGINE_ASSERT(slices_ > 2);
		RENGINE_ASSERT(stacks_ > 2);

		clear();
		index().clear();

		PositionNormalVertexDeclaration vertex;
		VertexBuffer::Interface<PositionNormalVertexDeclaration> vertex_stream = interface<PositionNormalVertexDeclaration>();

		Real const cx = center_.x();
		Real const cy = center_.y();
		Real const cz = center_.z();

		Real x = 0.0f;
		Real y = 0.0f;
		Real z = 0.0f;

		Real theta = 0.0f;
		Real rho = 0.0f;

		Real dtheta = pi_mul_2_angle / Real(slices_);
		Real drho = pi_div_2_angle/ Real(stacks_);
		Real dheight = height_ / Real(stacks_);
		Real half_height = height_ * 0.5f;

		Uint base_index = 0;

		//
		// Sphere Y+ Tip Section
		//
		base_index = Uint (size());

		vertex.position = Vector3D(cx, cy + radius_ + half_height, cz);
		vertex.normal = Vector3D(0.0f, 1.0f, 0.0f);
		vertex_stream.add(vertex);

		for (Uint slice = 0; slice != slices_; ++slice)
		{
			theta = slice * dtheta;

			x = -sin(theta) * sin(drho);
			y = cos(drho);
			z = cos(theta) * sin(drho);

			vertex.position = Vector3D(cx + x * radius_, cy + half_height + y * radius_, cz + z * radius_);
			vertex.normal = Vector3D(x, y, z);
			vertex_stream.add(vertex);

			if (slice > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice + 1);

				index().push_back(base_index + slice + 1);
				index().push_back(base_index + slice);
				index().push_back(base_index);

			}
		}

		//join the last slice to the first
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + 1);

		index().push_back(base_index + 1);
		index().push_back(base_index + slices_);
		index().push_back(base_index);

		//
		// Sphere Y+ Intermediate Section
		//
		for (Uint stack = 2; stack <= stacks_; ++stack)
		{
			rho = stack * drho;
			base_index = Uint (size());

			for (Uint slice = 0; slice != slices_; ++slice)
			{
				theta = slice * dtheta;

				x = -sin(theta) * sin(rho);
				y = cos(rho);
				z = cos(theta) * sin(rho);

				vertex.position = Vector3D(cx + x * radius_, cy + half_height + y * radius_, cz + z * radius_);
				vertex.normal = Vector3D(x, y, z);
				vertex_stream.add(vertex);

				if (slice > 0)
				{
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + (slice - 1));
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + slice);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

					index().push_back(base_index + slice);
					index().push_back(base_index + slice - 1);
					index().push_back(base_index - slices_ + slice);

					index().push_back(base_index - slices_ + slice);
					index().push_back(base_index + slice - 1);
					index().push_back(base_index - slices_ + (slice - 1));
				}
			}

			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 1);

			index().push_back(base_index - slices_);
			index().push_back(base_index);
			index().push_back(base_index + slices_ - 1);

			index().push_back(base_index + slices_ - 1);
			index().push_back(base_index - 1);
			index().push_back(base_index - slices_);
		}

		//
		// Cylinder Section
		//

		for (Uint stack = 0; stack <= stacks_; ++stack)
		{
			y = Real(stack) * dheight;
			base_index = Uint (size());

			for (Uint slice = 0; slice != slices_; ++slice)
			{
				theta = Real(slice) * dtheta;
			    x = -sin(theta);
			    z = cos(theta);

			    vertex.position = Vector3D(cx + x * radius_,
										   cy + y - half_height,
										   cz + z * radius_);
			    vertex.normal = Vector3D(x, 0.0f, z);
			    vertex_stream.add(vertex);

				if ((stack > 0) && (slice > 0))
				{
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + (slice - 1));
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + slice);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

					index().push_back(base_index - slices_ + (slice - 1));
					index().push_back(base_index + slice - 1);
					index().push_back(base_index + slice);

					index().push_back(base_index + slice);
					index().push_back(base_index - slices_ + slice);
					index().push_back(base_index - slices_ + (slice - 1));
				}
			}

			//join the last slice to the first
			if (stack > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 1);

				index().push_back(base_index - 1);
				index().push_back(base_index + slices_ - 1);
				index().push_back(base_index);

				index().push_back(base_index);
				index().push_back(base_index - slices_);
				index().push_back(base_index - 1);
			}

		}

		//
		// Sphere Y- Tip Section
		//
		base_index = Uint (size());
		vertex.position = Vector3D(cx, cy - radius_ - half_height, cz);
		vertex.normal = Vector3D(0.0f, -1.0f, 0.0f);
		vertex_stream.add(vertex);

		for (Uint slice = 0; slice != slices_; ++slice)
		{
			theta = slice * dtheta;

			x = -sin(theta) * sin(drho);
			y = -cos(drho);
			z = cos(theta) * sin(drho);

			vertex.position = Vector3D(cx + x * radius_, cy - half_height + y * radius_, cz + z * radius_);
			vertex.normal = Vector3D(x, y, z);
			vertex_stream.add(vertex);

			if (slice > 0)
			{
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);
				SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice + 1);

				index().push_back(base_index + slice + 1);
				index().push_back(base_index);
				index().push_back(base_index + slice);
			}
		}

		//join the last slice to the first
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_);
		SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + 1);

		index().push_back(base_index + 1);
		index().push_back(base_index);
		index().push_back(base_index + slices_);

		//
		// Sphere Y- Intermediate Section
		//
		for (Uint stack = 2; stack <= stacks_; ++stack)
		{
			rho = stack * drho;
			base_index = Uint (size());

			for (Uint slice = 0; slice != slices_; ++slice)
			{
				theta = slice * dtheta;

				x = -sin(theta) * sin(rho);
				y = -cos(rho);
				z = cos(theta) * sin(rho);

				vertex.position = Vector3D(cx + x * radius_, cy - half_height + y * radius_, cz + z * radius_);
				vertex.normal = Vector3D(x, y, z);
				vertex_stream.add(vertex);

				if (slice > 0)
				{
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + (slice - 1));
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_ + slice);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice - 1);
					SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slice);

					index().push_back(base_index - slices_ + (slice - 1));
					index().push_back(base_index + slice - 1);
					index().push_back(base_index + slice);

					index().push_back(base_index + slice);
					index().push_back(base_index - slices_ + slice);
					index().push_back(base_index - slices_ + (slice - 1));
				}
			}

			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index + slices_ - 1);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - slices_);
			SHAPE_RENGINE_ASSERT_VECTOR_POS((*this), base_index - 1);

			index().push_back(base_index - 1);
			index().push_back(base_index + slices_ - 1);
			index().push_back(base_index);

			index().push_back(base_index);
			index().push_back(base_index - slices_);
			index().push_back(base_index - 1);
		}
	}
}
