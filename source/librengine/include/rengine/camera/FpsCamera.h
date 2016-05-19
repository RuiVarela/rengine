// __!!rengine_copyright!!__ //

#ifndef __RENGINE_FPSCAMERA_H__
#define __RENGINE_FPSCAMERA_H__

#include <rengine/system/SystemVariable.h>
#include <rengine/camera/Camera.h>
#include <rengine/windowing/Windowing.h>
#include <rengine/math/Vector.h>

namespace rengine
{
	class FpsCamera: public Camera, public InterfaceEventHandler
	{
	public:
		static Uint const number_of_keys = 6;
		enum KeyCode
		{
			KeyUp,
			KeyDown,
			KeyLeft,
			KeyRight,
			KeyFront,
			KeyBack
		};

		FpsCamera();
		virtual ~FpsCamera();

		virtual void connect();
		virtual void disconnect();

		virtual void update();
		virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window);

		Real moveFactor() const { return *move_factor_; }
		Real mouseSensitivityFactor() const { return *mouse_sensitivity_factor_; }

		Real yaw() const { return yaw_; }
		Real pitch() const { return pitch_; }
		Real roll() const { return roll_; }

		Vector3D const& position() const { return position_; }

		void setMoveFactor(Real const move_factor) { *move_factor_ = move_factor; }
		void setMouseSensitivityFactor(Real const mouse_sensitivity_factor) { *mouse_sensitivity_factor_ = mouse_sensitivity_factor; }

		void setYaw(Real const yaw) { yaw_ = yaw; matrix_needs_update_ = true; }
		void setPitch(Real const pitch) { pitch_ = pitch; matrix_needs_update_ = true; }
		void setRoll(Real const roll) { roll_ = roll; matrix_needs_update_ = true; }

		void setPosition(Vector3D const& position ) { position_ = position; matrix_needs_update_ = true; }

		Bool anyKeyPressed() const;
	private:
		Real yaw_;
		Real pitch_;
		Real roll_;

		Vector3D position_;
		Bool matrix_needs_update_;

		SharedPointer<SystemVariable> move_factor_;
		SharedPointer<SystemVariable> mouse_sensitivity_factor_;

		Bool keys_[number_of_keys];
	};

} // end of namespace

#endif // __RENGINE__FPSCAMERA_H__
