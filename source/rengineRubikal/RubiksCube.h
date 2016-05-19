#ifndef __RUBIKSCUBE_H__
#define __RUBIKSCUBE_H__

#include <iosfwd>
#include <vector>

class RubiksCube
{
public:
	typedef unsigned int CubeCode; //32 bits int
	static const unsigned int NoneIndex = 0xFFFFFFFF;

	//
	// Face color [3 bits]
	//
	enum FaceColor
	{
		Red	= 0,
		Yellow	= 1,
		Orange	= 2,
		White	= 3,
		Blue	= 4,
		Green	= 5,
		Black	= 6
	};

	//
	// Face orientation [3 bits]
	//
	enum Face
	{
		FrontFace	= 0,
		RightFace	= 1,
		BackFace	= 2,
		LeftFace	= 3,
		TopFace		= 4,
		BottomFace	= 5,
		NoneFace	= 6
	};

	enum CubeType
	{
		All				= 0,
		LineCube 		= 1,
		FaceMasterCube	= 2,
		CornerCube		= 4,
		CentralLineCube = 16
	};

	enum Movement
	{
		Front		= 0,
		Right		= 1,
		Up			= 2
	};

	struct Cube
	{
		CubeCode static const color_size = 3;
		CubeCode static const position_size = 7;
		//total size = 6 (faces) * 3 (bits) + 2 (position) * 7 (bits) = 32 bits

		Cube();

		CubeCode code;
		CubeCode descriptor;

		void setColor(Face const& face, FaceColor const& color);
		FaceColor getColor(Face const& face) const;

		bool hasColor(FaceColor const& color) const;

		void setCannonicalPosition(CubeCode const& position);
		CubeCode getCannonicalPosition() const;

		void setCurrentPosition(CubeCode const& position);
		CubeCode getCurrentPosition() const;

		void setCubeType(CubeType const type);
		CubeType getCubeType() const;
	};


	struct CubeMove
	{
		Movement movement;
		unsigned int depth;
		int turns;

		bool operator==(CubeMove const& rhs);
	};

	typedef std::vector<CubeMove> CubeMoves;

	typedef std::vector<unsigned int> Positions;
	typedef std::vector<Face> Faces;


	struct CubeFace
	{
		FaceColor color[3][3];
	};
	typedef std::vector<CubeFace> CubeFaces;



	RubiksCube();
	RubiksCube(RubiksCube const& copy);

	RubiksCube& operator = (RubiksCube const& copy);

	~RubiksCube();

	/**
	 * Gets the cube length, a regular rubik cube has 3 units of length
	 *
	 * Currently cubes only support 7bits position. Max length is 5
	 */
	unsigned int getCubeLength() const;

	unsigned int getCubeLengthMinusOne() const;

	unsigned int getCenterIndex() const;

	/*
	 *  		Front
	 * 	[ [0, 2][1, 2][2, 2] ]
	 *  [ [0, 1][1, 1][2, 1] ]
	 *  [ [0, 0][1, 0][2, 0] ]
	 *
	 *  [ [x, y][x, y][x, y] ]
	 */

	/**
	 * Inspectors and modifiers
	 * Color for the given cell. the cell is indexed by face, x coordinate and y coordinate
	 * x, y coordinates are viewed as in front of desired face.
	 *
	 * face limit is [0, 6[
	 *    x limit is [0, cube_lenght_[
	 *    y limit is [0, cube_lenght_[
	 *
	 * To view the right-face, rotate a real rubic cube from right to left one time.
	 * To view the back-face, rotate a real rubic cube from right to left two times.
	 * To view the left-face, rotate a real rubic cube from right to left three times.
	 * To view the top-face, rotate a real rubic cube from top to bottom one time.
	 * To view the bottom-face, rotate a real rubic cube from bottom to top one time.
	 */
	FaceColor const& cell(unsigned int const face, unsigned int const x, unsigned int const y) const;
	FaceColor& cell(unsigned int const face, unsigned int const x, unsigned int const y);

	/**
	 * Same as above but using linear indexing
	 * index = (face * cube_length * cube_length + y * cube_length + x);
	 * index = faceCoordinatesToIndex(face, x, y);
	 */
	FaceColor const& cell(unsigned int const index) const;
	FaceColor& cell(unsigned int const index);

	unsigned int faceCoordinatesToIndex(unsigned int const face, unsigned int const x, unsigned int const y) const;
	void indexToFaceCoordinates(unsigned int index, unsigned int& face, unsigned int& x, unsigned int& y) const;


	/**
	 * Inspectors and modifiers for the inner cubes representation
	 *
	 * x, y, z coordinates are always viewed as in front of the the canonical global cube representation
	 * - On the solved canonical representation White is right, Yellow is left, Blue is top
	 *
	 * x limit is [0, cube_lenght_[
	 * y limit is [0, cube_lenght_[
	 * z limit is [0, cube_lenght_[
	 */
	Cube const& cube(unsigned int const x, unsigned int const y, unsigned int const z) const;
	Cube& cube(unsigned int const x, unsigned int const y, unsigned int const z);

	/**
	 * Same as above but using linear indexing
	 * index =  (z * cube_length_ * cube_length_ + y * cube_length_ + x);
	 * index = cubeCoordinatesToIndex(x, y, z);
	 */
	Cube const& cube(unsigned int const index) const;
	Cube& cube(unsigned int const index);

	unsigned int cubeCoordinatesToIndex(unsigned int const x, unsigned int const y, unsigned int const z) const;
	void indexToCubeCoordinates(unsigned int index, unsigned int& x, unsigned int& y, unsigned int& z) const;


	/**
	 * Calculates the corresponding cell index from a inner cube's face
	 */
	unsigned int cellFromCube(unsigned int const cube_index, Face const& cube_face) const;

	/**
	 * Calculates the corresponding cube index from cell's face index
	 * cube_face is set with the corresponding cube's face
	 */
	unsigned int cubeFromCell(unsigned int const face_index, Face& cube_face) const;


	/**
	 * Gets the current position of the cube with cannonical_position
	 */
	unsigned int cubeFromCannonicalPosition(unsigned int const cannonical_position) const;


	/*
	 * Performs a basic movement on the cube
	 *
	 * movement: type of movement executed
	 * depth: the selected ring relative to the type of movement [0, getCubeLength() - 1]
	 * turns: the total turns to perform. a negative value inverts the rotation
	 */
	virtual void move(Movement const& movement, unsigned int const depth, int turns = 1);
	void move(CubeMove const& cube_move);

	//
	// Reverts the cube to the previous state
	//
	//TODO!! this
	void unmove();

	// TODO::Checkpoints

	//should clear checkpoints and moves
	void clearHistory();



	/**
	 * Resets the rubik cube state
	 */
	void setSolvedSetup();

	/**
	 * Sets the cube in the faces setup,
	 * faces must have size == 6
	 */
	void setSetup(CubeFaces const& faces);



	CubeMoves const& moves() const;
	CubeMoves& moves();



	/**
	 * Cube Queries
	 *
	 * finds the cubes who have the queried colors
	 */
	Positions find(FaceColor const& first, FaceColor const& second, FaceColor const& third, CubeType const& type) const;

	/**
	 * Finds the exact match
	 *
	 * -1 = not found
	 */
	int find(Cube const& cube) const;


	/**
	 * Gets the faces where cube is
	 */
	Faces getFaces(unsigned int cube_index) const;


	/**
	 * Converts the cube index to 3D Cube positions
	 * x [-1 , 1]
	 * y [-1 , 1]
	 * z [-1 , 1]
	 */
	void indexToCenteredPosition(unsigned int index, float& x, float& y, float& z) const;

	/**
	 * Calculates the distance between 2 cubes
	 */
	void cubesVector(unsigned int const left, unsigned int const right, float& x, float& y, float& z) const;

	/**
	 * Calculates the vector between 2 cubes
	 */
	float cubesDistance(unsigned int const left, unsigned int const right) const;

	bool isCubeOnFace(Face const face, unsigned int index) const;

	static bool isInverse(CubeMove const& lhs, CubeMove const& rhs);

	/**
	 * Gets the base move for the specified face
	 */
	CubeMove faceBaseMove(Face const& face);

	/**
	 * Gets the opposite face
	 */
	static Face oppositeFace(Face const& face);


	/**
	 * Computes the corresponding face normal
	 */
	static void faceNormal(Face const face, float& x, float& y, float& z);
	/***
	 * CubeCode
	 */

	void serializeTo(std::ostream &out);

	// checks is the cube is valid
	bool isValid() const;
private:
	void initializeCube(unsigned int const length);
	void deinitialize();

	void rotateCube(Cube& cube, Movement const& movement, bool inverted = false);

	void normalizeTurns(int& turns, bool& inverted);
	void rotateFace(Face const face, bool const inverted = false);
	void moveFront(unsigned int const depth, bool const inverted = false);
	void moveRight(unsigned int const depth, bool const inverted = false);
	void moveUp(unsigned int const depth, bool const inverted = false);
	void rotateFrontRing(unsigned int const depth, bool const inverted = false);
	void rotateRightRing(unsigned int const depth, bool const inverted = false);
	void rotateUpRing(unsigned int const depth, bool const inverted = false);

	void performMove(Movement const& movement, unsigned int const depth, int turns = 1);

	FaceColor& bufferCell(unsigned int const x, unsigned int const y);
	void copyBufferToFace(unsigned int const face);
	void copyFaceToBuffer(unsigned int const face);

	bool isValidContract() const;

	void copyObject(RubiksCube const& copy);

	unsigned int cube_length_;

	FaceColor* cells_;
	FaceColor* buffer_;

	Cube* cubes_;

	CubeMoves cube_moves_;
};

std::ostream& operator << (std::ostream& out, RubiksCube::FaceColor const& face);

//
// Implementation
//

inline bool RubiksCube::CubeMove::operator==(CubeMove const& rhs)
{
	return ( (movement == rhs.movement) && (depth == rhs.depth) && (turns == rhs.turns) );
}

inline void RubiksCube::Cube::setColor(Face const& face, FaceColor const& color)
{
	CubeCode const allowed_values_bit_mask = ~(~0 << color_size);

	//clear the old value
	code &= ~(allowed_values_bit_mask << (face * color_size));

	//set the new value
	code |= (color & allowed_values_bit_mask) << (face * color_size);
}

inline RubiksCube::FaceColor RubiksCube::Cube::getColor(Face const& face) const
{
	CubeCode const allowed_values_bit_mask = ~(~0 << color_size);

	return FaceColor( (code >> (face * color_size)) & allowed_values_bit_mask );
}

inline bool RubiksCube::Cube::hasColor(FaceColor const& color) const
{

	return ( (getColor(FrontFace) == color) ||
			 (getColor(RightFace) == color) ||
			 (getColor(BackFace) == color) ||
			 (getColor(LeftFace) == color) ||
			 (getColor(TopFace) == color) ||
			 (getColor(BottomFace) == color)
			);
}

inline void RubiksCube::Cube::setCannonicalPosition(CubeCode const& position)
{
	CubeCode const allowed_values_bit_mask = ~(~0 << position_size);

	//clear the old value
	code &= ~(allowed_values_bit_mask << (6 * color_size));

	//set the new value
	code |= (position & allowed_values_bit_mask) << (6 * color_size);

}

inline RubiksCube::CubeCode RubiksCube::Cube::getCannonicalPosition() const
{
	CubeCode const allowed_values_bit_mask = ~(~0 << position_size);

	return CubeCode( (code >> (6 * color_size)) & allowed_values_bit_mask );
}

inline void RubiksCube::Cube::setCurrentPosition(CubeCode const& position)
{
	CubeCode const allowed_values_bit_mask = ~(~0 << position_size);

	//clear the old value
	code &= ~(allowed_values_bit_mask << (6 * color_size + position_size));

	//set the new value
	code |= (position & allowed_values_bit_mask) << (6 * color_size + position_size);
}

inline RubiksCube::CubeCode RubiksCube::Cube::getCurrentPosition() const
{
	CubeCode const allowed_values_bit_mask = ~(~0 << position_size);

	return CubeCode( (code >> (6 * color_size + position_size)) & allowed_values_bit_mask );
}

inline void RubiksCube::Cube::setCubeType(CubeType const type)
{
	descriptor = type;
}

inline RubiksCube::CubeType RubiksCube::Cube::getCubeType() const
{
	return CubeType(descriptor);
}

inline unsigned int RubiksCube::faceCoordinatesToIndex(unsigned int const face, unsigned int const x, unsigned int const y) const
{
	return (face * cube_length_ * cube_length_ + y * cube_length_ + x);
}

inline void RubiksCube::indexToFaceCoordinates(unsigned int index, unsigned int& face, unsigned int& x, unsigned int& y) const
{
	face = (unsigned int)(index / (cube_length_ * cube_length_));
	index = index % (cube_length_ * cube_length_); //index now holds reminder

	y = (unsigned int)(index / cube_length_);
	x = index % cube_length_;
}

inline unsigned int RubiksCube::cubeCoordinatesToIndex(unsigned int const x, unsigned int const y, unsigned int const z) const
{
	return (z * cube_length_ * cube_length_ + y * cube_length_ + x);
}

inline void RubiksCube::indexToCubeCoordinates(unsigned int index, unsigned int& x, unsigned int& y, unsigned int&z) const
{
	z = (unsigned int)(index / (cube_length_ * cube_length_));
	index = index % (cube_length_ * cube_length_); //index now holds reminder

	y = (unsigned int)(index / cube_length_);
	x = index % cube_length_;
}

inline unsigned int RubiksCube::getCubeLength() const
{
	return cube_length_;
}

inline unsigned int RubiksCube::getCubeLengthMinusOne() const
{
	return getCubeLength() - 1;
}

inline unsigned int RubiksCube::getCenterIndex() const
{
	return getCubeLength() / 2;
}

inline RubiksCube::CubeMoves const& RubiksCube::moves() const
{
	return cube_moves_;
}

inline RubiksCube::CubeMoves& RubiksCube::moves()
{
	return cube_moves_;
}

inline void RubiksCube::move(CubeMove const& cube_move)
{
	move(cube_move.movement, cube_move.depth, cube_move.turns);
}

inline void RubiksCube::clearHistory()
{
	cube_moves_.clear();
}

inline RubiksCube::Face RubiksCube::oppositeFace(Face const& face)
{
	Face opposite;

	switch (face)
	{
		case FrontFace	: opposite = BackFace;	break;
		case BackFace	: opposite = FrontFace;	break;
		case RightFace	: opposite = LeftFace;	break;
		case LeftFace	: opposite = RightFace;	break;
		case TopFace	: opposite = BottomFace;	break;
		case BottomFace	: opposite = TopFace;	break;

		default: break;
	}

	return opposite;
}

inline void RubiksCube::faceNormal(Face const face, float& x, float& y, float& z)
{
	switch (face)
	{
		case FrontFace:
		{
			x = 0.0f;
			y = 0.0f;
			z = -1.0f;
		}
			break;

		case BackFace:
		{
			x = 0.0f;
			y = 0.0f;
			z = 1.0f;
		}
			break;

		case RightFace:
		{
			x = 1.0f;
			y = 0.0f;
			z = 0.0f;
		}
			break;

		case LeftFace:
		{
			x = -1.0f;
			y = 0.0f;
			z = 0.0f;
		}
			break;

		case TopFace:
		{
			x = 0.0f;
			y = 1.0f;
			z = 0.0f;
		}
			break;

		case BottomFace:
		{
			x = 0.0f;
			y = -1.0f;
			z = 0.0f;
		}
			break;

		default:
		{
			x = 0.0f;
			y = 0.0f;
			z = 0.0f;
		}
			break;

	}
}

inline void RubiksCube::indexToCenteredPosition(unsigned int index, float& x, float& y, float& z) const
{
	unsigned int int_x, int_y, int_z;
	unsigned int const center_index = (unsigned int)(getCubeLength() / 2);
	float center_position = float(center_index);

	indexToCubeCoordinates(index, int_x, int_y, int_z);

	x = float(int_x) - center_position;
	y = float(int_y) - center_position;
	z = float(int_z) - center_position;
}

inline RubiksCube::Faces RubiksCube::getFaces(unsigned int cube_index) const
{
	RubiksCube::Faces faces;

	Cube const& target_cube = cube(cube_index);

	if (target_cube.getColor(FrontFace) != Black)
	{
		faces.push_back(FrontFace);
	}

	if (target_cube.getColor(RightFace) != Black)
	{
		faces.push_back(RightFace);
	}

	if (target_cube.getColor(BackFace) != Black)
	{
		faces.push_back(BackFace);
	}

	if (target_cube.getColor(LeftFace) != Black)
	{
		faces.push_back(LeftFace);
	}

	if (target_cube.getColor(TopFace) != Black)
	{
		faces.push_back(TopFace);
	}

	if (target_cube.getColor(BottomFace) != Black)
	{
		faces.push_back(BottomFace);
	}

	return faces;
}

inline bool RubiksCube::isInverse(CubeMove const& lhs, CubeMove const& rhs)
{
	return ( (lhs.movement == rhs.movement) &&
			 (lhs.depth == rhs.depth) &&
			 (lhs.turns == -rhs.turns) );
}

#endif //__RUBIKSCUBE_H__
