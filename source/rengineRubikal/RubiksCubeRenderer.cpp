#include "RubiksCubeRenderer.h"

#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/camera/Camera.h>
#include <rengine/state/Texture.h>
#include <rengine/state/Program.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/outputstream/OutputStream.h>

#include <rengine/geometry/VertexDeclaration.h>

using namespace rengine;
using namespace std;


static Vector4D const red_color(1.0f, 0.0f, 0.0f, 1.0f);
static Vector4D const yellow_color(1.0f, 1.0f, 0.0f, 1.0f);
static Vector4D const orange_color(1.0f, 0.5f, 0.0f, 1.0f);
static Vector4D const white_color(1.0f, 1.0f, 1.0f, 1.0f);
static Vector4D const blue_color(0.0f, 0.0f, 1.0f, 1.0f);
static Vector4D const green_color(0.0f, 1.0f, 0.0f, 1.0f);
static Vector4D const black_color(0.0f, 0.0f, 0.0f, 1.0f);

struct VertexDeclaration
{
	Vector3D position;
	Vector3D normal;
	Vector4D color;
	Vector2D tex_coord;

	static void configure(VertexBuffer& vertex_buffer)
	{
		vertex_buffer.addChannel(VertexBuffer::Position);
		vertex_buffer.addChannel(VertexBuffer::Normal);
		vertex_buffer.addChannel(VertexBuffer::Color);
		vertex_buffer.addChannel(VertexBuffer::TexCoords);
		vertex_buffer.construct();
	}
};

CubeShape::CubeShape()
:width_(0.2f), cannonical_position_(0)
{
	VertexDeclaration::configure(*this);
}

inline Vector4D getColorFromFace(RubiksCube::FaceColor const& face_color)
{
	Vector4D color;

	if (face_color == RubiksCube::Black)
	{
		color = black_color;
	}
	else if (face_color == RubiksCube::Red)
	{
		color = red_color;
	}
	else if (face_color == RubiksCube::Green)
	{
		color = green_color;
	}
	else if (face_color == RubiksCube::Blue)
	{
		color = blue_color;
	}
	else if (face_color == RubiksCube::White)
	{
		color = white_color;
	}
	else if (face_color == RubiksCube::Yellow)
	{
		color = yellow_color;
	}
	else if (face_color == RubiksCube::Orange)
	{
		color = orange_color;
	}

	return color;
}

void CubeShape::computeGeometry(RubiksCube::Cube const& target_cube)
{

	clear();
	index().clear();

	//TODO: fix texture coords

	float const n = 0.577350259f;
	float const d = width_ * 0.5f;

	VertexBuffer::Interface<VertexDeclaration> vertex_stream = interface<VertexDeclaration>();
	VertexDeclaration vertex;

	SizeType current_index = size();
	Vector4D face_color = getColorFromFace(target_cube.getColor(RubiksCube::FrontFace));
	// front face
	vertex.position = Vector3D(-d, -d, d);
	vertex.normal = Vector3D(-n, -n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, -d, d);
	vertex.normal = Vector3D(n, -n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, d, d);
	vertex.normal = Vector3D(n, n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 1.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(-d, d, d);
	vertex.normal = Vector3D(-n, n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 1.0f);
	vertex_stream.add(vertex);

	index().push_back(current_index + 0);
	index().push_back(current_index + 1);
	index().push_back(current_index + 2);
	index().push_back(current_index + 2);
	index().push_back(current_index + 3);
	index().push_back(current_index + 0);


	// right
	current_index = size();
	face_color = getColorFromFace(target_cube.getColor(RubiksCube::RightFace));


	vertex.position = Vector3D(d, -d, d);
	vertex.normal = Vector3D(n, -n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, -d, -d);
	vertex.normal = Vector3D(n, -n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, d, -d);
	vertex.normal = Vector3D(n, n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 1.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, d, d);
	vertex.normal = Vector3D(n, n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 1.0f);
	vertex_stream.add(vertex);

	index().push_back(current_index + 0);
	index().push_back(current_index + 1);
	index().push_back(current_index + 2);
	index().push_back(current_index + 2);
	index().push_back(current_index + 3);
	index().push_back(current_index + 0);


	// back
	current_index = size();
	face_color = getColorFromFace(target_cube.getColor(RubiksCube::BackFace));

	vertex.position = Vector3D(-d, -d, -d);
	vertex.normal = Vector3D(-n, -n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(-d, d, -d);
	vertex.normal = Vector3D(-n, n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, d, -d);
	vertex.normal = Vector3D(n, n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 1.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, -d, -d);
	vertex.normal = Vector3D(n, -n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 1.0f);
	vertex_stream.add(vertex);

	index().push_back(current_index + 0);
	index().push_back(current_index + 1);
	index().push_back(current_index + 2);
	index().push_back(current_index + 2);
	index().push_back(current_index + 3);
	index().push_back(current_index + 0);

	// left
	current_index = size();
	face_color = getColorFromFace(target_cube.getColor(RubiksCube::LeftFace));

	vertex.position = Vector3D(-d, -d, d);
	vertex.normal = Vector3D(-n, -n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(-d, d, d);
	vertex.normal = Vector3D(-n, n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(-d, d, -d);
	vertex.normal = Vector3D(-n, n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 1.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(-d, -d, -d);
	vertex.normal = Vector3D(-n, -n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 1.0f);
	vertex_stream.add(vertex);

	index().push_back(current_index + 0);
	index().push_back(current_index + 1);
	index().push_back(current_index + 2);
	index().push_back(current_index + 2);
	index().push_back(current_index + 3);
	index().push_back(current_index + 0);


	// top
	current_index = size();
	face_color = getColorFromFace(target_cube.getColor(RubiksCube::TopFace));

	vertex.position = Vector3D(-d, d, d);
	vertex.normal = Vector3D(-n, n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, d, d);
	vertex.normal = Vector3D(n, n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, d, -d);
	vertex.normal = Vector3D(n, n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 1.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(-d, d, -d);
	vertex.normal = Vector3D(-n, n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 1.0f);
	vertex_stream.add(vertex);

	index().push_back(current_index + 0);
	index().push_back(current_index + 1);
	index().push_back(current_index + 2);
	index().push_back(current_index + 2);
	index().push_back(current_index + 3);
	index().push_back(current_index + 0);


	// bottom
	current_index = size();
	face_color = getColorFromFace(target_cube.getColor(RubiksCube::BottomFace));

	vertex.position = Vector3D(-d, -d, -d);
	vertex.normal = Vector3D(-n, -n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, -d, -d);
	vertex.normal = Vector3D(n, -n, -n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 0.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(d, -d, d);
	vertex.normal = Vector3D(n, -n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(1.0f, 1.0f);
	vertex_stream.add(vertex);

	vertex.position = Vector3D(-d, -d, d);
	vertex.normal = Vector3D(-n, -n, n);
	vertex.color = face_color;
	vertex.tex_coord = Vector2D(0.0f, 1.0f);
	vertex_stream.add(vertex);

	index().push_back(current_index + 0);
	index().push_back(current_index + 1);
	index().push_back(current_index + 2);
	index().push_back(current_index + 2);
	index().push_back(current_index + 3);
	index().push_back(current_index + 0);

	//setNeedsPrepareRendering(true);
	setNeedsDataRefresh(true);
}

RubiksCubeRenderer::RubiksCubeRenderer()
:cubes_(0), cube_size_(0.0f), spacing_(0.0f), draw_states_(0),
last_update_(0.0f), move_start_time_(0.0f)
{
	initialize();
}

RubiksCubeRenderer::~RubiksCubeRenderer()
{
	delete[](cubes_);
}

void RubiksCubeRenderer::prepareDrawing(rengine::RenderEngine& render_engine)
{

}

void RubiksCubeRenderer::unprepareDrawing(rengine::RenderEngine& render_engine)
{

}

CubeShape const& RubiksCubeRenderer::cubeShape(unsigned int const x, unsigned int const y, unsigned int const z) const
{
	RENGINE_ASSERT(x < getCubeLength());
	RENGINE_ASSERT(y < getCubeLength());
	RENGINE_ASSERT(z < getCubeLength());

	return *cubes_[cubeCoordinatesToIndex(x, y, z)];
}

CubeShape& RubiksCubeRenderer::cubeShape(unsigned int const x, unsigned int const y, unsigned int const z)
{
	RENGINE_ASSERT(x < getCubeLength());
	RENGINE_ASSERT(y < getCubeLength());
	RENGINE_ASSERT(z < getCubeLength());

	return *cubes_[cubeCoordinatesToIndex(x, y, z)];
}

CubeShape const& RubiksCubeRenderer::cubeShape(unsigned int const index) const
{
	RENGINE_ASSERT(index < (getCubeLength() * getCubeLength() * getCubeLength()));
	return *cubes_[index];
}

CubeShape& RubiksCubeRenderer::cubeShape(unsigned int const index)
{
	RENGINE_ASSERT(index < (getCubeLength() * getCubeLength() * getCubeLength()));
	return *cubes_[index];
}

void RubiksCubeRenderer::synchGeometry()
{
	float global_width = globalWidth();
	Vector3D current_position;

	current_position.x() = (-global_width / 2.0f) + (getCubeSize() / 2.0f);
	for (unsigned int x = 0; x != getCubeLength(); ++x)
	{
		current_position.y() = (-global_width / 2.0f) + (getCubeSize() / 2.0f);
		for (unsigned int y = 0; y != getCubeLength(); ++y)
		{
			current_position.z() = (global_width / 2.0f) - (getCubeSize() / 2.0f);
			for (unsigned int z = 0; z != getCubeLength(); ++z)
			{
				cubes_[cubeCoordinatesToIndex(x, y, z)]->setWidth(getCubeSize());
				cubes_[cubeCoordinatesToIndex(x, y, z)]->orientation() = Quaternion();
				cubes_[cubeCoordinatesToIndex(x, y, z)]->movingOrientation() = Quaternion();
				cubes_[cubeCoordinatesToIndex(x, y, z)]->setPosition(current_position);

				cubes_[cubeCoordinatesToIndex(x, y, z)]->setCannonicalPosition(cube(x, y, z).getCannonicalPosition());
				cubes_[cubeCoordinatesToIndex(x, y, z)]->computeGeometry( cube(x, y, z) );

				current_position.z() -= getCubeSize() + getSpacing();
			}
			current_position.y() += getCubeSize() + getSpacing();
		}
		current_position.x() += getCubeSize() + getSpacing();
	}
}


void RubiksCubeRenderer::initialize()
{
	state_ = Idle;

	cube_size_ = 0.2f;
	spacing_ = getCubeSize() * 0.1f;

	draw_states_ = new DrawStates();
	cubes_ = new SharedPointer<CubeShape>[getCubeLength() * getCubeLength() * getCubeLength()];


	for (unsigned int x = 0; x != getCubeLength(); ++x)
	{
		for (unsigned int y = 0; y != getCubeLength(); ++y)
		{
			for (unsigned int z = 0; z != getCubeLength(); ++z)
			{
				SharedPointer<CubeShape> shape = new CubeShape();
				shape->setDrawStates(draw_states_);
				cubes_[cubeCoordinatesToIndex(x, y, z)] = shape;
			}
		}
	}

	synchGeometry();

	speed_ = new SystemVariable("cube_rotation_speed", 1.0f);
	speed_->setDescription("cube speed factor. 1.0 takes 1 second to rotate, 2.0 takes half a second.");
	CoreEngine::instance()->system().registerVariable(speed_);


	draw_states_->setCapability(DrawStates::DepthTest, DrawStates::On);
	draw_states_->setProgram( CoreEngine::instance()->resourceManager().load<Program>("data/shaders/RubikalCube.eff"));

	//CoreEngine::instance()->renderEngine().pushDrawStates();
	//CoreEngine::instance()->renderEngine().apply(*draw_states_);
	//CoreEngine::instance()->renderEngine().reportProgram(*draw_states_->getProgram());
	//CoreEngine::instance()->renderEngine().popDrawStates();

/*
 	// load the textures
	vector<string> textures;
	textures.push_back("data/images/bricks/WTX_GK_001_cc.jpg");
	textures.push_back("data/images/bricks/WTX_GK_001_nm.jpg");
	textures.push_back("data/images/bricks/WTX_GK_001_sc.jpg");
	textures.push_back("data/images/bricks/WTX_GK_001_hm.jpg");

	for (vector<string>::iterator i = textures.begin(); i != textures.end(); ++i)
	{
		string filename = *i;

		SharedPointer<Texture2D> texture = CoreEngine::instance()->resourceManager().load<Texture2D>(filename);

		if (!texture)
		{
			CoreEngine::instance()->log() << "Unable to load: " << filename << std::endl;
		}
		else
		{
			draw_states_->setTexture(texture);
		}
	}
*/

}

void RubiksCubeRenderer::draw(rengine::RenderEngine& render_engine)
{
	RENGINE_ASSERT(draw_states_->hasState(DrawStates::Program));

	SharedPointer<Program> program = draw_states_->getProgram();

	program->uniform("model_view_projection").set(CoreEngine::instance()->camera()->viewMatrix(), 1);
	program->uniform("model_view_projection").set(CoreEngine::instance()->camera()->projectionMatrix(), 2);


	for (unsigned int x = 0; x != getCubeLength(); ++x)
	{
		for (unsigned int y = 0; y != getCubeLength(); ++y)
		{
			for (unsigned int z = 0; z != getCubeLength(); ++z)
			{
				Matrix translation = Matrix::translate( cubes_[cubeCoordinatesToIndex(x, y, z)]->getPosition() );
				Quaternion rotation = cubes_[cubeCoordinatesToIndex(x, y, z)]->orientation() *
									  cubes_[cubeCoordinatesToIndex(x, y, z)]->movingOrientation();

				program->uniform("model_view_projection").set(translation * rotation, 0);


				render_engine.draw(*cubes_[cubeCoordinatesToIndex(x, y, z)]);

			}
		}
	}
}

void RubiksCubeRenderer::update(float const time)
{
	float const move_duration = 1.0f / speed();

	if (state() == Moving)
	{

		float factor = clampTo( (time - move_start_time_) / move_duration, 0.0f, 1.0f);


		Vector3D rotation_axis;
		switch (current_move_.movement)
		{
			case Front:
			rotation_axis.set(0.0f, 0.0f, 1.0f);
				break;
			case Right:
			rotation_axis.set(1.0f, 0.0f, 0.0f);
				break;
			case Up:
			rotation_axis.set(0.0f, 1.0f, 0.0f);
				break;
			default:
			rotation_axis.set(0.0f, 0.0f, 0.0f);
				break;
		}

		rotation_axis *= -sign( float(current_move_.turns) );
		Quaternion rotation = Quaternion::rotate(pi_div_2_angle * factor, rotation_axis);

		//CoreEngine::instance()->log() << "Time " << time << " Factor: " << factor << " Angle: " <<  pi_div_2_angle * factor << std::endl;

		for (unsigned int x = 0; x != getCubeLength(); ++x)
		{
			for (unsigned int y = 0; y != getCubeLength(); ++y)
			{
				for (unsigned int z = 0; z != getCubeLength(); ++z)
				{

					//
					// we need to look up the real cube associated with the rendering cube
					//
					unsigned int real_cube_position = cubeFromCannonicalPosition( cubes_[cubeCoordinatesToIndex(x, y, z)]->getCannonicalPosition() );

					unsigned int real_x = 0;
					unsigned int real_y = 0;
					unsigned int real_z = 0;
					indexToCubeCoordinates(real_cube_position, real_x, real_y ,real_z);


					if ( (current_move_.movement == Front) && (real_z == current_move_.depth) )
					{
						cubes_[cubeCoordinatesToIndex(x, y, z)]->movingOrientation() = rotation;
					}

					if ( (current_move_.movement == Right) && (real_x == (getCubeLength() - current_move_.depth - 1)) )
					{
						cubes_[cubeCoordinatesToIndex(x, y, z)]->movingOrientation() = rotation;
					}

					if ( (current_move_.movement == Up) && (real_y == (getCubeLength() - current_move_.depth - 1)) )
					{
						cubes_[cubeCoordinatesToIndex(x, y, z)]->movingOrientation() = rotation;
					}

				}
			}
		}

		if (factor >= 1.0f)
		{
			state_ = Idle;
			//CoreEngine::instance()->log() << "Moving: Stop" << std::endl;

			RubiksCube::move(current_move_.movement, current_move_.depth, current_move_.turns);
			//serializeTo(CoreEngine::instance()->log());


			//
			// Concat the moving orientation to the global orientation
			//

			for (unsigned int x = 0; x != getCubeLength(); ++x)
			{
				for (unsigned int y = 0; y != getCubeLength(); ++y)
				{
					for (unsigned int z = 0; z != getCubeLength(); ++z)
					{
						cubes_[cubeCoordinatesToIndex(x, y, z)]->orientation() *= cubes_[cubeCoordinatesToIndex(x, y, z)]->movingOrientation();
						cubes_[cubeCoordinatesToIndex(x, y, z)]->movingOrientation().setAsIdentity();
					}
				}
			}


		}
	}

	last_update_ = time;
}

void RubiksCubeRenderer::startMove(Movement const& movement, unsigned int const depth, int turns)
{
	if (state() == Idle)
	{
		move_start_time_ = last_update_;
		state_ = Moving;

		//CoreEngine::instance()->log() << "Moving: Start" << std::endl;

		//
		// Hold on..
		// logic is one move behind schedule :P
		// RubiksCube::move(movement, depth, turns);
		//
		current_move_.movement = movement;
		current_move_.depth = depth;
		current_move_.turns = turns;
	}
}
