#ifndef __ALGORITHM_H__
#define __ALGORITHM_H__

#include "RubiksCube.h"

class RubiksCubeAlgorithm
{
public :
	RubiksCubeAlgorithm();
	virtual ~RubiksCubeAlgorithm();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);
};

class RubiksCubeShuffle : public RubiksCubeAlgorithm
{
public:
	RubiksCubeShuffle(unsigned int total_moves);
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);
private:
	unsigned int total_moves_;
};

class RubiksCubeCross : public RubiksCubeAlgorithm
{
public:
	RubiksCubeCross();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);

	static bool solved(RubiksCube const& rubics_cube);
	static bool solvedFace(RubiksCube::Face const& face, RubiksCube const& rubics_cube);
};

class RubiksCubeCorners : public RubiksCubeAlgorithm
{
public:
	RubiksCubeCorners();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);

	static bool solved(RubiksCube const& rubics_cube);
	static bool solvedCorner(unsigned int corner, RubiksCube const& rubics_cube);
private:
	void rotationFaceForCorner(RubiksCube const& rubics_cube, unsigned int const& corner, RubiksCube::Face& face, int& sign) const;
	bool correctPositionForFlip(RubiksCube const& rubics_cube, unsigned int const& corner, unsigned int const& position);
};

class RubiksCubeMiddleLayer : public RubiksCubeAlgorithm
{
public:
	RubiksCubeMiddleLayer();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);

	static bool solved(RubiksCube const& rubics_cube);
private:
	// 0 -- moves_target_to_left
	// 1 -- moves_target_to_right
	unsigned int methodToUse(RubiksCube const& rubics_cube, unsigned int const& corner, RubiksCube::Face const& align_face);

	void facesToUse(RubiksCube const& rubics_cube, unsigned int const& corner, RubiksCube::Face& left_face, RubiksCube::Face& right_face);

	void applyMethod(RubiksCube& rubics_cube, RubiksCube::Face const& left_face, RubiksCube::Face const& right_face, unsigned int method);
};

class RubiksCubeBottomCross : public RubiksCubeAlgorithm
{
public:
	RubiksCubeBottomCross();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);

	static bool solved(RubiksCube const& rubics_cube);
private:
	bool patternFound(RubiksCube const& rubics_cube, unsigned int pattern);
	void facesToUse(RubiksCube const& rubics_cube, unsigned int const& pattern, RubiksCube::Face& left_face, RubiksCube::Face& right_face);
};

class RubiksCubeBottomCorners : public RubiksCubeAlgorithm
{
public:
	RubiksCubeBottomCorners();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);

	static bool solved(RubiksCube const& rubics_cube);
private:
	bool patternFound(RubiksCube const& rubics_cube, unsigned int pattern);
};


class RubiksCubeBottomCubeCorners : public RubiksCubeAlgorithm
{
public:
	RubiksCubeBottomCubeCorners();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);

	static bool solved(RubiksCube const& rubics_cube);
private:
	bool patternFound(RubiksCube const& rubics_cube, unsigned int pattern);
};

class RubiksCubeBottomEdges : public RubiksCubeAlgorithm
{
public:
	RubiksCubeBottomEdges();
	virtual RubiksCube::CubeMoves operator()(RubiksCube& rubics_cube);

	static bool solved(RubiksCube const& rubics_cube);
private:
	bool patternFound(RubiksCube const& rubics_cube, unsigned int pattern);
	unsigned int getRotation(RubiksCube const& rubics_cube);
};

//
// Some basic optimizations
// -> M * M' = Nothing
// -> M * M * M = M'
//

class RubiksCubeOptimizer
{
public:
	RubiksCubeOptimizer();
	RubiksCube::CubeMoves operator()(RubiksCube::CubeMoves& moves);
private:
	// -> M * M' = Nothing
	RubiksCube::CubeMoves inverseOptimization(RubiksCube::CubeMoves const& moves);

	// -> M * M * M = M'
	RubiksCube::CubeMoves fastSameMove(RubiksCube::CubeMoves const& moves);
};


//
// Takes an unsorted list of faces
// Checks is those faces make a cube, and orders them
//
class RubiksCubeFaceAligner
{
public:
	RubiksCubeFaceAligner();

	// builds a cube face from input accumulating the specified rotation
	// rotation = 3 (0 turns), rotation = 2 (1 turns), rotation = 1 (2 turns), rotation = 0 (3 turns)
	static void buildFaceFromRotation(RubiksCube::CubeFace const& input, RubiksCube::CubeFace& output, unsigned int rotation);

	// Checks if the supplied faces are ordered and correctly rotated to make a cube
	static bool isValidCube(RubiksCube::CubeFaces const& faces);

	// Checks if the supplied faces are ordered and there is a face rotation combination that makes a cube
	static bool canMakeCubeFromOrderedFaces(RubiksCube::CubeFaces const& faces);

	// computer the correctly rotated faces
	// if the supplied faces are ordered and there is a face rotation combination that makes a cube
	static bool makeCubeFromOrderedFaces(RubiksCube::CubeFaces const& faces, RubiksCube::CubeFaces& corrected_faces);


	// Checks if there is an order and face rotation combination that makes a cube
	static bool canMakeCubeFromFaces(RubiksCube::CubeFaces const& faces);

	// computer the correctly rotated faces if there is an order and face rotation combination that makes a cube
	static bool makeCubeFromFaces(RubiksCube::CubeFaces const& faces, RubiksCube::CubeFaces& corrected_faces);


	// computer the correctly rotated faces if there is an order and face rotation combination that makes a cube
	bool makeCubeFromFacesThreaded(RubiksCube::CubeFaces const& faces, RubiksCube::CubeFaces& corrected_faces);

private:
	static bool nextRotationSetup(unsigned int* rotation_stack);
};


//
// Implementation
//

inline RubiksCubeAlgorithm::RubiksCubeAlgorithm()
{
}

inline RubiksCubeAlgorithm::~RubiksCubeAlgorithm()
{
}

inline RubiksCube::CubeMoves RubiksCubeAlgorithm::operator()(RubiksCube& rubics_cube)
{
	return RubiksCube::CubeMoves();
}

inline RubiksCubeShuffle::RubiksCubeShuffle(unsigned int total_moves)
:total_moves_(total_moves)
{
}

inline RubiksCubeCross::RubiksCubeCross()
{
}

inline RubiksCubeCorners::RubiksCubeCorners()
{
}

inline RubiksCubeOptimizer::RubiksCubeOptimizer()
{
}

inline RubiksCubeMiddleLayer::RubiksCubeMiddleLayer()
{
}

inline RubiksCubeBottomCross::RubiksCubeBottomCross()
{
}

inline RubiksCubeBottomCorners::RubiksCubeBottomCorners()
{
}

inline RubiksCubeBottomCubeCorners::RubiksCubeBottomCubeCorners()
{
}

inline RubiksCubeBottomEdges::RubiksCubeBottomEdges()
{
}
#endif // __ALGORITHM_H__
