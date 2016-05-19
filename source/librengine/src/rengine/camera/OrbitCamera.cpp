// __!!rengine_copyright!!__ //

#include <rengine/camera/OrbitCamera.h>
#include <rengine/CoreEngine.h>
#include <rengine/system/System.h>

#include <string>
#include <iostream>

namespace rengine
{
	OrbitCamera::OrbitCamera()
	{
		radius_ = new SystemVariable("orbit_camera_radius", 2.0f);
		radius_->setDescription("Radius orbit camera spherical coordinate");

		elevation_ = new SystemVariable("orbit_camera_elevation", 0.0f);
		radius_->setDescription("Elevation orbit camera spherical coordinate [Degrees]");

		azimuth_ = new SystemVariable("orbit_camera_azimuth", 90.0f);
		azimuth_->setDescription("Azimuth orbit camera spherical coordinate [Degrees]");


		radius_change_ = new SystemVariable("orbit_camera_radius_change", 0);
		radius_change_->setDescription("Radius change signal [1 increases, 0 maintains, -1 decreases]");

		elevation_change_ = new SystemVariable("orbit_camera_elevation_change", 0);
		elevation_change_->setDescription("Elevation change signal [1 increases, 0 maintains, -1 decreases]");

		azimuth_change_ = new SystemVariable("orbit_camera_azimuth_change", 0);
		azimuth_change_->setDescription("Azimuth change signal [1 increases, 0 maintains, -1 decreases]");

		max_elevation_ = new SystemVariable("orbit_camera_max_elevation", 70.0f);
		max_elevation_->setDescription("Orbit camera maximum elevation");

		min_radius_ = new SystemVariable("orbit_camera_min_radius", 1.0f);
		min_radius_->setDescription("Orbit camera minimum radius");

		max_radius_ = new SystemVariable("orbit_camera_max_radius", 4.0f);
		max_radius_->setDescription("Orbit camera maximum radius");


		angle_increase_factor_ = new SystemVariable("orbit_camera_angle_increase_factor", 145.0f);
		angle_increase_factor_->setDescription("Orbit camera angle change filter - increase factor");

		angle_decay_factor_ = new SystemVariable("orbit_camera_angle_decay_factor", 45.0f);
		angle_decay_factor_->setDescription("Orbit camera angle change filter - decay factor");

		radius_increase_factor_ = new SystemVariable("orbit_camera_radius_increase_factor", 0.5f);
		radius_increase_factor_->setDescription("Orbit camera radius change filter - increase factor");

		radius_decay_factor_ = new SystemVariable("orbit_camera_radius_decay_factor", 0.1f);
		radius_decay_factor_->setDescription("Orbit camera radius change filter - decay factor");

		elevation_bucket_ = 0.0f;
		azimuth_bucket_ = 0.0f;
		radius_bucket_ = 0.0f;

		matrix_needs_update_ = true;
	}

	OrbitCamera::~OrbitCamera()
	{
		disconnect();
	}

	void OrbitCamera::connect()
	{
		disconnect();

		CoreEngine::instance()->system().registerVariable(radius_);
		CoreEngine::instance()->system().registerVariable(elevation_);
		CoreEngine::instance()->system().registerVariable(azimuth_);
		CoreEngine::instance()->system().registerVariable(radius_change_);
		CoreEngine::instance()->system().registerVariable(elevation_change_);
		CoreEngine::instance()->system().registerVariable(azimuth_change_);
		CoreEngine::instance()->system().registerVariable(max_elevation_);
		CoreEngine::instance()->system().registerVariable(min_radius_);
		CoreEngine::instance()->system().registerVariable(max_radius_);
		CoreEngine::instance()->system().registerVariable(angle_increase_factor_);
		CoreEngine::instance()->system().registerVariable(angle_decay_factor_);
		CoreEngine::instance()->system().registerVariable(radius_increase_factor_);
		CoreEngine::instance()->system().registerVariable(radius_decay_factor_);

		elevation_bucket_ = 0.0f;
		azimuth_bucket_ = 0.0f;
		radius_bucket_ = 0.0f;

		matrix_needs_update_ = true;
	}

	void OrbitCamera::disconnect()
	{
		CoreEngine::instance()->system().unregisterVariable(radius_);
		CoreEngine::instance()->system().unregisterVariable(elevation_);
		CoreEngine::instance()->system().unregisterVariable(azimuth_);
		CoreEngine::instance()->system().unregisterVariable(radius_change_);
		CoreEngine::instance()->system().unregisterVariable(elevation_change_);
		CoreEngine::instance()->system().unregisterVariable(azimuth_change_);
		CoreEngine::instance()->system().unregisterVariable(max_elevation_);
		CoreEngine::instance()->system().unregisterVariable(min_radius_);
		CoreEngine::instance()->system().unregisterVariable(max_radius_);
		CoreEngine::instance()->system().unregisterVariable(angle_increase_factor_);
		CoreEngine::instance()->system().unregisterVariable(angle_decay_factor_);
		CoreEngine::instance()->system().unregisterVariable(radius_increase_factor_);
		CoreEngine::instance()->system().unregisterVariable(radius_decay_factor_);
	}

	void OrbitCamera::update()
	{
		Real elapsed_time = Real( CoreEngine::instance()->frameDeltaTime() );

		if (elevation_change_->asInt() != 0)
		{
			elevation_bucket_ += Real( elevation_change_->asInt() ) * elapsed_time * angleIncreaseFactor();
		}

		if (azimuth_change_->asInt() != 0)
		{
			azimuth_bucket_ += Real( azimuth_change_->asInt() ) * elapsed_time * angleIncreaseFactor();
		}

		if (radius_change_->asInt() != 0)
		{
			radius_bucket_ += Real( radius_change_->asInt() ) * elapsed_time * radiusIncreaseFactor();
		}

		if (!equivalent(azimuth_bucket_, 0.0f))
		{
			setAzimuth(azimuth() + azimuth_bucket_ * elapsed_time);

			Real bucket_sign = sign(azimuth_bucket_);
			azimuth_bucket_ -= bucket_sign * elapsed_time * angleDecayFactor();
			if (sign(azimuth_bucket_) != bucket_sign)
			{
				azimuth_bucket_ = 0.0f;
			}

			matrix_needs_update_ = true;
		}
		else
		{
			azimuth_bucket_ = 0.0f;
		}

		if (!equivalent(elevation_bucket_, 0.0f))
		{
			setElevation(elevation() + elevation_bucket_ * elapsed_time);

			Real bucket_sign = sign(elevation_bucket_);
			elevation_bucket_ -= bucket_sign * elapsed_time * angleDecayFactor();
			if (sign(elevation_bucket_) != bucket_sign)
			{
				elevation_bucket_ = 0.0f;
			}

			if (elevation() >= maxElevation())
			{
				elevation_bucket_ = 0.0f;
				setElevation(maxElevation());
			}
			else if (elevation() <= -maxElevation())
			{
				elevation_bucket_ = 0.0f;
				setElevation(-maxElevation());
			}

			matrix_needs_update_ = true;
		}
		else
		{
			elevation_bucket_ = 0.0f;
		}

		if (!equivalent(radius_bucket_, 0.0f))
		{
			setRadius(radius() + radius_bucket_ * elapsed_time);

			Real bucket_sign = sign(radius_bucket_);
			radius_bucket_ -= bucket_sign * elapsed_time * radiusDecayFactor();
			if (sign(radius_bucket_) != bucket_sign)
			{
				radius_bucket_ = 0.0f;
			}

			if (radius() >= maxRadius())
			{
				radius_bucket_ = 0.0f;
				setRadius(maxRadius());
			}
			else if (radius() <= minRadius())
			{
				radius_bucket_ = 0.0f;
				setRadius(minRadius());
			}

			matrix_needs_update_ = true;
		}




		if (matrix_needs_update_)
		{
			Vector3D eye(radius() * cos(degreesToRadians(elevation())) * cos(degreesToRadians(azimuth())),
					     radius() * sin(degreesToRadians(elevation())),
					     radius() * cos(degreesToRadians(elevation())) * sin(degreesToRadians(azimuth())) );

			Vector3D const up(0.0f, 1.0f, 0.0f);

			view_matrix.setAsLookAt(origin_ + eye, origin_, up);

			matrix_needs_update_ = false;
		}
	}
} // namespace rengine
