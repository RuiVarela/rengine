// __!!rengine_copyright!!__ //

#ifndef __RENGINE_CAMERA_H__
#define __RENGINE_CAMERA_H__

#include <rengine/math/Matrix.h>

namespace rengine
{
	class Camera
	{
	public:
		Camera() {};
		virtual ~Camera() {};

		virtual void connect() {}
		virtual void disconnect() {}

		virtual Matrix const& viewMatrix() const;
		virtual void setViewMatrix(Matrix const& matrix);

		virtual Matrix const& projectionMatrix() const;
		virtual void setProjectionMatrix(Matrix const& matrix);

	    void setProjectionAsOrtho(Real const left, Real const right, Real const bottom, Real const top, Real const z_near, Real const z_far);
	    void setProjectionAsOrtho2D(Real const left, Real const right, Real const bottom, Real const top);
	    void setProjectionAsFrustum(Real const left, Real const right, Real const bottom, Real const top, Real const z_near, Real const z_far);
	    void setProjectionAsPerspective(Real const fovy, Real const aspect_ratio, Real const z_near, Real const z_far);

		virtual void update() {};

	protected:
		Matrix view_matrix;
		Matrix projection_matrix_;
	};

	//
	// Implementation
	//

	RENGINE_INLINE Matrix const& Camera::viewMatrix() const
	{
		return view_matrix;
	}

	RENGINE_INLINE void Camera::setViewMatrix(Matrix const& matrix)
	{
		view_matrix = matrix;
	}

	RENGINE_INLINE Matrix const& Camera::projectionMatrix() const
	{
		return projection_matrix_;
	}

	RENGINE_INLINE void Camera::setProjectionMatrix(Matrix const& matrix)
	{
		projection_matrix_ = matrix;
	}

	RENGINE_INLINE void Camera::setProjectionAsOrtho(Real const left, Real const right, Real const bottom, Real const top, Real const z_near, Real const z_far)
    {
    	setProjectionMatrix( Matrix::ortho(left, right, bottom, top, z_near, z_far) );
    }

	RENGINE_INLINE void Camera::setProjectionAsOrtho2D(Real const left, Real const right, Real const bottom, Real const top)
    {
    	setProjectionMatrix( Matrix::ortho2D(left, right, bottom, top) );
    }

	RENGINE_INLINE void Camera::setProjectionAsFrustum(Real const left, Real const right, Real const bottom, Real const top, Real const z_near, Real const z_far)
    {
    	setProjectionMatrix( Matrix::frustum(left, right, bottom, top, z_near, z_far) );
    }

	RENGINE_INLINE void Camera::setProjectionAsPerspective(Real const fovy, Real const aspect_ratio, Real const z_near, Real const z_far)
    {
    	setProjectionMatrix( Matrix::perspective(fovy, aspect_ratio, z_near, z_far) );
    }

} // end of namespace

#endif // __RENGINE__CAMERA_H__
