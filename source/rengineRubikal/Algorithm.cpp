#include "Algorithm.h"

#include <list>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <iomanip>

#include <rengine/thread/Thread.h>
#include <rengine/lang/Lang.h>
#include <rengine/lang/debug/Debug.h>



//#define ALGORITHM_LOG(msg) std::cout << msg << std::endl;
#define ALGORITHM_LOG(msg)

RubiksCube::CubeMoves RubiksCubeOptimizer::inverseOptimization(RubiksCube::CubeMoves const& moves)
{
	RubiksCube::CubeMoves::size_type current = 0;
	RubiksCube::CubeMoves in = moves;
	RubiksCube::CubeMoves out;

	//
	// remove movements followed by inverse
	//

	while (current != in.size())
	{
		bool add_to_out = true;

		if (!out.empty())
		{
			RubiksCube::CubeMove out_back = out.back();
			RubiksCube::CubeMove in_front = in[current];

			if (RubiksCube::isInverse(out_back, in_front))
			{
				add_to_out = false;
				out.pop_back();
			}
		}

		if (add_to_out)
		{
			out.push_back(in[current]);
		}

		++current;
	}

	return out;
}

RubiksCube::CubeMoves RubiksCubeOptimizer::fastSameMove(RubiksCube::CubeMoves const& moves)
{
	RubiksCube::CubeMoves::size_type current = 0;
	RubiksCube::CubeMoves in = moves;
	RubiksCube::CubeMoves out;

	typedef std::list<RubiksCube::CubeMove> MoveBuffer;
	MoveBuffer buffer;

	while (current != in.size())
	{
		buffer.push_back(in[current]);

		if (buffer.size() == 3)
		{
			bool all_equal = true;

			for (MoveBuffer::iterator i = buffer.begin(); i != buffer.end(); ++i)
			{
				RubiksCube::CubeMove current_buffer_move = *i;
				RubiksCube::CubeMove test_move = buffer.back();
				all_equal &= (current_buffer_move == test_move);
			}

			if (all_equal)
			{
				RubiksCube::CubeMove move = buffer.back();
				move.turns *= -1;
				out.push_back(move);

				buffer.clear();
			}
			else
			{
				out.push_back(buffer.front());
				buffer.pop_front();
			}

		}

		++current;
	}

	while (!buffer.empty())
	{
		out.push_back(buffer.front());
		buffer.pop_front();
	}

	//	out = in;


	return out;
}

RubiksCube::CubeMoves RubiksCubeOptimizer::operator()(RubiksCube::CubeMoves& moves)
{
	RubiksCube::CubeMoves in = moves;
	RubiksCube::CubeMoves out;

	unsigned int in_size = (unsigned int) (in.size());
	unsigned int out_size = (unsigned int) (out.size());

	while (in_size != out_size)
	{
		out = fastSameMove(inverseOptimization(in));

		in_size = (unsigned int) (in.size());
		out_size = (unsigned int) (out.size());

		in = out;
	}

	return out;
}

RubiksCube::CubeMoves RubiksCubeShuffle::operator()(RubiksCube& rubics_cube)
{
	RubiksCube::CubeMoves moves;

	RubiksCube::CubeMove move;
	srand(time(0));

	unsigned int last_movement = 100;
	for (unsigned int i = 0; i != total_moves_; ++i)
	{
		move.depth = rand() % rubics_cube.getCubeLength();

		unsigned int current_movement = 0;

		do
		{
			current_movement = RubiksCube::Movement(rand() % 3);
		}
		while (current_movement == last_movement);

		last_movement = current_movement;

		move.movement = RubiksCube::Movement(current_movement);
		move.turns = ((rand() % 10) > 4) ? 1 : -1;

		moves.push_back(move);
	}

	return moves;
}

bool RubiksCubeCross::solved(RubiksCube const& rubics_cube)
{
	return solvedFace(RubiksCube::FrontFace, rubics_cube) && solvedFace(RubiksCube::RightFace, rubics_cube) && solvedFace(RubiksCube::BackFace, rubics_cube) && solvedFace(
			RubiksCube::LeftFace, rubics_cube);
}

bool RubiksCubeCross::solvedFace(RubiksCube::Face const& face, RubiksCube const& rubics_cube)
{
	bool stage_solved = true;

	unsigned int const center_index = (unsigned int) (rubics_cube.getCubeLength() / 2);

	for (unsigned int i = 0; i != center_index; ++i)
	{
		stage_solved &= (rubics_cube.cell(face, center_index, center_index) == rubics_cube.cell(face, center_index, center_index + i + 1));

		switch (face)
		{
			case RubiksCube::Front:
			stage_solved &= (rubics_cube.cell(RubiksCube::TopFace, center_index, i) == rubics_cube.cell(RubiksCube::TopFace, center_index, center_index));
				break;
			case RubiksCube::Right:
			stage_solved &= (rubics_cube.cell(RubiksCube::TopFace, rubics_cube.getCubeLength() - i - 1, center_index) == rubics_cube.cell(RubiksCube::TopFace, center_index,
					center_index));
				break;
			case RubiksCube::BackFace:
			stage_solved &= (rubics_cube.cell(RubiksCube::TopFace, center_index, rubics_cube.getCubeLength() - i - 1) == rubics_cube.cell(RubiksCube::TopFace, center_index,
					center_index));
				break;
			case RubiksCube::LeftFace:
			stage_solved &= (rubics_cube.cell(RubiksCube::TopFace, i, center_index) == rubics_cube.cell(RubiksCube::TopFace, center_index, center_index));
				break;

			default:
				break;

		}
	}

	return stage_solved;
}

static RubiksCube::Face faceOnTheRight(RubiksCube::Face const face)
{
	RubiksCube::Face face_on_the_right;

	switch (face)
	{
		case RubiksCube::FrontFace:
		face_on_the_right = RubiksCube::RightFace;
			break;

		case RubiksCube::RightFace:
		face_on_the_right = RubiksCube::BackFace;
			break;

		case RubiksCube::BackFace:
		face_on_the_right = RubiksCube::LeftFace;
			break;

		case RubiksCube::LeftFace:
		face_on_the_right = RubiksCube::FrontFace;
			break;

		default:
			break;

	}

	return face_on_the_right;
}

//
// Cross
//

RubiksCube::CubeMoves RubiksCubeCross::operator()(RubiksCube& rubics_cube)
{
	RubiksCube cube(rubics_cube);
	cube.clearHistory();

	typedef std::vector<RubiksCube::Face> FaceVector;
	FaceVector faces;
	faces.push_back(RubiksCube::LeftFace);
	faces.push_back(RubiksCube::RightFace);
	faces.push_back(RubiksCube::BackFace);
	faces.push_back(RubiksCube::FrontFace);

	unsigned int const center_index = cube.getCenterIndex();
	RubiksCube::FaceColor top_color = cube.cell(RubiksCube::TopFace, center_index, center_index);

	while (!faces.empty())
	{
		RubiksCube::Face current_face = faces.back();
		faces.pop_back();

		while (!solvedFace(current_face, cube))
		{
			RubiksCube::FaceColor face_color = cube.cell(current_face, center_index, center_index);

			//
			// Find the position of the target central line
			//
			RubiksCube::Positions cubes_found = cube.find(top_color, face_color, RubiksCube::Black, RubiksCube::CentralLineCube);
			RENGINE_ASSERT(cubes_found.size() == 1);
			unsigned int found_index = cubes_found.back();

			//
			// Find where should the cube should be!
			//
			RubiksCube::Face cell_face;
			int target_position = cube.cubeFromCell(cube.faceCoordinatesToIndex(current_face, cube.getCenterIndex(), cube.getCubeLengthMinusOne()), cell_face);
			RENGINE_ASSERT(current_face == cell_face);

			//
			// If the found cube is on the target face
			//
			if (cube.isCubeOnFace(current_face, found_index))
			{
				float x, y, z;
				cube.cubesVector(target_position, found_index, x, y, z);
				float distance = cube.cubesDistance(target_position, found_index);

				unsigned int distance_as_int = (unsigned int) (distance);
				unsigned int move_turns = (unsigned int) (distance) / center_index;

				int sign = ((x < 0.0f) || (y < 0.0f) || (z < 0.0f)) ? -1 : 1;

				RubiksCube::CubeMove move = cube.faceBaseMove(current_face);

				//we need to check if the "face" color cube color matches the cubes-current-face color
				if (cube.cube(found_index).getColor(current_face) == face_color)
				{
					move.turns *= sign;
					for (unsigned int i = 0; i != move_turns; ++i)
					{
						cube.move(move);
					}
					ALGORITHM_LOG( "Faces Aligned [" << x << "] [" << y << "] [" << z << "]" << " : " << distance );
				}
				else
				{
					//
					// we need to align the face
					//

					// move the face to the bottom and do the flip:P

					if (move_turns == 0) //on the correct place but not align
					{
						move_turns = 2;
					}

					if (distance_as_int != cube.getCubeLengthMinusOne())
					{
						move.turns *= -sign;
						for (unsigned int i = 0; i != move_turns; ++i)
						{
							cube.move(move);
						}
					}

					RubiksCube::Face face_on_the_right = faceOnTheRight(current_face);

					move.movement = RubiksCube::Up;
					move.depth = cube.getCubeLengthMinusOne();
					move.turns = -1;
					cube.move(move);

					int face_on_the_right_sign = ((current_face == RubiksCube::FrontFace) || (current_face == RubiksCube::LeftFace)) ? 1 : -1;
					int current_face_sign = ((current_face == RubiksCube::BackFace) || (current_face == RubiksCube::LeftFace)) ? -1 : 1;

					move = cube.faceBaseMove(face_on_the_right);
					move.turns = 1 * face_on_the_right_sign;
					cube.move(move);

					move = cube.faceBaseMove(current_face);
					move.turns = -1 * current_face_sign;
					cube.move(move);

					//
					// This is only necessary to put the sides back on the initial position
					//
					move = cube.faceBaseMove(face_on_the_right);
					move.turns = -1 * face_on_the_right_sign;
					cube.move(move);

					ALGORITHM_LOG( "Faces Not Aligned [" << x << "] [" << y << "] [" << z << "]" << " : " << distance );
				}

			}
			else // if the found cube is not on the target face
			{

				ALGORITHM_LOG( "Target on another face" );

				//
				// we need to move the cube to the bottom center
				//
				RubiksCube::Faces faces = cube.getFaces(found_index);
				RubiksCube::Face face = RubiksCube::TopFace;

				ALGORITHM_LOG( "Faces [" << faces.size() << "]");

				for (unsigned i = 0; (i != faces.size()) && (face == RubiksCube::TopFace); ++i)
				{
					ALGORITHM_LOG( "Face [" << faces[i] << "]");

					if ((faces[i] != RubiksCube::TopFace) && (faces[i] != RubiksCube::BottomFace))
					{
						face = faces[i];
					}
				}

				RubiksCube::Face cell_face;
				int bottom_position = cube.cubeFromCell(cube.faceCoordinatesToIndex(face, cube.getCenterIndex(), cube.getCubeLengthMinusOne()), cell_face);
				RENGINE_ASSERT(RubiksCube::Face(face) == cell_face);

				float x, y, z;
				cube.cubesVector(bottom_position, found_index, x, y, z);
				float distance = cube.cubesDistance(bottom_position, found_index);

				ALGORITHM_LOG( "Not On The Same Face [" << face << "] - [" << x << "] [" << y << "] [" << z << "]" << " : " << distance );
				unsigned int move_to_bottom_turns = (unsigned int) (distance) / center_index;
				RubiksCube::CubeMove move_to_bottom = cube.faceBaseMove(face);
				int sign = ((x < 0.0f) || (y < 0.0f) || (z < 0.0f)) ? -1 : 1;

				if (move_to_bottom_turns == 0) // on top
				{
					move_to_bottom_turns = 2;
				}
				else if (move_to_bottom_turns == 2) // already on bottom
				{
					move_to_bottom_turns = 0;
				}

				move_to_bottom.turns *= -sign;
				for (unsigned int i = 0; i != move_to_bottom_turns; ++i)
				{
					cube.move(move_to_bottom);
				}

				//
				// The cube is now on the face's bottom center position, move-it to the target face
				//

				cube.indexToCenteredPosition(bottom_position, x, y, z);
				distance = cube.cubesDistance(target_position, bottom_position);
				ALGORITHM_LOG( "Moving to the correct face [" << x << "] [" << y << "] [" << z << "]" << " : " << distance );

				unsigned int move_turns = (unsigned int) (distance) / center_index;
				RubiksCube::CubeMove move = cube.faceBaseMove(RubiksCube::TopFace);
				move.depth = cube.getCubeLengthMinusOne();

				if ((current_face == RubiksCube::FrontFace) || (current_face == RubiksCube::BackFace))
				{
					sign = (x < 0.0f) ? -1 : 1;
				}
				else
				{
					sign = (z < 0.0f) ? -1 : 1;
				}

				sign *= ((current_face == RubiksCube::LeftFace) || (current_face == RubiksCube::BackFace)) ? -1 : 1;

				move.turns *= sign;
				for (unsigned int i = 0; i != move_turns; ++i)
				{
					cube.move(move);
				}

				//
				// Put the top cube on the old place (not always needed)
				//
				move_to_bottom.turns *= -1;
				for (unsigned int i = 0; i != move_to_bottom_turns; ++i)
				{
					cube.move(move_to_bottom);
				}
			}
		}
	}

	return cube.moves();
}

//
// Corners
//

bool RubiksCubeCorners::solved(RubiksCube const& rubics_cube)
{
	return solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, rubics_cube.getCubeLengthMinusOne(), 0), rubics_cube) &&
		   solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne(), 0), rubics_cube) &&
		   solvedCorner(rubics_cube.cubeCoordinatesToIndex(0,rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne()), rubics_cube) &&
		   solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne()), rubics_cube);
}

bool RubiksCubeCorners::solvedCorner(unsigned int corner, RubiksCube const& rubics_cube)
{
	bool solved = true;

	unsigned int const center_index = rubics_cube.getCenterIndex();

	RubiksCube::Cube const& cube = rubics_cube.cube(corner);

	RubiksCube::FaceColor top_color = rubics_cube.cell(RubiksCube::TopFace, center_index, center_index);
	RubiksCube::FaceColor front_color = rubics_cube.cell(RubiksCube::FrontFace, center_index, center_index);
	RubiksCube::FaceColor back_color = rubics_cube.cell(RubiksCube::BackFace, center_index, center_index);
	RubiksCube::FaceColor right_color = rubics_cube.cell(RubiksCube::RightFace, center_index, center_index);
	RubiksCube::FaceColor left_color = rubics_cube.cell(RubiksCube::LeftFace, center_index, center_index);

	//solved &= (cube.getColor(RubiksCube::TopFace) != RubiksCube::Black);

	if (cube.getColor(RubiksCube::TopFace) != RubiksCube::Black)
	{
		solved &= (cube.getColor(RubiksCube::TopFace) == top_color);
	}

	if (cube.getColor(RubiksCube::FrontFace) != RubiksCube::Black)
	{
		solved &= (cube.getColor(RubiksCube::FrontFace) == front_color);
	}

	if (cube.getColor(RubiksCube::RightFace) != RubiksCube::Black)
	{
		solved &= (cube.getColor(RubiksCube::RightFace) == right_color);
	}

	if (cube.getColor(RubiksCube::LeftFace) != RubiksCube::Black)
	{
		solved &= (cube.getColor(RubiksCube::LeftFace) == left_color);
	}

	if (cube.getColor(RubiksCube::BackFace) != RubiksCube::Black)
	{
		solved &= (cube.getColor(RubiksCube::BackFace) == back_color);
	}

	return solved;
}

void RubiksCubeCorners::rotationFaceForCorner(RubiksCube const& rubics_cube, unsigned int const& corner, RubiksCube::Face& face, int& sign) const
{
	if (corner == rubics_cube.cubeCoordinatesToIndex(0, rubics_cube.getCubeLengthMinusOne(), 0))
	{
		face = RubiksCube::FrontFace;
		sign = -1;
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne(), 0))
	{
		face = RubiksCube::RightFace;
		sign = -1;
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(0, rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne()))
	{
		face = RubiksCube::LeftFace;
		sign = 1;
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne()))
	{
		face = RubiksCube::BackFace;
		sign = 1;
	}
}

bool RubiksCubeCorners::correctPositionForFlip(RubiksCube const& rubics_cube, unsigned int const& corner, unsigned int const& position)
{
	bool correct = false;

	if (corner == rubics_cube.cubeCoordinatesToIndex(0, rubics_cube.getCubeLengthMinusOne(), 0))
	{
		correct = (position == rubics_cube.cubeCoordinatesToIndex(0, 0, rubics_cube.getCubeLengthMinusOne()));
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne(), 0))
	{
		correct = (position == rubics_cube.cubeCoordinatesToIndex(0, 0, 0));
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(0, rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne()))
	{
		correct = (position == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, rubics_cube.getCubeLengthMinusOne()));
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne(), rubics_cube.getCubeLengthMinusOne()))
	{
		correct = (position == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, 0));
	}

	return correct;
}

RubiksCube::CubeMoves RubiksCubeCorners::operator()(RubiksCube& rubics_cube)
{
	RubiksCube cube(rubics_cube);
	cube.clearHistory();

	//
	// Ensure the top cross is solved
	//
	RubiksCubeCross cross;
	RubiksCube::CubeMoves moves = cross(rubics_cube);
	for (RubiksCube::CubeMoves::size_type i = 0; i != moves.size(); ++i)
	{
		cube.move(moves[i]);
	}

	typedef std::vector<unsigned int> CornerVector;
	typedef std::vector<RubiksCube::FaceColor> FaceColors;

	CornerVector corners;
	corners.push_back(cube.cubeCoordinatesToIndex(0, cube.getCubeLengthMinusOne(), 0));
	corners.push_back(cube.cubeCoordinatesToIndex(cube.getCubeLengthMinusOne(), cube.getCubeLengthMinusOne(), 0));
	corners.push_back(cube.cubeCoordinatesToIndex(0, cube.getCubeLengthMinusOne(), cube.getCubeLengthMinusOne()));
	corners.push_back(cube.cubeCoordinatesToIndex(cube.getCubeLengthMinusOne(), cube.getCubeLengthMinusOne(), cube.getCubeLengthMinusOne()));

	unsigned int const center_index = cube.getCenterIndex();

	while (!corners.empty())
	{
		unsigned int corner = corners.back();
		corners.pop_back();

		while (!solvedCorner(corner, cube))
		{

			//
			// find faces and colors
			//
			RubiksCube::Faces faces = cube.getFaces(corner);
			FaceColors face_color;
			for (RubiksCube::Faces::const_iterator i = faces.begin(); i != faces.end(); ++i)
			{
				face_color.push_back(cube.cell(*i, center_index, center_index));
			}

			//
			// find the current position of the cube
			//
			RubiksCube::Positions positions = cube.find(face_color[0], face_color[1], face_color[2], RubiksCube::CornerCube);
			RENGINE_ASSERT(positions.size() == 1);

			unsigned int found = positions.back();
			unsigned int x, y, z;
			cube.indexToCubeCoordinates(found, x, y, z);

			//
			// If it is at the top we must move it to the bottom
			//
			if (y == cube.getCubeLengthMinusOne())
			{
				RubiksCube::Face rotation_face;
				int sign = 0;
				rotationFaceForCorner(cube, found, rotation_face, sign);

				RubiksCube::CubeMove up_to_bottom_move = cube.faceBaseMove(rotation_face);
				up_to_bottom_move.turns = sign;
				cube.move(up_to_bottom_move);

				cube.move(cube.faceBaseMove(RubiksCube::BottomFace));

				up_to_bottom_move.turns = -sign;
				cube.move(up_to_bottom_move);
			}
			else if (correctPositionForFlip(cube, corner, found))
			{
				//
				// Do the flip baby :P
				//
				RubiksCube::Face rotation_face;
				int sign = 0;
				rotationFaceForCorner(cube, corner, rotation_face, sign);

				RubiksCube::CubeMove right_to_left_move = cube.faceBaseMove(RubiksCube::BottomFace);
				right_to_left_move.turns = -1;
				cube.move(right_to_left_move);

				RubiksCube::CubeMove up_to_bottom_move = cube.faceBaseMove(rotation_face);
				up_to_bottom_move.turns = sign;
				cube.move(up_to_bottom_move);

				cube.move(cube.faceBaseMove(RubiksCube::BottomFace));

				up_to_bottom_move.turns = -sign;
				cube.move(up_to_bottom_move);
			}
			else
			{
				//
				// The cube is on the bottom line bt not on the correct place
				//
				cube.move(cube.faceBaseMove(RubiksCube::BottomFace));
			}

			ALGORITHM_LOG( "Corner [" << corner << "] : Not Solved." );
		}
	}

	return cube.moves();
}

bool RubiksCubeMiddleLayer::solved(RubiksCube const& rubics_cube)
{
	unsigned int const center_index = rubics_cube.getCenterIndex();

	return RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, center_index, 0), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), center_index, 0), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, center_index, rubics_cube.getCubeLengthMinusOne()), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), center_index, rubics_cube.getCubeLengthMinusOne()), rubics_cube);
}

// 0 -- moves_target_to_left
// 1 -- moves_target_to_right
unsigned int RubiksCubeMiddleLayer::methodToUse(RubiksCube const& rubics_cube, unsigned int const& corner, RubiksCube::Face const& align_face)
{
	unsigned int method = 0;
	unsigned int const center_index = rubics_cube.getCenterIndex();

	if (corner == rubics_cube.cubeCoordinatesToIndex(0, center_index, 0))
	{
		if (align_face == RubiksCube::LeftFace)
		{
			method = 1;
		}
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), center_index, 0))
	{
		if (align_face == RubiksCube::FrontFace)
		{
			method = 1;
		}
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(0, center_index, rubics_cube.getCubeLengthMinusOne()))
	{
		if (align_face == RubiksCube::BackFace)
		{
			method = 1;
		}
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), center_index, rubics_cube.getCubeLengthMinusOne()))
	{
		if (align_face == RubiksCube::RightFace)
		{
			method = 1;
		}
	}

	return method;
}

void RubiksCubeMiddleLayer::facesToUse(RubiksCube const& rubics_cube, unsigned int const& corner, RubiksCube::Face& left_face, RubiksCube::Face& right_face)
{
	unsigned int const center_index = rubics_cube.getCenterIndex();

	if (corner == rubics_cube.cubeCoordinatesToIndex(0, center_index, 0))
	{
		left_face = RubiksCube::LeftFace;
		right_face = RubiksCube::FrontFace;
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), center_index, 0))
	{
		left_face = RubiksCube::FrontFace;
		right_face = RubiksCube::RightFace;
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(0, center_index, rubics_cube.getCubeLengthMinusOne()))
	{
		left_face = RubiksCube::BackFace;
		right_face = RubiksCube::LeftFace;
	}
	else if (corner == rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), center_index, rubics_cube.getCubeLengthMinusOne()))
	{
		left_face = RubiksCube::RightFace;
		right_face = RubiksCube::BackFace;
	}
}

void RubiksCubeMiddleLayer::applyMethod(RubiksCube& rubics_cube, RubiksCube::Face const& left_face, RubiksCube::Face const& right_face, unsigned int method)
{
	int left_sign = ((left_face == RubiksCube::FrontFace) || (left_face == RubiksCube::RightFace)) ? -1 : 1;
	int right_sign = ((right_face == RubiksCube::FrontFace) || (right_face == RubiksCube::RightFace)) ? 1 : -1;

	if (method == 0)
	{
		ALGORITHM_LOG( "RubiksCubeMiddleLayer Method: 0" );

		RubiksCube::CubeMove bottom = rubics_cube.faceBaseMove(RubiksCube::BottomFace);
		bottom.turns = -1;
		rubics_cube.move(bottom);

		RubiksCube::CubeMove left = rubics_cube.faceBaseMove(left_face);
		left.turns = -1 * left_sign;
		rubics_cube.move(left);

		bottom.turns = 1;
		rubics_cube.move(bottom);

		left.turns = 1 * left_sign;
		rubics_cube.move(left);

		bottom.turns = 1;
		rubics_cube.move(bottom);

		RubiksCube::CubeMove right = rubics_cube.faceBaseMove(right_face);
		right.turns = -1 * right_sign;
		rubics_cube.move(right);

		bottom.turns = -1;
		rubics_cube.move(bottom);

		right.turns = 1 * right_sign;
		rubics_cube.move(right);
	}
	else
	{
		ALGORITHM_LOG( "RubiksCubeMiddleLayer Method: 1" );

		RubiksCube::CubeMove bottom = rubics_cube.faceBaseMove(RubiksCube::BottomFace);
		bottom.turns = 1;
		rubics_cube.move(bottom);

		RubiksCube::CubeMove right = rubics_cube.faceBaseMove(right_face);
		right.turns = -1 * right_sign;
		rubics_cube.move(right);

		bottom.turns = -1;
		rubics_cube.move(bottom);

		right.turns = 1 * right_sign;
		rubics_cube.move(right);

		bottom.turns = -1;
		rubics_cube.move(bottom);

		RubiksCube::CubeMove left = rubics_cube.faceBaseMove(left_face);
		left.turns = -1 * left_sign;
		rubics_cube.move(left);

		bottom.turns = 1;
		rubics_cube.move(bottom);

		left.turns = 1 * left_sign;
		rubics_cube.move(left);
	}

}

RubiksCube::CubeMoves RubiksCubeMiddleLayer::operator()(RubiksCube& rubics_cube)
{
	RubiksCube cube(rubics_cube);
	cube.clearHistory();

	//
	// Ensure the corners are solved
	//
	RubiksCubeCorners corners_solver;
	RubiksCube::CubeMoves moves = corners_solver(rubics_cube);
	for (RubiksCube::CubeMoves::size_type i = 0; i != moves.size(); ++i)
	{
		cube.move(moves[i]);
	}

	typedef std::vector<unsigned int> CornerVector;
	typedef std::vector<RubiksCube::FaceColor> FaceColors;

	unsigned int const center_index = cube.getCenterIndex();

	CornerVector corners;
	corners.push_back(cube.cubeCoordinatesToIndex(0, center_index, 0));
	corners.push_back(cube.cubeCoordinatesToIndex(cube.getCubeLengthMinusOne(), center_index, 0));
	corners.push_back(cube.cubeCoordinatesToIndex(0, center_index, cube.getCubeLengthMinusOne()));
	corners.push_back(cube.cubeCoordinatesToIndex(cube.getCubeLengthMinusOne(), center_index, cube.getCubeLengthMinusOne()));

	while (!corners.empty())
	{
		unsigned int corner = corners.back();
		corners.pop_back();

		while (!RubiksCubeCorners::solvedCorner(corner, cube))
		{
			//
			// find faces and colors
			//
			RubiksCube::Faces faces = cube.getFaces(corner);
			FaceColors face_color;
			for (RubiksCube::Faces::const_iterator i = faces.begin(); i != faces.end(); ++i)
			{
				face_color.push_back(cube.cell(*i, center_index, center_index));
			}

			//
			// find the current position of the cube
			//
			RubiksCube::Positions positions = cube.find(face_color[0], face_color[1], RubiksCube::Black, RubiksCube::CentralLineCube);
			RENGINE_ASSERT(positions.size() == 1);

			unsigned int found = positions.back();
			unsigned int x, y, z;
			cube.indexToCubeCoordinates(found, x, y, z);

			RENGINE_ASSERT(y != cube.getCubeLengthMinusOne());

			if (y == 0) // on the lower ring
			{

				//
				// Check if is aligned with the correct face color
				//
				bool is_aligned = false;
				FaceColors::size_type align_face = 0;

				for (; (align_face != faces.size()) && !is_aligned; ++align_face)
				{
					is_aligned = (cube.cube(found).getColor(faces[align_face]) == face_color[align_face]);
				}
				RENGINE_ASSERT(align_face > 0);
				align_face--;

				if (is_aligned)
				{
					unsigned int method = methodToUse(cube, corner, faces[align_face]);
					RubiksCube::Face left_face, right_face;
					facesToUse(cube, corner, left_face, right_face);

					applyMethod(cube, left_face, right_face, method);

					ALGORITHM_LOG( "MiddleLayer Corner is aligned." );
				}
				else
				{
					cube.move(cube.faceBaseMove(RubiksCube::BottomFace));
				}
			}
			else
			{
				unsigned int method = 0;
				RubiksCube::Face left_face, right_face;
				facesToUse(cube, found, left_face, right_face);

				applyMethod(cube, left_face, right_face, method);
			}

		}
	}

	return cube.moves();
}

bool RubiksCubeBottomCross::solved(RubiksCube const& rubics_cube)
{
	bool cube_solved = false;

	unsigned int const center_index = rubics_cube.getCenterIndex();
	unsigned int const length_minus_one = rubics_cube.getCubeLengthMinusOne();

	RubiksCube::FaceColor face_color = rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index);

	cube_solved = ((rubics_cube.cell(RubiksCube::BottomFace, 0, center_index) == face_color) &&
			       (rubics_cube.cell(RubiksCube::BottomFace, length_minus_one, center_index) == face_color) &&
			       (rubics_cube.cell(RubiksCube::BottomFace, center_index, length_minus_one) == face_color) &&
			       (rubics_cube.cell(RubiksCube::BottomFace, center_index, 0) == face_color));

	return cube_solved;
}

//
//    0       1       2       3       4       5       6
// [0,0,0] [0,X,0] [0,0,0] [0,X,0] [0,X,0] [0,0,0] [0,0,0]
// [0,X,0] [0,X,0] [X,X,X] [X,X,0] [0,X,X] [0,X,X] [X,X,0]
// [0,0,0] [0,X,0] [0,0,0] [0,0,0] [0,0,0] [0,X,0] [0,X,0]

bool RubiksCubeBottomCross::patternFound(RubiksCube const& rubics_cube, unsigned int pattern)
{
	bool pattern_found = false;

	unsigned int const center_index = rubics_cube.getCenterIndex();
	unsigned int const length_minus_one = rubics_cube.getCubeLengthMinusOne();

	RubiksCube::FaceColor face_color = rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index);

	if (pattern == 0)
	{
		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, center_index) != face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, length_minus_one, center_index) != face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, center_index, length_minus_one) != face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, center_index, 0) != face_color));
	}
	else if (pattern == 1)
	{
		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, center_index, length_minus_one) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, center_index, 0) == face_color));
	}
	else if (pattern == 2)
	{
		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, length_minus_one, center_index) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 0, center_index) == face_color));
	}
	else if (pattern == 3)
	{
		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, center_index, length_minus_one) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 0, center_index) == face_color));
	}
	else if (pattern == 4)
	{
		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, center_index, length_minus_one) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, length_minus_one, center_index) == face_color));
	}
	else if (pattern == 5)
	{
		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, center_index, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, length_minus_one, center_index) == face_color));
	}
	else if (pattern == 6)
	{
		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, center_index, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 0, center_index) == face_color));
	}

	return pattern_found;
}

void RubiksCubeBottomCross::facesToUse(RubiksCube const& rubics_cube, unsigned int const& pattern, RubiksCube::Face& left_face, RubiksCube::Face& right_face)
{
	if ((pattern == 0) || (pattern == 2) || (pattern == 5))
	{
		left_face = RubiksCube::LeftFace;
		right_face = RubiksCube::FrontFace;
	}
	else if ((pattern == 1) || (pattern == 6))
	{
		left_face = RubiksCube::FrontFace;
		right_face = RubiksCube::RightFace;
	}
	else if (pattern == 3)
	{
		left_face = RubiksCube::RightFace;
		right_face = RubiksCube::BackFace;
	}
	else if (pattern == 4)
	{
		left_face = RubiksCube::BackFace;
		right_face = RubiksCube::LeftFace;
	}
}

RubiksCube::CubeMoves RubiksCubeBottomCross::operator()(RubiksCube& rubics_cube)
{
	RubiksCube cube(rubics_cube);
	cube.clearHistory();

	//
	// Ensure the middle layers is solved
	//
	RubiksCubeMiddleLayer solver;
	RubiksCube::CubeMoves moves = solver(rubics_cube);
	for (RubiksCube::CubeMoves::size_type i = 0; i != moves.size(); ++i)
	{
		cube.move(moves[i]);
	}

	while (!solved(cube))
	{
		bool found = false;
		unsigned int pattern_found = 0;

		while (!found)
		{
			found = patternFound(cube, pattern_found);

			if (!found)
			{
				++pattern_found;
			}
		}

		ALGORITHM_LOG( "Pattern Found: " << pattern_found );

		RubiksCube::Face left_face, right_face;
		facesToUse(cube, pattern_found, left_face, right_face);

		int left_sign = ((left_face == RubiksCube::FrontFace) || (left_face == RubiksCube::RightFace)) ? -1 : 1;
		int right_sign = ((right_face == RubiksCube::FrontFace) || (right_face == RubiksCube::RightFace)) ? 1 : -1;

		RubiksCube::CubeMove left = cube.faceBaseMove(left_face);
		left.turns = left_sign;
		cube.move(left);

		RubiksCube::CubeMove bottom = cube.faceBaseMove(RubiksCube::BottomFace);
		bottom.turns = 1;
		cube.move(bottom);

		RubiksCube::CubeMove right = cube.faceBaseMove(right_face);
		right.turns = -1 * right_sign;
		cube.move(right);

		bottom.turns = -1;
		cube.move(bottom);

		right.turns = right_sign;
		cube.move(right);

		left.turns = -1 * left_sign;
		cube.move(left);

	}

	return cube.moves();
}

bool RubiksCubeBottomCorners::solved(RubiksCube const& rubics_cube)
{
	unsigned int const center_index = rubics_cube.getCenterIndex();

	RubiksCube::FaceColor face_color = rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index);

	bool solved = true;

	for (unsigned int x = 0; (x != rubics_cube.getCubeLength()) && solved; ++x)
	{
		for (unsigned int y = 0; (y != rubics_cube.getCubeLength()) && solved; ++y)
		{
			solved &= (rubics_cube.cell(RubiksCube::BottomFace, x, y) == face_color);
		}
	}

	return solved;
}

//
//    0           1          2          3          4           5         6
//
//       X      X          X   X          X          X                 X
// X[0,X,0]    [0,X,X]    [0,X,0]    [X,X,0]    [X,X,0]   X[0,X,0]X   [0,X,0]X
//  [X,X,X]    [X,X,X]    [X,X,X]    [X,X,X]    [X,X,X]    [X,X,X]    [X,X,X]
//  [X,X,0]X  X[0,X,0]    [X,X,X]    [X,X,0]   X[0,X,X]   X[0,X,0]X   [0,X,0]X
//                  X                     X                            X
//
// Move: 1  others : Move: 2
//

bool RubiksCubeBottomCorners::patternFound(RubiksCube const& rubics_cube, unsigned int pattern)
{
	bool pattern_found = false;

	unsigned int const center_index = rubics_cube.getCenterIndex();

	RubiksCube::FaceColor face_color = rubics_cube.cell(RubiksCube::BottomFace, center_index, center_index);

	if (pattern == 0)
	{

		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 0, 1) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 0, 2) != face_color) &&

				         (rubics_cube.cell(RubiksCube::BottomFace, 1, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 1, 1) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 1, 2) == face_color) &&

				         (rubics_cube.cell(RubiksCube::BottomFace, 2, 0) != face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 2, 1) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 2, 2) != face_color) &&

				         (rubics_cube.cell(RubiksCube::FrontFace, 2, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::RightFace, 2, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::LeftFace,  2, 0) == face_color)

		);
	}
	else if (pattern == 1)
	{

		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, 0) != face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 0, 1) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 0, 2) != face_color) &&

				         (rubics_cube.cell(RubiksCube::BottomFace, 1, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 1, 1) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 1, 2) == face_color) &&

				         (rubics_cube.cell(RubiksCube::BottomFace, 2, 0) != face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 2, 1) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BottomFace, 2, 2) == face_color) &&

				         (rubics_cube.cell(RubiksCube::FrontFace, 0, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::BackFace, 0, 0) == face_color) &&
				         (rubics_cube.cell(RubiksCube::LeftFace, 0, 0) == face_color));
	}
	else if (pattern == 2)
	{

		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 0, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 0, 2) != face_color) &&

						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 2) == face_color) &&

						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 2) != face_color) &&

						 (rubics_cube.cell(RubiksCube::FrontFace, 0, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::FrontFace, 2, 0) == face_color));
	}
	else if (pattern == 3)
	{

		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 0, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 0, 2) == face_color) &&

						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 2) == face_color) &&

						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 0) != face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 2) != face_color) &&

						 (rubics_cube.cell(RubiksCube::BackFace, 0, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::FrontFace, 2, 0) == face_color));
	}
	else if (pattern == 4)
	{

		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, 0) != face_color) &&
				(rubics_cube.cell(RubiksCube::BottomFace, 0, 1) == face_color) &&
				(rubics_cube.cell(RubiksCube::BottomFace, 0, 2) == face_color) &&

				(rubics_cube.cell(RubiksCube::BottomFace, 1, 0) == face_color) &&
				(rubics_cube.cell(RubiksCube::BottomFace, 1, 1) == face_color) &&
				(rubics_cube.cell(RubiksCube::BottomFace, 1, 2) == face_color) &&

				(rubics_cube.cell(RubiksCube::BottomFace, 2, 0) == face_color) &&
				(rubics_cube.cell(RubiksCube::BottomFace, 2, 1) == face_color) &&
				(rubics_cube.cell(RubiksCube::BottomFace, 2, 2) != face_color) &&

				(rubics_cube.cell(RubiksCube::LeftFace, 0, 0) == face_color) &&
				(rubics_cube.cell(RubiksCube::FrontFace, 2, 0) == face_color));
	}
	else if (pattern == 5)
	{

		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, 0) != face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 0, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 0, 2) != face_color) &&

						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 1, 2) == face_color) &&

						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 0) != face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 1) == face_color) &&
						 (rubics_cube.cell(RubiksCube::BottomFace, 2, 2) != face_color) &&

						 (rubics_cube.cell(RubiksCube::RightFace, 0, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::RightFace, 2, 0) == face_color) &&

						 (rubics_cube.cell(RubiksCube::LeftFace, 0, 0) == face_color) &&
						 (rubics_cube.cell(RubiksCube::LeftFace, 2, 0) == face_color));
	}
	else if (pattern == 6)
	{

		pattern_found = ((rubics_cube.cell(RubiksCube::BottomFace, 0, 0) != face_color) &&
					     (rubics_cube.cell(RubiksCube::BottomFace, 0, 1) == face_color) &&
					     (rubics_cube.cell(RubiksCube::BottomFace, 0, 2) != face_color) &&

					     (rubics_cube.cell(RubiksCube::BottomFace, 1, 0) == face_color) &&
					     (rubics_cube.cell(RubiksCube::BottomFace, 1, 1) == face_color) &&
					     (rubics_cube.cell(RubiksCube::BottomFace, 1, 2) == face_color) &&

					     (rubics_cube.cell(RubiksCube::BottomFace, 2, 0) != face_color) &&
					     (rubics_cube.cell(RubiksCube::BottomFace, 2, 1) == face_color) &&
					     (rubics_cube.cell(RubiksCube::BottomFace, 2, 2) != face_color) &&

					     (rubics_cube.cell(RubiksCube::RightFace, 0, 0) == face_color) &&
					     (rubics_cube.cell(RubiksCube::RightFace, 2, 0) == face_color) &&

					     (rubics_cube.cell(RubiksCube::FrontFace, 0, 0) == face_color) &&
					     (rubics_cube.cell(RubiksCube::BackFace, 2, 0) == face_color));
	}

	return pattern_found;
}

RubiksCube::CubeMoves RubiksCubeBottomCorners::operator()(RubiksCube& rubics_cube)
{
	RubiksCube cube(rubics_cube);
	cube.clearHistory();

	RubiksCubeBottomCross solver;
	RubiksCube::CubeMoves moves = solver(rubics_cube);
	for (RubiksCube::CubeMoves::size_type i = 0; i != moves.size(); ++i)
	{
		cube.move(moves[i]);
	}

	while (!solved(cube))
	{
		bool found = false;

		unsigned int const total_patterns = 7;
		unsigned int pattern_found = 0;

		unsigned int rotation = 0;

		while (!found)
		{
			RENGINE_ASSERT(rotation < 4);

			// test all patterns
			pattern_found = 0;
			while (!found && (pattern_found != total_patterns))
			{
				found = patternFound(cube, pattern_found);
				ALGORITHM_LOG( "Pattern Test: " << pattern_found << " [" << found << "]");
				if (!found)
				{
					++pattern_found;
				}
			}

			if (!found)
			{
				ALGORITHM_LOG( "Trying another rotation: " << rotation);
				cube.move(cube.faceBaseMove(RubiksCube::BottomFace));
				++rotation;
			}

		}

		if (rotation != 0)
		{
			ALGORITHM_LOG( "Fixing rotation " << rotation);
			for (unsigned int i = rotation; i != 4; ++i)
			{
				cube.move(cube.faceBaseMove(RubiksCube::BottomFace));
			}

		}

		ALGORITHM_LOG( "Pattern Found: " << pattern_found );

		RubiksCube::Face left_face = RubiksCube::LeftFace;
		RubiksCube::Face right_face = RubiksCube::FrontFace;

		if (rotation == 1)
		{
			left_face = RubiksCube::FrontFace;
			right_face = RubiksCube::RightFace;
		}
		else if (rotation == 2)
		{
			left_face = RubiksCube::RightFace;
			right_face = RubiksCube::BackFace;
		}
		else if (rotation == 3)
		{
			left_face = RubiksCube::BackFace;
			right_face = RubiksCube::LeftFace;
		}

		int left_sign = ((left_face == RubiksCube::FrontFace) || (left_face == RubiksCube::RightFace)) ? -1 : 1;
		int right_sign = ((right_face == RubiksCube::FrontFace) || (right_face == RubiksCube::RightFace)) ? 1 : -1;

		if (pattern_found == 0)
		{
			RubiksCube::CubeMove right = cube.faceBaseMove(right_face);
			right.turns = -1 * right_sign;
			cube.move(right);

			RubiksCube::CubeMove bottom = cube.faceBaseMove(RubiksCube::BottomFace);
			bottom.turns = 1;
			cube.move(bottom);

			right.turns = right_sign;
			cube.move(right);

			bottom.turns = 1;
			cube.move(bottom);

			right.turns = -1 * right_sign;
			cube.move(right);

			bottom.turns = 1;
			cube.move(bottom);

			bottom.turns = 1;
			cube.move(bottom);

			right.turns = right_sign;
			cube.move(right);
		}
		else
		{
			RubiksCube::CubeMove left = cube.faceBaseMove(left_face);
			left.turns = -1 * left_sign;
			cube.move(left);

			RubiksCube::CubeMove bottom = cube.faceBaseMove(RubiksCube::BottomFace);
			bottom.turns = -1;
			cube.move(bottom);

			left.turns = left_sign;
			cube.move(left);

			bottom.turns = -1;
			cube.move(bottom);

			left.turns = -1 * left_sign;
			cube.move(left);

			bottom.turns = -1;
			cube.move(bottom);

			bottom.turns = -1;
			cube.move(bottom);

			left.turns = left_sign;
			cube.move(left);
		}
	}

	return cube.moves();
}

bool RubiksCubeBottomCubeCorners::solved(RubiksCube const& rubics_cube)
{
	return RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, 0, 0), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, 0), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, 0, rubics_cube.getCubeLengthMinusOne()), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, rubics_cube.getCubeLengthMinusOne()), rubics_cube);

}
/*
 bool RubiksCubeBottomCubeCorners::solvedCorner(unsigned int corner, RubiksCube const& rubics_cube)
 {

 return RubiksCubeCorners::()
 bool solved = true;

 unsigned int const center_index = rubics_cube.getCenterIndex();

 RubiksCube::Cube const& cube = rubics_cube.cube(corner);

 RubiksCube::FaceColor front_color = rubics_cube.cell(RubiksCube::FrontFace, center_index, center_index);
 RubiksCube::FaceColor back_color = rubics_cube.cell(RubiksCube::BackFace, center_index, center_index);
 RubiksCube::FaceColor right_color = rubics_cube.cell(RubiksCube::RightFace, center_index, center_index);
 RubiksCube::FaceColor left_color = rubics_cube.cell(RubiksCube::LeftFace, center_index, center_index);


 if (cube.getColor(RubiksCube::FrontFace) != RubiksCube::Black)
 {
 solved &= (cube.getColor(RubiksCube::FrontFace) == front_color);
 }

 if (cube.getColor(RubiksCube::RightFace) != RubiksCube::Black)
 {
 solved &= (cube.getColor(RubiksCube::RightFace) == right_color);
 }

 if (cube.getColor(RubiksCube::LeftFace) != RubiksCube::Black)
 {
 solved &= (cube.getColor(RubiksCube::LeftFace) == left_color);
 }

 if (cube.getColor(RubiksCube::BackFace) != RubiksCube::Black)
 {
 solved &= (cube.getColor(RubiksCube::BackFace) == back_color);
 }

 return solved;
 }
 */
bool RubiksCubeBottomCubeCorners::patternFound(RubiksCube const& rubics_cube, unsigned int pattern)
{
	bool pattern_found = false;

	unsigned int const center_index = rubics_cube.getCenterIndex();

	if (pattern == 0)
	{
		pattern_found = RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, 0), rubics_cube);

		if (pattern_found)
		{
			RubiksCube::FaceColor front_color = rubics_cube.cell(RubiksCube::FrontFace, center_index, center_index);
			RubiksCube::FaceColor right_color = rubics_cube.cell(RubiksCube::RightFace, center_index, center_index);

			RubiksCube::Cube const& cube_0 = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(0, 0, 0));
			RubiksCube::Cube const& cube_1 = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(0, 0, rubics_cube.getCubeLengthMinusOne()));

			pattern_found = ((cube_0.getColor(RubiksCube::LeftFace) == right_color) & (cube_1.getColor(RubiksCube::LeftFace) == front_color));
		}

	}
	/*
	 else if (pattern == 1)
	 {
	 pattern_found = solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, 0, 0), rubics_cube);

	 if (pattern_found)
	 {
	 RubiksCube::FaceColor front_color = rubics_cube.cell(RubiksCube::FrontFace, center_index, center_index);
	 RubiksCube::FaceColor left_color  = rubics_cube.cell(RubiksCube::LeftFace, center_index, center_index);

	 RubiksCube::Cube const& cube_0 = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, 0));
	 RubiksCube::Cube const& cube_1 = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, rubics_cube.getCubeLengthMinusOne()));

	 pattern_found = ( (cube_0.getColor(RubiksCube::RightFace) == left_color) & (cube_1.getColor(RubiksCube::RightFace) == front_color) );
	 }
	 }
	 */
	else if (pattern == 2)
	{
		pattern_found = ((RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, 0, 0), rubics_cube) && RubiksCubeCorners::solvedCorner(
				rubics_cube.cubeCoordinatesToIndex(0, 0, rubics_cube.getCubeLengthMinusOne()), rubics_cube)));

	}
	else if (pattern == 3)
	{
		pattern_found = ((RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, 0, 0), rubics_cube) && RubiksCubeCorners::solvedCorner(
				rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, rubics_cube.getCubeLengthMinusOne()), rubics_cube)) ||

		(RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, 0), rubics_cube) && RubiksCubeCorners::solvedCorner(
				rubics_cube.cubeCoordinatesToIndex(0, 0, rubics_cube.getCubeLengthMinusOne()), rubics_cube)));
	}

	return pattern_found;
}

RubiksCube::CubeMoves RubiksCubeBottomCubeCorners::operator()(RubiksCube& rubics_cube)
{
	RubiksCube cube(rubics_cube);
	cube.clearHistory();

	RubiksCubeBottomCorners solver;
	RubiksCube::CubeMoves moves = solver(rubics_cube);
	for (RubiksCube::CubeMoves::size_type i = 0; i != moves.size(); ++i)
	{
		cube.move(moves[i]);
	}

	while (!solved(cube))
	{
		bool found = false;

		unsigned int pattern = 0;
		unsigned int all_rotation = 0;
		unsigned int rotation = 0;

		while (!found)
		{
			RENGINE_ASSERT(pattern < 4);

			all_rotation = 0;
			while (all_rotation != 4 && !found)
			{
				rotation = 0;
				while (rotation != 4 && !found)
				{
					found = patternFound(cube, pattern);

					if (!found)
					{
						ALGORITHM_LOG( "Trying another rotation: " << rotation);
						RubiksCube::CubeMove move = cube.faceBaseMove(RubiksCube::BottomFace);
						cube.move(move);
						++rotation;
					}
				}

				if (rotation == 4)
				{
					rotation = 0;
				}

				if (rotation != 0)
				{
					ALGORITHM_LOG( "Fixing rotation " << rotation);
					for (unsigned int i = 0; i != rotation; ++i)
					{
						RubiksCube::CubeMove move = cube.faceBaseMove(RubiksCube::BottomFace);
						move.turns = -1;
						cube.move(move);
					}
				}

				if (!found)
				{
					ALGORITHM_LOG( "Trying another All rotation: " << all_rotation);
					RubiksCube::CubeMove move;
					move.turns = 1;
					move.movement = RubiksCube::Up;

					move.depth = 0;
					cube.move(move);

					move.depth = 1;
					cube.move(move);

					move.depth = 2;
					cube.move(move);
					++all_rotation;
				}
			}

			if (all_rotation != 0)
			{
				ALGORITHM_LOG( "Fixing All rotation " << all_rotation);
				for (unsigned int i = 0; i != all_rotation; ++i)
				{
					RubiksCube::CubeMove move;
					move.turns = -1;
					move.movement = RubiksCube::Up;

					move.depth = 2;
					cube.move(move);

					move.depth = 1;
					cube.move(move);

					move.depth = 0;
					cube.move(move);
				}
			}

			if (!found)
			{
				++pattern;
			}
		}

		//		if (pattern == 2)
		//		{
		//			ALGORITHM_LOG( "RENGINE_ASSERT(pattern != 2); " );
		//		}
		//
		//      the cube may be finished

		if (rotation != 0)
		{
			ALGORITHM_LOG( "Replaying rotation " << rotation);
			for (unsigned int i = 0; i != rotation; ++i)
			{
				RubiksCube::CubeMove move = cube.faceBaseMove(RubiksCube::BottomFace);
				cube.move(move);
			}
		}

		if (all_rotation == 4)
		{
			all_rotation = 0;
		}

		ALGORITHM_LOG( "Pattern Found: " << pattern << " Rotation: " << rotation << " All Rotation: " << all_rotation);

		RubiksCube::Face left_face = RubiksCube::LeftFace;
		RubiksCube::Face right_face = RubiksCube::FrontFace;
		RubiksCube::Face back_face = RubiksCube::BackFace;

		if (all_rotation == 1)
		{
			left_face = RubiksCube::FrontFace;
			right_face = RubiksCube::RightFace;
			back_face = RubiksCube::LeftFace;
		}
		else if (all_rotation == 2)
		{
			left_face = RubiksCube::RightFace;
			right_face = RubiksCube::BackFace;
			back_face = RubiksCube::FrontFace;
		}
		else if (all_rotation == 3)
		{
			left_face = RubiksCube::BackFace;
			right_face = RubiksCube::LeftFace;
			back_face = RubiksCube::RightFace;
		}

		int left_sign = ((left_face == RubiksCube::FrontFace) || (left_face == RubiksCube::RightFace)) ? -1 : 1;
		int right_sign = ((right_face == RubiksCube::FrontFace) || (right_face == RubiksCube::RightFace)) ? 1 : -1;
		int back_sign = ((back_face == RubiksCube::FrontFace) || (back_face == RubiksCube::RightFace)) ? -1 : 1;

		//		ALGORITHM_LOG( "left_sign: " << left_sign << " right_sign: " << right_sign << " back_sign: " << back_sign);

		if ((pattern == 0) || (pattern == 3))
		{
			// method 1
			RubiksCube::CubeMove left = cube.faceBaseMove(left_face);
			left.turns = 1 * left_sign;
			cube.move(left);

			RubiksCube::CubeMove front = cube.faceBaseMove(right_face);
			front.turns = 1 * right_sign;
			cube.move(front);

			left.turns = 1 * left_sign;
			cube.move(left);

			RubiksCube::CubeMove back = cube.faceBaseMove(back_face);
			back.turns = -1 * back_sign;
			cube.move(back);
			cube.move(back);

			left.turns = -1 * left_sign;
			cube.move(left);

			front.turns = -1 * right_sign;
			cube.move(front);

			left.turns = 1 * left_sign;
			cube.move(left);

			back.turns = -1 * back_sign;
			cube.move(back);
			cube.move(back);

			left.turns = -1 * left_sign;
			cube.move(left);
			cube.move(left);
		}

		/*		else if (pattern_found == 1)
		 {
		 }
		 */

	}

	return cube.moves();
}

bool RubiksCubeBottomEdges::solved(RubiksCube const& rubics_cube)
{
	unsigned int const center_index = rubics_cube.getCenterIndex();

	return RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(center_index, 0, 0), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(center_index, 0, rubics_cube.getCubeLengthMinusOne()), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, 0, center_index), rubics_cube) &&
		   RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, center_index), rubics_cube);
}

bool RubiksCubeBottomEdges::patternFound(RubiksCube const& rubics_cube, unsigned int pattern)
{
	unsigned solved_cubes = 0;
	unsigned int const center_index = rubics_cube.getCenterIndex();

	if (RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(center_index, 0, 0), rubics_cube))
	{
		solved_cubes++;
	}

	if (RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(center_index, 0, rubics_cube.getCubeLengthMinusOne()), rubics_cube))
	{
		solved_cubes++;
	}

	if (RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(0, 0, center_index), rubics_cube))
	{
		solved_cubes++;
	}

	if (RubiksCubeCorners::solvedCorner(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, center_index), rubics_cube))
	{
		solved_cubes++;
	}

	if ((pattern == 0) && (solved_cubes == 4)) // all solved
	{
		return true;
	}
	else if ((pattern == 1) && (solved_cubes == 0)) // 4 edges wrong
	{
		return true;
	}
	else if (solved_cubes == 1)
	{

		if (pattern == 2)
		{
			RubiksCube::Cube const& front_cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(center_index, 0, 0));
			RubiksCube::Cube const& left_cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(0, 0, center_index));
			RubiksCube::Cube const& back_cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(center_index, 0, rubics_cube.getCubeLengthMinusOne()));

			RubiksCube::FaceColor right_color = rubics_cube.cell(RubiksCube::RightFace, center_index, center_index);
			RubiksCube::FaceColor front_color = rubics_cube.cell(RubiksCube::FrontFace, center_index, center_index);
			RubiksCube::FaceColor left_color = rubics_cube.cell(RubiksCube::LeftFace, center_index, center_index);

			return ( (front_cube.getColor(RubiksCube::FrontFace) == right_color) ||
					 (left_cube.getColor(RubiksCube::LeftFace) == front_color) ||
					 (back_cube.getColor(RubiksCube::BackFace) == left_color)
				   );
		}
		else if (pattern == 3)
		{
			RubiksCube::Cube const& front_cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(center_index, 0, 0));
			RubiksCube::Cube const& left_cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(0, 0, center_index));
			RubiksCube::Cube const& back_cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(center_index, 0, rubics_cube.getCubeLengthMinusOne()));

			RubiksCube::FaceColor right_color = rubics_cube.cell(RubiksCube::RightFace, center_index, center_index);
			RubiksCube::FaceColor back_color = rubics_cube.cell(RubiksCube::BackFace, center_index, center_index);
			RubiksCube::FaceColor left_color = rubics_cube.cell(RubiksCube::LeftFace, center_index, center_index);

			return ( (front_cube.getColor(RubiksCube::FrontFace) == left_color) ||
					 (left_cube.getColor(RubiksCube::LeftFace) == back_color) ||
				     (back_cube.getColor(RubiksCube::BackFace) == right_color)
				   );
		}
	}

	return false;
}

unsigned int RubiksCubeBottomEdges::getRotation(RubiksCube const& rubics_cube)
{
	unsigned int rotation = 0;

	unsigned int const center_index = rubics_cube.getCenterIndex();

	RubiksCube::FaceColor front_color = rubics_cube.cell(RubiksCube::FrontFace, center_index, center_index);
	RubiksCube::FaceColor right_color = rubics_cube.cell(RubiksCube::RightFace, center_index, center_index);
	RubiksCube::FaceColor left_color = rubics_cube.cell(RubiksCube::LeftFace, center_index, center_index);


	{
		RubiksCube::Cube const& cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(rubics_cube.getCubeLengthMinusOne(), 0, center_index));

		if (cube.getColor(RubiksCube::RightFace) == right_color)
		{
			rotation = 3;
		}
	}

	{
		RubiksCube::Cube const& cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(center_index, 0, 0));

		if (cube.getColor(RubiksCube::FrontFace) == front_color)
		{
			rotation = 2;
		}
	}

	{
		RubiksCube::Cube const& cube = rubics_cube.cube(rubics_cube.cubeCoordinatesToIndex(0, 0, center_index));

		if (cube.getColor(RubiksCube::LeftFace) == left_color)
		{
			rotation = 1;
		}
	}


	return rotation;
}

RubiksCube::CubeMoves RubiksCubeBottomEdges::operator()(RubiksCube& rubics_cube)
{
	RubiksCube cube(rubics_cube);
	cube.clearHistory();

	RubiksCubeBottomCubeCorners solver;
	RubiksCube::CubeMoves moves = solver(rubics_cube);
	for (RubiksCube::CubeMoves::size_type i = 0; i != moves.size(); ++i)
	{
		cube.move(moves[i]);
	}

	while (!solved(cube))
	{


		bool found = false;
		unsigned int pattern_found = 0;

		while (!found)
		{
			RENGINE_ASSERT(pattern_found < 4);

			found = patternFound(cube, pattern_found);

			if (!found)
			{
				++pattern_found;
			}
		}

		ALGORITHM_LOG( "Pattern Found: " << pattern_found);

		unsigned int rotation = getRotation(cube);

		ALGORITHM_LOG( "Rotation: " << rotation);


		if ((pattern_found == 1) || (pattern_found == 2))
		{
			RubiksCube::Face left_face = RubiksCube::LeftFace;
			RubiksCube::Face right_face = RubiksCube::FrontFace;

			if (rotation == 1)
			{
				left_face = RubiksCube::FrontFace;
				right_face = RubiksCube::RightFace;
			}
			else if (rotation == 2)
			{
				left_face = RubiksCube::RightFace;
				right_face = RubiksCube::BackFace;
			}
			else if (rotation == 3)
			{
				left_face = RubiksCube::BackFace;
				right_face = RubiksCube::LeftFace;
			}

			int right_sign = ((right_face == RubiksCube::FrontFace) || (right_face == RubiksCube::RightFace)) ? 1 : -1;
			int left_sign = ((left_face == RubiksCube::FrontFace) || (left_face == RubiksCube::RightFace)) ? -1 : 1;

			RubiksCube::CubeMove left = cube.faceBaseMove(left_face);
			left.turns = -1 * left_sign;
			cube.move(left);

			RubiksCube::CubeMove bottom = cube.faceBaseMove(RubiksCube::BottomFace);
			bottom.turns = -1;
			cube.move(bottom);

			left.turns = 1 * left_sign;
			cube.move(left);

			bottom.turns = -1;
			cube.move(bottom);

			left.turns = -1 * left_sign;
			cube.move(left);

			bottom.turns = -1;
			cube.move(bottom);
			cube.move(bottom);

			left.turns = 1 * left_sign;
			cube.move(left);



			RubiksCube::CubeMove right = cube.faceBaseMove(right_face);
			right.turns = -1 * right_sign;
			cube.move(right);

			bottom.turns = 1;
			cube.move(bottom);

			right.turns = 1 * right_sign;
			cube.move(right);

			bottom.turns = 1;
			cube.move(bottom);

			right.turns = -1 * right_sign;
			cube.move(right);

			bottom.turns = 1;
			cube.move(bottom);
			cube.move(bottom);

			right.turns = 1 * right_sign;
			cube.move(right);

		}
		else if (pattern_found == 3)
		{
			RubiksCube::Face right_face = RubiksCube::RightFace;
			RubiksCube::Face left_face = RubiksCube::FrontFace;

			if (rotation == 1)
			{
				right_face = RubiksCube::BackFace;
				left_face =  RubiksCube::RightFace;
			}
			else if (rotation == 2)
			{
				right_face = RubiksCube::LeftFace;
				left_face =  RubiksCube::BackFace;
			}
			else if (rotation == 3)
			{
				right_face = RubiksCube::FrontFace;
				left_face =  RubiksCube::LeftFace;
			}

			int right_sign = ((right_face == RubiksCube::FrontFace) || (right_face == RubiksCube::RightFace)) ? 1 : -1;
			int left_sign = ((left_face == RubiksCube::FrontFace) || (left_face == RubiksCube::RightFace)) ? -1 : 1;

			RubiksCube::CubeMove right = cube.faceBaseMove(right_face);
			right.turns = -1 * right_sign;
			cube.move(right);

			RubiksCube::CubeMove bottom = cube.faceBaseMove(RubiksCube::BottomFace);
			bottom.turns = 1;
			cube.move(bottom);

			right.turns = 1 * right_sign;
			cube.move(right);

			bottom.turns = 1;
			cube.move(bottom);

			right.turns = -1 * right_sign;
			cube.move(right);

			bottom.turns = 1;
			cube.move(bottom);
			cube.move(bottom);

			right.turns = 1 * right_sign;
			cube.move(right);



			RubiksCube::CubeMove left = cube.faceBaseMove(left_face);
			left.turns = -1 * left_sign;
			cube.move(left);

			bottom.turns = -1;
			cube.move(bottom);

			left.turns = 1 * left_sign;
			cube.move(left);

			bottom.turns = -1;
			cube.move(bottom);

			left.turns = -1 * left_sign;
			cube.move(left);

			bottom.turns = -1;
			cube.move(bottom);
			cube.move(bottom);

			left.turns = 1 * left_sign;
			cube.move(left);
		}
	}

	return cube.moves();
}


void RubiksCubeFaceAligner::buildFaceFromRotation(RubiksCube::CubeFace const& input, RubiksCube::CubeFace& output, unsigned int rotation)
{
	if (rotation == 3)
	{
		output = input;
	}
	else if (rotation == 2)
	{
		//one turn
		output.color[0][0] = input.color[0][2];
		output.color[1][0] = input.color[0][1];
		output.color[2][0] = input.color[0][0];
		output.color[0][1] = input.color[1][2];
		output.color[1][1] = input.color[1][1];
		output.color[2][1] = input.color[1][0];
		output.color[0][2] = input.color[2][2];
		output.color[1][2] = input.color[2][1];
		output.color[2][2] = input.color[2][0];
	}
	else if (rotation == 1)
	{
		//two turns
		output.color[0][0] = input.color[2][2];
		output.color[1][0] = input.color[1][2];
		output.color[2][0] = input.color[0][2];
		output.color[0][1] = input.color[2][1];
		output.color[1][1] = input.color[1][1];
		output.color[2][1] = input.color[0][1];
		output.color[0][2] = input.color[2][0];
		output.color[1][2] = input.color[1][0];
		output.color[2][2] = input.color[0][0];
	}
	else if (rotation == 0)
	{

		//three turns
		output.color[0][0] = input.color[2][0];
		output.color[1][0] = input.color[2][1];
		output.color[2][0] = input.color[2][2];
		output.color[0][1] = input.color[1][0];
		output.color[1][1] = input.color[1][1];
		output.color[2][1] = input.color[1][2];
		output.color[0][2] = input.color[0][0];
		output.color[1][2] = input.color[0][1];
		output.color[2][2] = input.color[0][2];
	}
}

RubiksCubeFaceAligner::RubiksCubeFaceAligner()
{
}

bool RubiksCubeFaceAligner::makeCubeFromOrderedFaces(RubiksCube::CubeFaces const& faces, RubiksCube::CubeFaces& corrected_faces)
{
	RENGINE_ASSERT(faces.size() == 6);
	bool is_valid = false;

	corrected_faces.resize(6, RubiksCube::CubeFace());

	unsigned int rotation_stack[6];

	for (unsigned int i = 0; i != 6; ++i)
	{
		rotation_stack[i] = 3;
	}

	unsigned int counter = 0;

	do
	{

		for (unsigned int i = 0; i != 6; ++i)
		{
			buildFaceFromRotation(faces[i], corrected_faces[i], rotation_stack[i]);
		}

//		std::cout << "[" << std::setw(5) << std::setfill('0')
//				         << counter << "] " << rotation_stack[0]
//										    << rotation_stack[1]
//										    << rotation_stack[2]
//										    << rotation_stack[3]
//										    << rotation_stack[4]
//										    << rotation_stack[5]
//										    << std::endl;
		is_valid = isValidCube(corrected_faces);

		++counter;
	}
	while(!is_valid && nextRotationSetup(rotation_stack));


	return is_valid;
}

bool RubiksCubeFaceAligner::canMakeCubeFromOrderedFaces(RubiksCube::CubeFaces const& faces)
{
	RubiksCube::CubeFaces dummy;
	return makeCubeFromOrderedFaces(faces, dummy);
}

bool RubiksCubeFaceAligner::isValidCube(RubiksCube::CubeFaces const& faces)
{
	bool is_valid = true;
	RubiksCube test_cube;

	unsigned int cube_length = test_cube.getCubeLength();

	//
	// Set the faces
	//
	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length; ++x)
		{
			for (unsigned int y = 0; y != cube_length; ++y)
			{
				test_cube.cell(face, x, y) = faces[face].color[x][y];
			}
		}
	}

	//
	// Correct the cubes
	//

	// set all faces to back
	for (unsigned int i = 0; i != cube_length * cube_length * cube_length; ++i)
	{
		for (unsigned int face = 0; face != 6; ++face)
		{
			test_cube.cube(i).setColor(RubiksCube::Face(face), RubiksCube::Black);
		}
	}

	// synch the colors with the faces
	for (unsigned int face = 0; face != 6; ++face)
	{
		for (unsigned int x = 0; x != cube_length; ++x)
		{
			for (unsigned int y = 0; y != cube_length; ++y)
			{
				unsigned int index = test_cube.faceCoordinatesToIndex(face, x, y);

				RubiksCube::Face cube_face;
				unsigned int cube_index = test_cube.cubeFromCell(index, cube_face);

				test_cube.cube(cube_index).setColor(cube_face, test_cube.cell(index));
			}
		}
	}



	// synch the cubes positions and types
	RubiksCube cannonical_cube;
	for (unsigned int i = 0; i != cube_length * cube_length * cube_length; ++i)
	{
		int index = cannonical_cube.find(test_cube.cube(i));
		//std::cout << "---" << i << " -- " << index << std::endl;

		if (index < 0)
		{
			return false;

		}

		test_cube.cube(i).setCurrentPosition(i);
		test_cube.cube(i).setCannonicalPosition(cannonical_cube.cube(index).getCannonicalPosition());
		test_cube.cube(i).setCubeType(cannonical_cube.cube(index).getCubeType());
	}

	if (is_valid)
	{
		unsigned int found_count = 0;

		unsigned int linear_size = cube_length * cube_length * cube_length;
		bool* found = new bool[linear_size];

		for (unsigned int i = 0; i != linear_size; ++i)
		{
			found[i] = false;
		}

		for (unsigned int i = 0; i != linear_size; ++i)
		{
			RubiksCube::Cube const& cube = test_cube.cube(i);

			int found_index = cannonical_cube.find(cube);

			if (found_index < 0)
			{
				break;
			}

			if (found[found_index])
			{
				break;
			}

			found[found_index] = true;
		}

		for (unsigned int i = 0; i != linear_size; ++i)
		{
			if (found[i])
			{
				found_count++;
			}
		}


		delete[](found);

		//std::cout << "found_count " <<found_count << " -- " << std::endl;

		is_valid = (found_count == linear_size);
	}


	return is_valid;
}

bool RubiksCubeFaceAligner::nextRotationSetup(unsigned int* rotation_stack)
{

	if ( (rotation_stack[0] == 0) &&
		 (rotation_stack[1] == 0) &&
		 (rotation_stack[2] == 0) &&
		 (rotation_stack[3] == 0) &&
		 (rotation_stack[4] == 0) &&
		 (rotation_stack[5] == 0) )
	{
		return false;
	}

	unsigned int rotation_index = 0;


	//find current rotation index
	for (unsigned int i = 5; i >= 0; --i)
	{
		if (rotation_stack[i])
		{
			rotation_index = i;
			break;
		}
	}


	rotation_stack[rotation_index]--;

	//if (rotation_stack[rotation_index] == 0)
	{
		for (unsigned int i = (rotation_index + 1); i != 6; ++i)
		{
			rotation_stack[i] = 3;
		}
	}


	return true;
}

struct OrderSetup
{
	OrderSetup()
	{
		order[0] = 0;
		order[1] = 0;
		order[2] = 0;
		order[3] = 0;
		order[4] = 0;
		order[5] = 0;
	}

	int order[6];
};

typedef std::list<int> NumberAvailable;
typedef std::vector<OrderSetup> OrderSetups;

void computeOrder(OrderSetups& orders, NumberAvailable const& available, OrderSetup& current, int index)
{
	for (unsigned int i = 0; i != available.size(); ++i)
	{
		int current_number = -1;

		//
		// remove the current from the list to pass
		//
		NumberAvailable copy;
		unsigned int count = 0;
		for (NumberAvailable::const_iterator element = available.begin(); element != available.end(); ++element)
		{
			if (count != i)
			{
				copy.push_back(*element);
			}
			else
			{
				current_number = *element;
			}
			++count;
		}

		current.order[index] = current_number;

		if (index < 5)
		{
			computeOrder(orders, copy, current, index + 1);
		}
		else
		{
			orders.push_back(current);
		}
	}
}



bool RubiksCubeFaceAligner::canMakeCubeFromFaces(RubiksCube::CubeFaces const& faces)
{
	RubiksCube::CubeFaces dummy;
	return makeCubeFromOrderedFaces(faces, dummy);
}

bool RubiksCubeFaceAligner::makeCubeFromFaces(RubiksCube::CubeFaces const& faces, RubiksCube::CubeFaces& corrected_faces)
{
	RENGINE_ASSERT(faces.size() == 6);
	bool is_valid = false;


	NumberAvailable available;
	available.push_back(0);
	available.push_back(1);
	available.push_back(2);
	available.push_back(3);
	available.push_back(4);
	available.push_back(5);

	OrderSetups orders;
	OrderSetup order;

	computeOrder(orders, available, order, 0);
	unsigned int counter = 0;

	RubiksCube::CubeFaces ordered_faces;
	do
	{
		order = orders.back();
		orders.pop_back();


		ordered_faces.clear();
		for (unsigned int i = 0; i != 6; ++i)
		{
			ordered_faces.push_back( faces[ order.order[5 - i] ] );
		}

//		std::cout << "[" << std::setw(5) << std::setfill('0')
//				         << counter << "] " << order.order[0]
//										    << order.order[1]
//										    << order.order[2]
//										    << order.order[3]
//										    << order.order[4]
//										    << order.order[5]
//										    << std::endl;

		is_valid = makeCubeFromOrderedFaces(ordered_faces, corrected_faces);

		++counter;
	}
	while(!is_valid && !orders.empty());


	return is_valid;
}

class AlignerTester : public rengine::Thread
{
public:
	AlignerTester(OrderSetups& orders, RubiksCube::CubeFaces const& faces, rengine::Block& block)
		:m_orders(orders), m_faces(faces), m_block(block)
	{
		m_found = false;

	}

	~AlignerTester()
	{

	}

	bool found()
	{
		return m_found;
	}

	RubiksCube::CubeFaces faces()
	{
		return m_ordered_faces;
	}

	virtual void run()
	{
		if (m_orders.empty())
		{
			return;
		}


		OrderSetup order;

		do
		{
			order = m_orders.back();
			m_orders.pop_back();


			m_ordered_faces.clear();
			for (unsigned int i = 0; i != 6; ++i)
			{
				m_ordered_faces.push_back( m_faces[ order.order[5 - i] ] );
			}

			RubiksCube::CubeFaces corrected_faces;
			bool is_valid = RubiksCubeFaceAligner::makeCubeFromOrderedFaces(m_ordered_faces, corrected_faces);

			if (is_valid)
			{
				m_ordered_faces = corrected_faces;
				m_found = true;
			}

			if (is_valid || m_orders.empty())
			{
				signalShouldStop();
			}

		}
		while (keepRunning());

		m_block.release();
	}

private:

	bool m_found;
	RubiksCube::CubeFaces m_ordered_faces;
	OrderSetups& m_orders;
	RubiksCube::CubeFaces const& m_faces;
	rengine::Block& m_block;
};

bool RubiksCubeFaceAligner::makeCubeFromFacesThreaded(RubiksCube::CubeFaces const& faces, RubiksCube::CubeFaces& corrected_faces)
{
	RENGINE_ASSERT(faces.size() == 6);

	// fast path for orderes faces
	if (makeCubeFromOrderedFaces(faces, corrected_faces))
	{
		return true;
	}

	NumberAvailable available;
	available.push_back(0);
	available.push_back(1);
	available.push_back(2);
	available.push_back(3);
	available.push_back(4);
	available.push_back(5);

	OrderSetups orders;
	OrderSetup order;
	computeOrder(orders, available, order, 0);


	//divide computed orders in jobs
	int number_of_processors = rengine::Thread::numberOfProcessors();

	if (number_of_processors <= 0)
	{
		number_of_processors = 1;
	}

	typedef std::vector<OrderSetups> DividedOrders;
	DividedOrders divided_orders;


	int work_per_thread = int(orders.size() / number_of_processors);
	for (int i = 0; i != number_of_processors; ++i)
	{
		int start = i * work_per_thread;
		int end = start + work_per_thread;

		if (end > int(orders.size()))
		{
			end = int(orders.size());
		}

		if ( (end - start) > 0)
		{
			OrderSetups job;

			while (start != end)
			{
				job.push_back( orders[start] );
				++start;
			}

			divided_orders.push_back(job);
		}
	}

	typedef rengine::SharedPointer< AlignerTester > Worker;
	typedef std::vector< Worker > Workers;


	bool is_valid = false;

	{
		rengine::Block block;
		Workers workers;


		for (DividedOrders::size_type i = 0; i != divided_orders.size(); ++i)
		{
			workers.push_back( new AlignerTester(divided_orders[i], faces, block) );
			//workers.back()->setProcessorAfinity( rengine::Int(i) );
			workers.back()->start();
		}

		block.block();

		for (DividedOrders::size_type i = 0; i != divided_orders.size(); ++i)
		{
			workers[i]->stop();

			if (workers[i]->found())
			{
				corrected_faces = workers[i]->faces();
				is_valid = true;
			}
		}
	}



	return is_valid;
}
