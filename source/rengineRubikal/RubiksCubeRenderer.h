#ifndef RUBIKSCUBERENDERER_H_
#define RUBIKSCUBERENDERER_H_

#include "RubiksCube.h"

#include <rengine/geometry/Mesh.h>
#include <rengine/math/Quaternion.h>
#include <rengine/state/DrawStates.h>
#include <rengine/system/SystemVariable.h>

class CubeShape : public rengine::Mesh
{
public:
	CubeShape();

	rengine::Vector3D const& getPosition();
	void setPosition(rengine::Vector3D const& position);

	void setWidth(float const width);
	float const getWidth() const;

	void setCannonicalPosition(unsigned int cannonical_position);
	unsigned int getCannonicalPosition() const;

	void computeGeometry(RubiksCube::Cube const& target_cube);

	rengine::Quaternion& orientation();
	rengine::Quaternion const& orientation() const;

	rengine::Quaternion& movingOrientation();
	rengine::Quaternion const& movingOrientation() const;
private:
	rengine::Vector3D position_;
	rengine::Quaternion orientation_;
	rengine::Quaternion moving_orientation_;
	float width_;
	unsigned int cannonical_position_;
};

class RubiksCubeRenderer : public RubiksCube, public rengine::Drawable
{
public:
	enum State
	{
		Idle		= 0,
		Moving		= 1
	};

	RubiksCubeRenderer();
	virtual ~RubiksCubeRenderer();

	void update(float const time);
	virtual void prepareDrawing(rengine::RenderEngine& render_engine);
	virtual void unprepareDrawing(rengine::RenderEngine& render_engine);
	virtual void draw(rengine::RenderEngine& render_engine);

	CubeShape const& cubeShape(unsigned int const x, unsigned int const y, unsigned int const z) const;
	CubeShape& cubeShape(unsigned int const x, unsigned int const y, unsigned int const z);

	CubeShape const& cubeShape(unsigned int const index) const;
	CubeShape& cubeShape(unsigned int const index);

	void startMove(Movement const& movement, unsigned int const depth, int turns = 1);

	float getSpacing() const;
	float getCubeSize() const;
	float globalWidth() const;

	void setSpeed(float const speed);
	float speed() const;

	State state() const;

	void synchGeometry();
private:
	void initialize();
	rengine::SharedPointer<CubeShape> *cubes_;
	rengine::SharedPointer<rengine::SystemVariable> speed_;

	float cube_size_;
	float spacing_;
	State state_;

	rengine::SharedPointer<rengine::DrawStates> draw_states_;

	float last_update_;
	float move_start_time_;

	CubeMove current_move_;
};

//
// Implementation
//

inline rengine::Quaternion& CubeShape::orientation()
{
	return orientation_;
}

inline rengine::Quaternion const& CubeShape::orientation() const
{
	return orientation_;
}

inline rengine::Quaternion& CubeShape::movingOrientation()
{
	return moving_orientation_;
}

inline rengine::Quaternion const& CubeShape::movingOrientation() const
{
	return moving_orientation_;
}

inline rengine::Vector3D const& CubeShape::getPosition()
{
	return position_;
}

inline void CubeShape::setPosition(rengine::Vector3D const& position)
{
	position_ = position;
}

inline void CubeShape::setWidth(float const width)
{
	width_ = width;
}

inline float const CubeShape::getWidth() const
{
	return width_;
}

inline void CubeShape::setCannonicalPosition(unsigned int cannonical_position)
{
	cannonical_position_ = cannonical_position;
}

inline unsigned int CubeShape::getCannonicalPosition() const
{
	return cannonical_position_;
}

// Rendere
inline float RubiksCubeRenderer::getSpacing() const
{
	return spacing_;
}

inline float RubiksCubeRenderer::getCubeSize() const
{
	return cube_size_;
}

inline float RubiksCubeRenderer::globalWidth() const
{
	return float(getCubeLength()) * getCubeSize() +
		   float(getCubeLength() - 1) * getSpacing();
}

inline void RubiksCubeRenderer::setSpeed(float const speed)
{
	speed_->set(speed);
}

inline float RubiksCubeRenderer::speed() const
{
	return speed_->asFloat();
}

inline RubiksCubeRenderer::State RubiksCubeRenderer::state() const
{
	return state_;
}

#endif // RUBIKSCUBERENDERER_H_
