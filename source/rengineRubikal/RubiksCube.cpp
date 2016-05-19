#include "RubiksCube.h"

#include <rengine/lang/debug/Debug.h>
#include <cmath>
#include <iostream>
#include <string>

static bool equivalent(float right, float left, float epsilon = 1e-6)
{
	float delta = right - left;
	return (delta < 0.0f) ? (delta >= -epsilon) : (delta <= epsilon);
}

RubiksCube::Cube::Cube()
{
	code = 0;
	descriptor = LineCube;
}

RubiksCube::RubiksCube() :
	cube_length_(0), cells_(0), buffer_(0), cubes_(0)
{
	initializeCube(3);
}

RubiksCube::RubiksCube(RubiksCube const& copy) :
	cells_(0), buffer_(0), cubes_(0)
{
	copyObject(copy);
}


RubiksCube& RubiksCube::operator = (RubiksCube const& copy)
{
	copyObject(copy);
	return *this;
}

void RubiksCube::copyObject(RubiksCube const& copy)
{
	deinitialize();

	cube_length_ = copy.cube_length_;
	cube_moves_ = copy.cube_moves_;

	cells_ = new FaceColor[6 * cube_length_ * cube_length_]();
	buffer_ = new FaceColor[cube_length_ * cube_length_]();
	cubes_ = new Cube[cube_length_ * cube_length_ * cube_length_];


	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cells_[faceCoordinatesToIndex(face, x, y)] = copy.cells_[faceCoordinatesToIndex(face, x, y)];
			}
		}
	}

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			buffer_[y * cube_length_ + x] = copy.buffer_[y * cube_length_ + x];
		}
	}

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int z = 0; z != cube_length_; ++z)
			{
				cubes_[cubeCoordinatesToIndex(x, y, z)] = copy.cubes_[cubeCoordinatesToIndex(x, y, z)];
			}
		}
	}

}

RubiksCube::~RubiksCube()
{
	deinitialize();
}

void RubiksCube::deinitialize()
{
	if (cells_)
	{
		delete[] (cells_);
		cells_ = 0;
	}

	if (buffer_)
	{
		delete[] (buffer_);
		buffer_ = 0;
	}

	if (cubes_)
	{
		delete[] (cubes_);
		cubes_ = 0;
	}
}

void RubiksCube::initializeCube(unsigned int const length)
{
	RENGINE_ASSERT(!cells_);
	RENGINE_ASSERT(!buffer_);
	RENGINE_ASSERT(!cubes_);

	cube_length_ = length;
	cells_ = new FaceColor[6 * cube_length_ * cube_length_]();
	buffer_ = new FaceColor[cube_length_ * cube_length_]();
	cubes_ = new Cube[cube_length_ * cube_length_ * cube_length_];

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(x, y) = Red;
		}
	}

	setSolvedSetup();

	RENGINE_ASSERT(isValidContract());
}

void RubiksCube::setSolvedSetup()
{
	cube_moves_.clear();

	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cells_[faceCoordinatesToIndex(face, x, y)] = FaceColor(face);
			}
		}
	}

	unsigned int const cube_length_minus_one = cube_length_ - 1;
	unsigned int const center_index = (unsigned int)(getCubeLength() / 2);

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int z = 0; z != cube_length_; ++z)
			{
				unsigned int index = cubeCoordinatesToIndex(x, y, z);

				cubes_[index].setCannonicalPosition(index);
				cubes_[index].setCurrentPosition(index);

				if (x == 0)
				{
					cubes_[index].setColor(LeftFace, White);
					cubes_[index].setColor(RightFace, Black);
				}
				else if (x == cube_length_minus_one)
				{
					cubes_[index].setColor(LeftFace, Black);
					cubes_[index].setColor(RightFace, Yellow);
				}
				else
				{
					cubes_[index].setColor(LeftFace, Black);
					cubes_[index].setColor(RightFace, Black);
				}

				if (y == 0)
				{
					cubes_[index].setColor(BottomFace, Green);
					cubes_[index].setColor(TopFace, Black);
				}
				else if (y == cube_length_minus_one)
				{
					cubes_[index].setColor(BottomFace, Black);
					cubes_[index].setColor(TopFace, Blue);
				}
				else
				{
					cubes_[index].setColor(BottomFace, Black);
					cubes_[index].setColor(TopFace, Black);
				}

				if (z == 0)
				{
					cubes_[index].setColor(FrontFace, Red);
					cubes_[index].setColor(BackFace, Black);
				}
				else if (z == cube_length_minus_one)
				{
					cubes_[index].setColor(FrontFace, Black);
					cubes_[index].setColor(BackFace, Orange);
				}
				else
				{
					cubes_[index].setColor(FrontFace, Black);
					cubes_[index].setColor(BackFace, Black);
				}


				if ( ((x == center_index) && (y == center_index) && (z == 0)) ||
					 ((x == center_index) && (y == center_index) && (z == cube_length_minus_one)) ||

					 ((x == 0) && (y == center_index) && (z == center_index)) ||
					 ((x == cube_length_minus_one) && (y == center_index) && (z == center_index)) ||

					 ((x == center_index) && (y == 0) && (z == center_index)) ||
					 ((x == center_index) && (y == cube_length_minus_one) && (z == center_index))
					)
				{
					cubes_[index].setCubeType(FaceMasterCube);
				}

				if ( ((x == 0) && (y == 0) && (z == 0)) ||
					 ((x == 0) && (y == cube_length_minus_one) && (z == 0)) ||
					 ((x == cube_length_minus_one) && (y == cube_length_minus_one) && (z == 0)) ||
					 ((x == cube_length_minus_one) && (y == 0) && (z == 0)) ||


					 ((x == 0) && (y == 0) && (z == cube_length_minus_one)) ||
					 ((x == 0) && (y == cube_length_minus_one) && (z == cube_length_minus_one)) ||
					 ((x == cube_length_minus_one) && (y == cube_length_minus_one) && (z == cube_length_minus_one)) ||
					 ((x == cube_length_minus_one) && (y == 0) && (z == cube_length_minus_one))
					)
				{
					cubes_[index].setCubeType(CornerCube);
				}

				if ( ((x == center_index) && (y == 0) && (z == 0)) ||
					 ((x == center_index) && (y == cube_length_minus_one) && (z == 0)) ||
					 ((x == 0) && (y == center_index) && (z == 0)) ||
					 ((x == cube_length_minus_one) && (y == center_index) && (z == 0)) ||

					 ((x == center_index) && (y == 0) && (z == cube_length_minus_one)) ||
					 ((x == center_index) && (y == cube_length_minus_one) && (z == cube_length_minus_one)) ||
					 ((x == 0) && (y == center_index) && (z == cube_length_minus_one)) ||
					 ((x == cube_length_minus_one) && (y == center_index) && (z == cube_length_minus_one)) ||

					 ((x == 0) && (y == 0) && (z == center_index)) ||
					 ((x == 0) && (y == cube_length_minus_one) && (z == center_index)) ||

					 ((x == cube_length_minus_one) && (y == 0) && (z == center_index)) ||
					 ((x == cube_length_minus_one) && (y == cube_length_minus_one) && (z == center_index))
				   )
				{
					cubes_[index].setCubeType(CentralLineCube);
				}

			}
		}
	}

	RENGINE_ASSERT(isValidContract());
}

void RubiksCube::setSetup(CubeFaces const& faces)
{
	RENGINE_ASSERT(isValidContract());
	RENGINE_ASSERT(faces.size() == 6);

	//
	// Set the faces
	//
	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cell(face, x, y) = faces[face].color[x][y];
			}
		}
	}


	//
	// Correct the cubes
	//

	// set all faces to back
	for (unsigned int i = 0; i != cube_length_ * cube_length_ * cube_length_; ++i)
	{
		for (unsigned int face = 0; face != 6; ++face)
		{
			cubes_[i].setColor(Face(face), Black);
		}
	}


	// synch the colors with the faces
	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				unsigned int index = faceCoordinatesToIndex(face, x, y);

				Face cube_face;
				unsigned int cube_index = cubeFromCell(index, cube_face);

				cubes_[cube_index].setColor(cube_face, cells_[index]);
			}
		}
	}

	// synch the cubes positions and types
	RubiksCube cannonical_cube;
	for (unsigned int i = 0; i != cube_length_ * cube_length_ * cube_length_; ++i)
	{
		int index = cannonical_cube.find( cubes_[i] );
		//std::cout << "---" << i << " -- " << index << std::endl;

		RENGINE_ASSERT(index >= 0);
		RENGINE_ASSERT(index < int(cube_length_ * cube_length_ * cube_length_));

		cubes_[i].setCurrentPosition(i);
		cubes_[i].setCannonicalPosition( cannonical_cube.cube(index).getCannonicalPosition() );
		cubes_[i].setCubeType( cannonical_cube.cube(index).getCubeType() );
	}

	RENGINE_ASSERT(isValidContract());
}

RubiksCube::FaceColor const& RubiksCube::cell(unsigned int const index) const
{
	RENGINE_ASSERT(index < (6 * cube_length_ * cube_length_));
	return cells_[index];
}

RubiksCube::FaceColor& RubiksCube::cell(unsigned int const index)
{
	RENGINE_ASSERT(index < (6 * cube_length_ * cube_length_));
	return cells_[index];
}

RubiksCube::FaceColor const& RubiksCube::cell(unsigned int const face, unsigned int const x, unsigned int const y) const
{
	RENGINE_ASSERT(x < cube_length_);
	RENGINE_ASSERT(y < cube_length_);
	RENGINE_ASSERT(face < 6);
	return cells_[faceCoordinatesToIndex(face, x, y)];
}

RubiksCube::FaceColor& RubiksCube::cell(unsigned int const face, unsigned int const x, unsigned int const y)
{
	RENGINE_ASSERT(x < cube_length_);
	RENGINE_ASSERT(y < cube_length_);
	RENGINE_ASSERT(face < 6);

	return cells_[faceCoordinatesToIndex(face, x, y)];
}

RubiksCube::Cube const& RubiksCube::cube(unsigned int const index) const
{
	RENGINE_ASSERT(index < (cube_length_ * cube_length_ * cube_length_));
	RENGINE_ASSERT(isValidContract());

	return cubes_[index];
}

RubiksCube::Cube& RubiksCube::cube(unsigned int const index)
{
	RENGINE_ASSERT(index < (cube_length_ * cube_length_ * cube_length_));
	return cubes_[index];
}

RubiksCube::Cube const& RubiksCube::cube(unsigned int const x, unsigned int const y, unsigned int const z) const
{
	RENGINE_ASSERT(x < cube_length_);
	RENGINE_ASSERT(y < cube_length_);
	RENGINE_ASSERT(z < cube_length_);

	return cubes_[cubeCoordinatesToIndex(x, y, z)];
}

RubiksCube::Cube& RubiksCube::cube(unsigned int const x, unsigned int const y, unsigned int const z)
{
	RENGINE_ASSERT(x < cube_length_);
	RENGINE_ASSERT(y < cube_length_);
	RENGINE_ASSERT(z < cube_length_);

	return cubes_[cubeCoordinatesToIndex(x, y, z)];
}

RubiksCube::FaceColor& RubiksCube::bufferCell(unsigned int const x, unsigned int const y)
{
	RENGINE_ASSERT(x < cube_length_);
	RENGINE_ASSERT(y < cube_length_);

	return buffer_[y * cube_length_ + x];
}

void RubiksCube::copyBufferToFace(unsigned int const face)
{
	RENGINE_ASSERT(face < 6);

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(face, x, y) = bufferCell(x, y);
		}
	}
}

void RubiksCube::copyFaceToBuffer(unsigned int const face)
{
	RENGINE_ASSERT(face < 6);

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(x, y) = cell(face, x, y);
		}
	}
}

void RubiksCube::normalizeTurns(int& turns, bool &inverted)
{
	inverted = (turns < 0);

	if (inverted)
	{
		turns *= -1;
	}

	turns = turns % 4;
}

void RubiksCube::move(Movement const& movement, unsigned int const depth, int turns)
{
	RENGINE_ASSERT(depth < cube_length_);
	RENGINE_ASSERT(isValidContract());

	//
	// save move
	//
	CubeMove cube_move;
	cube_move.depth = depth;
	cube_move.movement = movement;
	cube_move.turns = turns;
	cube_moves_.push_back(cube_move);

	performMove(movement, depth, turns);

	RENGINE_ASSERT(isValidContract());
}

void RubiksCube::unmove()
{
	RENGINE_ASSERT(isValidContract());

	if (!moves().empty())
	{
		//
		// restore
		//
		CubeMove cube_move = cube_moves_.back();
		cube_moves_.pop_back();

		performMove(cube_move.movement, cube_move.depth, -int(cube_move.depth));
	}


	RENGINE_ASSERT(isValidContract());
}

void RubiksCube::performMove(Movement const& movement, unsigned int const depth, int turns)
{
	RENGINE_ASSERT(depth < cube_length_);
	RENGINE_ASSERT(isValidContract());

	//
	// do move
	//
	bool inverted = false;
	normalizeTurns(turns, inverted);

	switch (movement)
	{
		case Front:
		{
			for (int turn = 0; turn != turns; ++turn)
			{
				moveFront(depth, inverted);
			}
		}
			break;
		case Right:
		{
			for (int turn = 0; turn != turns; ++turn)
			{
				moveRight(depth, inverted);
			}
		}
			break;
		case Up:
		{
			for (int turn = 0; turn != turns; ++turn)
			{
				moveUp(depth, inverted);
			}
		}
			break;
		default:
		{

		}
			break;
	}


	RENGINE_ASSERT(isValidContract());
}

void RubiksCube::rotateCube(Cube& cube, Movement const& movement, bool inverted)
{
	Cube copy;

	if (movement == Front)
	{
		if (inverted)
		{
			copy.setColor(FrontFace, cube.getColor(FrontFace));
			copy.setColor(BackFace, cube.getColor(BackFace));
			copy.setColor(RightFace, cube.getColor(BottomFace));
			copy.setColor(LeftFace, cube.getColor(TopFace));
			copy.setColor(BottomFace, cube.getColor(LeftFace));
			copy.setColor(TopFace, cube.getColor(RightFace));
		}
		else
		{
			copy.setColor(FrontFace, cube.getColor(FrontFace));
			copy.setColor(BackFace, cube.getColor(BackFace));
			copy.setColor(RightFace, cube.getColor(TopFace));
			copy.setColor(LeftFace, cube.getColor(BottomFace));
			copy.setColor(BottomFace, cube.getColor(RightFace));
			copy.setColor(TopFace, cube.getColor(LeftFace));
		}
	}
	else if (movement == Right)
	{
		if (inverted)
		{
			copy.setColor(FrontFace, cube.getColor(TopFace));
			copy.setColor(BackFace, cube.getColor(BottomFace));
			copy.setColor(RightFace, cube.getColor(RightFace));
			copy.setColor(LeftFace, cube.getColor(LeftFace));
			copy.setColor(BottomFace, cube.getColor(FrontFace));
			copy.setColor(TopFace, cube.getColor(BackFace));
		}
		else
		{
			copy.setColor(FrontFace, cube.getColor(BottomFace));
			copy.setColor(BackFace, cube.getColor(TopFace));
			copy.setColor(RightFace, cube.getColor(RightFace));
			copy.setColor(LeftFace, cube.getColor(LeftFace));
			copy.setColor(BottomFace, cube.getColor(BackFace));
			copy.setColor(TopFace, cube.getColor(FrontFace));
		}
	}
	else if (movement == Up)
	{
		if (inverted)
		{
			copy.setColor(FrontFace, cube.getColor(LeftFace));
			copy.setColor(BackFace, cube.getColor(RightFace));
			copy.setColor(RightFace, cube.getColor(FrontFace));
			copy.setColor(LeftFace, cube.getColor(BackFace));
			copy.setColor(BottomFace, cube.getColor(BottomFace));
			copy.setColor(TopFace, cube.getColor(TopFace));

		}
		else
		{
			copy.setColor(FrontFace, cube.getColor(RightFace));
			copy.setColor(BackFace, cube.getColor(LeftFace));
			copy.setColor(RightFace, cube.getColor(BackFace));
			copy.setColor(LeftFace, cube.getColor(FrontFace));
			copy.setColor(BottomFace, cube.getColor(BottomFace));
			copy.setColor(TopFace, cube.getColor(TopFace));
		}
	}

	cube.setColor(FrontFace,	copy.getColor(FrontFace));
	cube.setColor(BackFace,		copy.getColor(BackFace));
	cube.setColor(RightFace,	copy.getColor(RightFace));
	cube.setColor(LeftFace,		copy.getColor(LeftFace));
	cube.setColor(BottomFace,	copy.getColor(BottomFace));
	cube.setColor(TopFace,		copy.getColor(TopFace));
}

void RubiksCube::rotateFace(Face const face, bool const inverted)
{
	if (inverted)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int x = 0; x != cube_length_; ++x)
			{
				bufferCell(cube_length_ - 1 - y, x) = cell(face, x, y);
			}
		}
		copyBufferToFace(face);
	}
	else
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int x = 0; x != cube_length_; ++x)
			{
				bufferCell(y, cube_length_ - 1 - x) = cell(face, x, y);
			}
		}
		copyBufferToFace(face);
	}
}

void RubiksCube::rotateFrontRing(unsigned int depth, bool const inverted)
{
	if (inverted)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(0, y) = cell(TopFace, y, depth);
		}

		//right -> top
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(TopFace, cube_length_ - 1 - y, depth) = cell(RightFace, depth, y);
		}

		//bottom -> right
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(RightFace, depth, y) = cell(BottomFace, y, cube_length_ - depth - 1);
		}

		//left -> bottom
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BottomFace, y, cube_length_ - depth - 1) = cell(LeftFace, cube_length_ - depth - 1, cube_length_ - 1 -y);
		}

		//buffer to white
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(LeftFace, cube_length_ - depth - 1, y) = bufferCell(0, y);
		}
	}
	else
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(0, y) = cell(TopFace, y, depth);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(TopFace, y, depth) = cell(LeftFace, cube_length_ - depth - 1, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(LeftFace, cube_length_ - depth - 1, cube_length_ - y - 1) = cell(BottomFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BottomFace, y, cube_length_ - depth - 1) = cell(RightFace, depth, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(RightFace, depth, cube_length_ - y - 1) = bufferCell(0, y);
		}
	}

	// rotate cubes
	Cube* cubes_buffer = new Cube[cube_length_ * cube_length_];

	if (inverted)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cubes_buffer[cubeCoordinatesToIndex(cube_length_ - 1 - y, x, 0)] = cube(x, y, depth);
			}
		}
	}
	else
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cubes_buffer[cubeCoordinatesToIndex(y, cube_length_ - 1 - x, 0)] = cube(x, y, depth);
			}
		}
	}

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			unsigned int const cube_index = cubeCoordinatesToIndex(x, y, 0);

			cubes_buffer[cube_index].setCurrentPosition( cube(x, y, depth).getCurrentPosition() );

			cube(x, y, depth) = cubes_buffer[cube_index];
			rotateCube(cube(x, y, depth), Front, inverted);
		}
	}


	delete[](cubes_buffer);
}

void RubiksCube::rotateRightRing(unsigned int const depth, bool const inverted)
{
	if (inverted)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(0, y) = cell(FrontFace, cube_length_ - depth - 1, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(FrontFace, cube_length_ - depth - 1, y) = cell(TopFace, cube_length_ - depth - 1, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(TopFace, cube_length_ - depth - 1, cube_length_ - y - 1) = cell(BackFace, depth, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BackFace, depth, y) = cell(BottomFace, cube_length_ - depth - 1,  cube_length_ - y - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BottomFace, cube_length_ - depth - 1, y) = bufferCell(0, y);
		}
	}
	else
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(0, y) = cell(FrontFace, cube_length_ - depth - 1, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(FrontFace, cube_length_ - depth - 1, y) = cell(BottomFace, cube_length_ - depth - 1, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BottomFace, cube_length_ - depth - 1, cube_length_ - y - 1) = cell(BackFace, depth, y);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BackFace, depth, y) = cell(TopFace, cube_length_ - depth - 1,  cube_length_ - y - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(TopFace, cube_length_ - depth - 1, y) = bufferCell(0, y);
		}
	}

	// rotate cubes
	Cube* cubes_buffer = new Cube[cube_length_ * cube_length_];

	if (inverted)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cubes_buffer[cubeCoordinatesToIndex(cube_length_ - 1 - y, x, 0)] = cube(cube_length_ - depth - 1, y, x);
			}
		}
	}
	else
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cubes_buffer[cubeCoordinatesToIndex(y, cube_length_ - 1 - x, 0)] = cube(cube_length_ - depth - 1, y, x);
			}
		}
	}

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			unsigned int const cube_index = cubeCoordinatesToIndex(x, y, 0);

			cubes_buffer[cube_index].setCurrentPosition( cube(cube_length_ - depth - 1, y, x).getCurrentPosition() );

			cube(cube_length_ - depth - 1, y, x) = cubes_buffer[cube_index];
			rotateCube( cube(cube_length_ - depth - 1, y, x), Right, inverted);
		}
	}

	delete[] (cubes_buffer);
}

void RubiksCube::rotateUpRing(unsigned int const depth, bool const inverted)
{
	if (inverted)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(0, y) = cell(FrontFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(FrontFace, y, cube_length_ - depth - 1) = cell(LeftFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(LeftFace, y, cube_length_ - depth - 1) = cell(BackFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BackFace, y, cube_length_ - depth - 1) = cell(RightFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(RightFace, y, cube_length_ - depth - 1) = bufferCell(0, y);
		}
	}
	else
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			bufferCell(0, y) = cell(FrontFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(FrontFace, y, cube_length_ - depth - 1) = cell(RightFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(RightFace, y, cube_length_ - depth - 1) = cell(BackFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(BackFace, y, cube_length_ - depth - 1) = cell(LeftFace, y, cube_length_ - depth - 1);
		}

		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			cell(LeftFace, y, cube_length_ - depth - 1) = bufferCell(0, y);
		}
	}

	// rotate cubes
	Cube* cubes_buffer = new Cube[cube_length_ * cube_length_];

	if (inverted)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cubes_buffer[cubeCoordinatesToIndex(cube_length_ - 1 - y, x, 0)] = cube(x, cube_length_ - depth - 1, y);
			}
		}
	}
	else
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				cubes_buffer[cubeCoordinatesToIndex(y, cube_length_ - 1 - x, 0)] = cube(x, cube_length_ - depth - 1, y);
			}
		}
	}

	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			unsigned int const cube_index = cubeCoordinatesToIndex(x, y, 0);
			cubes_buffer[cube_index].setCurrentPosition( cube(x, cube_length_ - depth - 1, y).getCurrentPosition() );

			cube(x, cube_length_ - depth - 1, y) = cubes_buffer[cube_index];
			rotateCube( cube(x, cube_length_ - depth - 1, y), Up, inverted);
		}
	}

	delete[] (cubes_buffer);
}

void RubiksCube::moveFront(unsigned int depth, bool const inverted)
{
	RENGINE_ASSERT(depth < cube_length_);

	if (depth == 0)
	{
		rotateFace(FrontFace, inverted);
	}
	else if (depth == (cube_length_ - 1))
	{
		rotateFace(BackFace, !inverted);
	}

	rotateFrontRing(depth, inverted);
}

void RubiksCube::moveRight(unsigned int const depth, bool const inverted)
{
	RENGINE_ASSERT(depth < cube_length_);

	if (depth == 0)
	{
		rotateFace(RightFace, inverted);
	}
	else if (depth == (cube_length_ - 1))
	{
		rotateFace(LeftFace, !inverted);
	}

	rotateRightRing(depth, inverted);
}

void RubiksCube::moveUp(unsigned int const depth, bool const inverted)
{
	RENGINE_ASSERT(depth < cube_length_);

	if (depth == 0)
	{
		rotateFace(TopFace, inverted);
	}
	else if (depth == (cube_length_ - 1))
	{
		rotateFace(BottomFace, !inverted);
	}

	rotateUpRing(depth, inverted);
}

unsigned int RubiksCube::cubeFromCell(unsigned int const face_index, Face& cube_face) const
{
	RENGINE_ASSERT(face_index != NoneIndex);
	RENGINE_ASSERT(face_index >= 0);
	RENGINE_ASSERT(face_index < (6 * cube_length_ * cube_length_));

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int face = 0;

	indexToFaceCoordinates(face_index, face, x, y);

	cube_face = Face(face);

	unsigned int cube_x = 0;
	unsigned int cube_y = 0;
	unsigned int cube_z = 0;

	unsigned int const cube_length_minus_one = cube_length_ - 1;

	if (face == LeftFace)
	{
		cube_x = 0;
		cube_y = y;
		cube_z = cube_length_minus_one - x;
	}
	else if (face == RightFace)
	{
		cube_x = cube_length_minus_one;
		cube_y = y;
		cube_z = x;
	}
	else if (face == BottomFace)
	{
		cube_x = x;
		cube_y = 0;
		cube_z = cube_length_minus_one - y;
	}
	else if (face == TopFace)
	{
		cube_x = x;
		cube_y = cube_length_minus_one;
		cube_z = y;
	}
	else if (face == FrontFace)
	{
		cube_z = 0;
		cube_x = x;
		cube_y = y;
	}
	else if (face == BackFace)
	{
		cube_z = cube_length_minus_one;
		cube_x = cube_length_minus_one - x;
		cube_y = y;
	}

	return cubeCoordinatesToIndex(cube_x, cube_y, cube_z);
}

unsigned int RubiksCube::cellFromCube(unsigned int const cube_index, Face const& cube_face) const
{
	RENGINE_ASSERT(cube_index != NoneIndex);
	RENGINE_ASSERT(cube_index >= 0);
	RENGINE_ASSERT(cube_index < (cube_length_ * cube_length_ * cube_length_));

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;

	indexToCubeCoordinates(cube_index, x, y, z);

	unsigned int const cube_length_minus_one = cube_length_ - 1;
	unsigned int index = NoneIndex;

	if ((x == 0) && (cube_face == LeftFace))
	{
		index = faceCoordinatesToIndex(LeftFace, cube_length_minus_one - z, y);
	}
	else if ((x == cube_length_minus_one) && (cube_face == RightFace))
	{
		index = faceCoordinatesToIndex(RightFace, z, y);
	}
	else if ((y == 0) && (cube_face == BottomFace))
	{
		index = faceCoordinatesToIndex(BottomFace, x, cube_length_minus_one - z);
	}
	else if ((y == cube_length_minus_one) && (cube_face == TopFace))
	{
		index = faceCoordinatesToIndex(TopFace, x, z);
	}
	else if ((z == 0) && (cube_face == FrontFace))
	{
		index = faceCoordinatesToIndex(FrontFace, x, y);
	}
	else if ((z == cube_length_minus_one) && (cube_face == BackFace))
	{
		index = faceCoordinatesToIndex(BackFace, cube_length_minus_one - x, y);
	}

	return index;
}

unsigned int RubiksCube::cubeFromCannonicalPosition(unsigned int const cannonical_position) const
{
	RENGINE_ASSERT(cannonical_position != NoneIndex);
	RENGINE_ASSERT(cannonical_position >= 0);
	RENGINE_ASSERT(cannonical_position < (cube_length_ * cube_length_ * cube_length_));

	unsigned int found = NoneIndex;

	for (unsigned int i = 0; ( (i != cube_length_ * cube_length_ * cube_length_) && (found == NoneIndex) ); ++i)
	{
		if (cube(i).getCannonicalPosition() == cannonical_position)
		{
			found = cube(i).getCurrentPosition();
		}
	}

	RENGINE_ASSERT(found != NoneIndex);

	return found;
}

bool RubiksCube::isValidContract() const
{
	return true;
	return isValid();
}

bool RubiksCube::isValid() const
{

	bool is_valid = true;

	//this should be a unit test
	// faceCoordinatesToIndex <-> indexToFaceCoordinates
	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				unsigned int index = faceCoordinatesToIndex(face, x, y);

				unsigned int check_x = 0;
				unsigned int check_y = 0;
				unsigned int check_face = 0;
				indexToFaceCoordinates(index, check_face, check_x, check_y);

				is_valid &= (check_x == x);
				//RENGINE_ASSERT(is_valid);

				is_valid &= (check_y == y);
				//RENGINE_ASSERT(is_valid);

				is_valid &= (check_face == face);
				//RENGINE_ASSERT(is_valid);
			}
		}
	}

	//this should be a unit test
	// cubeCoordinatesToIndex <-> indexToCubeCoordinates
	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int z = 0; z != cube_length_; ++z)
			{
				unsigned int index = cubeCoordinatesToIndex(x, y, z);

				//this should be a unit test
				unsigned int check_x = 0;
				unsigned int check_y = 0;
				unsigned int check_z = 0;
				indexToCubeCoordinates(index, check_x, check_y, check_z);

				is_valid &= (check_x == x);
				//RENGINE_ASSERT(is_valid);

				is_valid &= (check_y == y);
				//RENGINE_ASSERT(is_valid);

				is_valid &= (check_z == z);
				//RENGINE_ASSERT(is_valid);

				is_valid &= cubes_[index].getCurrentPosition() == index;
				//RENGINE_ASSERT(is_valid);
			}
		}
	}


	// this should be a unit test
	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int z = 0; z != cube_length_; ++z)
			{
				unsigned int cube_index = cubeCoordinatesToIndex(x, y, z);

				for (unsigned int face = 0; face != 6; ++face)
				{
					unsigned int cell_index = cellFromCube(cube_index, Face(face) );

					if (cell_index != NoneIndex)
					{
						Face check_face;
						unsigned int check_cube_index = cubeFromCell(cell_index, check_face);

						is_valid &= (cube_index == check_cube_index);
						//RENGINE_ASSERT(is_valid);

						is_valid &= (check_face == Face(face));
						//RENGINE_ASSERT(is_valid);
					}
				}
			}
		}
	}




	// test if the cubes and faces are synched
	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int z = 0; z != cube_length_; ++z)
			{
				unsigned int cube_index = cubeCoordinatesToIndex(x, y, z);

				for (unsigned int face = 0; face != 6; ++face)
				{
					unsigned int cell_index = cellFromCube(cube_index, Face(face));

					if (cell_index != NoneIndex)
					{
						is_valid &= ( cubes_[cube_index].getColor(Face(face)) == cells_[cell_index] );
						//RENGINE_ASSERT(is_valid);
					}
				}
			}
		}
	}

	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length_; ++x)
		{
			for (unsigned int y = 0; y != cube_length_; ++y)
			{
				unsigned int index = faceCoordinatesToIndex(face, x, y);

				Face cube_face;
				unsigned int cube_index = cubeFromCell(index, cube_face);

				is_valid &= ( cubes_[cube_index].getColor(cube_face) == cells_[index] );
				//RENGINE_ASSERT(is_valid);
			}
		}
	}

	//
	// check cube types
	//
	unsigned int corners = 0;
	unsigned int face_master = 0;
	unsigned int central_line = 0;
	for (unsigned int x = 0; x != cube_length_; ++x)
	{
		for (unsigned int y = 0; y != cube_length_; ++y)
		{
			for (unsigned int z = 0; z != cube_length_; ++z)
			{
				unsigned int cube_index = cubeCoordinatesToIndex(x, y, z);

				is_valid &= (cubes_[cube_index].getCubeType() != RubiksCube::All);
				RENGINE_ASSERT(is_valid);

				if (cubes_[cube_index].getCubeType() == RubiksCube::FaceMasterCube)
				{
					face_master++;
				}

				if (cubes_[cube_index].getCubeType() == RubiksCube::CentralLineCube)
				{
					central_line++;
				}

				if (cubes_[cube_index].getCubeType() == RubiksCube::CornerCube)
				{
					corners++;
				}
			}
		}
	}

	is_valid &= (corners == 8);
	RENGINE_ASSERT(is_valid);
	is_valid &= (face_master == 6);
	RENGINE_ASSERT(is_valid);
	is_valid &= (central_line == 12);
	RENGINE_ASSERT(is_valid);


	return is_valid;
}

int RubiksCube::find(Cube const& cube) const
{
	int found = -1;
	bool findable[6];


	for (unsigned int i = 0; (i != (cube_length_ * cube_length_ * cube_length_)) && (found == -1); ++i)
	{
		RubiksCube::Cube const& current_cube = this->cube(i);

		for (unsigned int j = 0; j != 6; ++j)
		{
			findable[j] = true;
		}
		int found_count = 0;


		for (unsigned int cube_face = 0; cube_face != 6; ++cube_face)
		{
			for (unsigned int this_face = 0; this_face != 6; ++this_face)
			{
				if (findable[this_face])
				{
					if ( current_cube.getColor(Face(this_face)) == cube.getColor(Face(cube_face)) )
					{
						findable[this_face] = false;
						found_count++;
						break;
					}
				}
			}
		}


		if (found_count == 6)
		{
			found = int(i);
		}

	}

	return found;
}


RubiksCube::Positions RubiksCube::find(FaceColor const& first, FaceColor const& second, FaceColor const& third, CubeType const& type) const
{
	Positions positions;

	for (unsigned int i = 0; i != (cube_length_ * cube_length_ * cube_length_); ++i)
	{
		RubiksCube::Cube const& current_cube = cube(i);

		if (
			 (current_cube.hasColor(first) && current_cube.hasColor(second) && current_cube.hasColor(third)) &&
			 ((current_cube.getCubeType() == type) || (type == RubiksCube::All))
		   )
		{
			positions.push_back(current_cube.getCurrentPosition());
		}
	}

	return positions;
}
void RubiksCube::cubesVector(unsigned int const left, unsigned int const right, float& x, float& y, float& z) const
{
	float left_x, left_y, left_z;
	indexToCenteredPosition(left, left_x, left_y, left_z);


	float right_x, right_y, right_z;
	indexToCenteredPosition(right, right_x, right_y, right_z);

	x = float(left_x) - float(right_x);
	y = float(left_y) - float(right_y);
	z = float(left_z) - float(right_z);

}

float RubiksCube::cubesDistance(unsigned int const left, unsigned int const right) const
{
	float x, y, z;
	cubesVector(left, right, x, y, z);

	return sqrt(x * x + y * y + z * z);
}

bool RubiksCube::isCubeOnFace(Face const face, unsigned int index) const
{
	float x, y, z;
	indexToCenteredPosition(index, x, y, z);

	float nx, ny, nz;
	RubiksCube::faceNormal(face, nx, ny, nz);

	float center_position = float( (unsigned int)(getCubeLength() / 2) );

	nx *= center_position;
	ny *= center_position;
	nz *= center_position;

	float distance = nx * x + ny * y + nz * z - center_position;

	return equivalent(distance, 0.0f);
}

RubiksCube::CubeMove RubiksCube::faceBaseMove(Face const& face)
{
	CubeMove move;

	switch (face)
	{
		case FrontFace:
		{
			move.depth = 0;
			move.turns = 1;
			move.movement = Front;

		}
			break;

		case BackFace:
		{
			move.depth = 2;
			move.turns = 1;
			move.movement = Front;
		}
			break;

		case RightFace:
		{
			move.depth = 0;
			move.turns = 1;
			move.movement = Right;
		}
			break;

		case LeftFace:
		{
			move.depth = 2;
			move.turns = 1;
			move.movement = Right;
		}
			break;

		case TopFace:
		{
			move.depth = 0;
			move.turns = 1;
			move.movement = Up;
		}
			break;

		case BottomFace:
		{
			move.depth = 2;
			move.turns = 1;
			move.movement = Up;
		}
			break;

		default:
		{
			move.depth = 0;
			move.turns = 0;
			move.movement = Right;
		}
			break;

	}

	return move;
}

void RubiksCube::serializeTo(std::ostream &out)
{
	for (unsigned int y = 0; y != cube_length_; ++y)
	{
		for (unsigned int face = 0; face != 6; ++face)
		{
			out << "[";
			for (unsigned int x = 0; x != cube_length_; ++x)
			{
				out << cell(face, x, cube_length_ - y - 1);

			}
			out << "]";
		}

		out << std::endl;
	}
}

std::ostream& operator << (std::ostream& out, RubiksCube::FaceColor const& face)
{
	std::string as_string;
	switch (face)
	{
		case RubiksCube::Red:		{ as_string = "R"; } break;
		case RubiksCube::Yellow: 	{ as_string = "Y"; } break;
		case RubiksCube::Orange:	{ as_string = "O"; } break;
		case RubiksCube::White:		{ as_string = "W"; } break;
		case RubiksCube::Blue:		{ as_string = "B"; } break;
		case RubiksCube::Green:		{ as_string = "G"; } break;
		case RubiksCube::Black:		{ as_string = "X"; } break;

		default:
		{
			as_string = "invalid_name";
		}
			break;
	}
	out << as_string;
	return out;
}


//
// Computes rotation index
// this should be removed
//
//void RubiksCube::computeIndex()
//{
//	unsigned int face = 0;
//
//	for (unsigned int y = 0; y != cube_length_; ++y)
//	{
//		for (unsigned int x = 0; x != cube_length_; ++x)
//		{
//			cell(face, x, y) = (RubiksCube::FaceColor)faceCoordinatesToIndex(face, x, y);
//		}
//	}
//
//
//	for (unsigned int i = 0; i != 4; ++i)
//	{
//
//
//
//			for (unsigned int y = 0; y != cube_length_; ++y)
//			{
//				for (unsigned int x = 0; x != cube_length_; ++x)
//				{
//					bufferCell(cube_length_ - 1 - y, x) = cell(face, x, y);
//				}
//			}
//			copyBufferToFace(face);
//
//	}
//
//
//
//	for (unsigned int y = 0; y != cube_length_; ++y)
//	{
//		for (unsigned int x = 0; x != cube_length_; ++x)
//		{
//			unsigned int index = (unsigned int) (cell(face, x, y));
//			unsigned int input_x = 0;
//			unsigned int input_y = 0;
//			indexToFaceCoordinates(index, face, input_x, input_y);
//
//			std::cout << "output.color[" << x << "][" << y << "] = input.color[" << input_x << "][" << input_y << "];" << std::endl;
//		}
//	}
//
//
//}

