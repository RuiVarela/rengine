// __!!rengine_copyright!!__ //

#ifndef __RENGINE_ORBITCAMERA_H__
#define __RENGINE_ORBITCAMERA_H__

#include <rengine/system/SystemVariable.h>
#include <rengine/camera/Camera.h>
#include <rengine/windowing/Windowing.h>
#include <rengine/math/Vector.h>

namespace rengine
{
	class OrbitCamera : public Camera
	{
	public:

		OrbitCamera();
		virtual ~OrbitCamera();

		virtual void connect();
		virtual void disconnect();

		virtual void update();



		Vector3D const& origin() const;
		void setOrigin(Vector3D const& origin_position);

		Real const& radius() const;
		void setRadius(Real const& spherical_radius);

		Real const& elevation() const;
		void setElevation(Real const& spherical_elevation);

		Real const& azimuth() const;
		void setAzimuth(Real const& sperical_azimuth);

		Real const& maxElevation() const;
		void setMaxElevation(Real const& max_elevation);

		Real const& maxRadius() const;
		void setMaxRadius(Real const& max_radius);

		Real const& minRadius() const;
		void setMinRadius(Real const& min_radius);

		Real const& angleIncreaseFactor() const;
		void setAngleIncreaseFactor(Real const& factor);

		Real const& angleDecayFactor() const;
		void setAngleDecayFactor(Real const& factor);

		Real const& radiusIncreaseFactor() const;
		void setRadiusIncreaseFactor(Real const& factor);

		Real const& radiusDecayFactor() const;
		void setRadiusDecayFactor(Real const& factor);
	private:
		Vector3D origin_;
		SharedPointer<SystemVariable> radius_;
		SharedPointer<SystemVariable> elevation_;
		SharedPointer<SystemVariable> azimuth_;

		SharedPointer<SystemVariable> radius_change_;
		SharedPointer<SystemVariable> elevation_change_;
		SharedPointer<SystemVariable> azimuth_change_;

		SharedPointer<SystemVariable> max_elevation_;
		SharedPointer<SystemVariable> min_radius_;
		SharedPointer<SystemVariable> max_radius_;

		SharedPointer<SystemVariable> angle_increase_factor_;
		SharedPointer<SystemVariable> angle_decay_factor_;

		SharedPointer<SystemVariable> radius_increase_factor_;
		SharedPointer<SystemVariable> radius_decay_factor_;

		Real elevation_bucket_;
		Real azimuth_bucket_;
		Real radius_bucket_;

		Bool matrix_needs_update_;

	};

	//
	// implementation
	//

	RENGINE_INLINE Real const& OrbitCamera::radius() const
	{
		return radius_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setRadius(Real const& spherical_radius)
	{
		radius_->set(spherical_radius);
	}

	RENGINE_INLINE Real const& OrbitCamera::elevation() const
	{
		return elevation_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setElevation(Real const& spherical_elevation)
	{
		elevation_->set(spherical_elevation);
	}

	RENGINE_INLINE Real const& OrbitCamera::azimuth() const
	{
		return azimuth_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setAzimuth(Real const& sperical_azimuth)
	{
		azimuth_->set(sperical_azimuth);
	}

	RENGINE_INLINE Real const& OrbitCamera::maxElevation() const
	{
		return max_elevation_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setMaxElevation(Real const& max_elevation)
	{
		max_elevation_->set(max_elevation);
	}

	RENGINE_INLINE Real const& OrbitCamera::angleIncreaseFactor() const
	{
		return angle_increase_factor_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setAngleIncreaseFactor(Real const& factor)
	{
		angle_increase_factor_->set(factor);
	}

	RENGINE_INLINE Real const& OrbitCamera::angleDecayFactor() const
	{
		return angle_decay_factor_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setAngleDecayFactor(Real const& factor)
	{
		angle_decay_factor_->set(factor);
	}

	RENGINE_INLINE Real const& OrbitCamera::radiusIncreaseFactor() const
	{
		return radius_increase_factor_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setRadiusIncreaseFactor(Real const& factor)
	{
		radius_increase_factor_->set(factor);
	}

	RENGINE_INLINE Real const& OrbitCamera::radiusDecayFactor() const
	{
		return radius_decay_factor_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setRadiusDecayFactor(Real const& factor)
	{
		radius_decay_factor_->set(factor);
	}

	RENGINE_INLINE Real const& OrbitCamera::maxRadius() const
	{
		return max_radius_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setMaxRadius(Real const& max_radius)
	{
		max_radius_->set(max_radius);
	}

	RENGINE_INLINE Real const& OrbitCamera::minRadius() const
	{
		return min_radius_->asFloat();
	}

	RENGINE_INLINE void OrbitCamera::setMinRadius(Real const& min_radius)
	{
		min_radius_->set(min_radius);
	}

} // end of namespace

#endif // __RENGINE_ORBITCAMERA_H__
