// __!!rengine_copyright!!__ //

#include <rengine/camera/FpsCamera.h>
#include <rengine/CoreEngine.h>
#include <rengine/system/System.h>

#include <string>
#include <iostream>

namespace rengine
{
	static Real const max_pitch_angle = rengine::degreesToRadians(80.0f);

	FpsCamera::FpsCamera()
		:yaw_(0.0f), pitch_(0.0f), roll_(0.0f), matrix_needs_update_(true)
	{
		move_factor_ = new SystemVariable("camera_move_factor", 1.0f);
		move_factor_->setDescription("Changes the translation speed factor.");

		mouse_sensitivity_factor_ = new SystemVariable("camera_mouse_sensitivity", 1.0f);
		mouse_sensitivity_factor_->setDescription("Changes the mouse sensitivity, > 1.0 is more sensitive.");

		for (Uint i = 0; i != number_of_keys; ++i)
		{
			keys_[i] = false;
		}
	}

	void FpsCamera::connect()
	{
		disconnect();

		CoreEngine::instance()->system().registerVariable(move_factor_);
		CoreEngine::instance()->system().registerVariable(mouse_sensitivity_factor_);

		for (Uint i = 0; i != number_of_keys; ++i)
		{
			keys_[i] = false;
		}
	}

	void FpsCamera::disconnect()
	{
		CoreEngine::instance()->system().unregisterVariable(move_factor_);
		CoreEngine::instance()->system().unregisterVariable(mouse_sensitivity_factor_);
	}

	FpsCamera::~FpsCamera()
	{
	}

	Bool FpsCamera::anyKeyPressed() const
	{
		Bool state = false;

		for (Uint i = 0; i != number_of_keys; ++i)
		{
			state |= keys_[i];
		}

		return state;
	}


	void FpsCamera::operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		switch(interface_event.eventType())
		{
		case InterfaceEvent::EventDrag :
			{
				if (interface_event.mouseButtonMask() & InterfaceEvent::MouseLeftButton)
				{
					Vector2D screen_center(interface_event.inputXnormalized( Real(interface_event.windowWidth()) / 2.0f ),
										   interface_event.inputYnormalized( Real(interface_event.windowHeight()) / 2.0f ));

					Vector2D mouse_position(interface_event.inputXnormalized(), interface_event.inputYnormalized());

					Vector2D delta = mouse_position - screen_center;
					delta.x() *= mouse_sensitivity_factor_->asFloat();
					delta.y() *= -mouse_sensitivity_factor_->asFloat();

					if ( ! (equivalent(delta.x(), 0.0f) && equivalent(delta.y(), 0.0f)) )
					{
						setPitch(pitch() + delta.y());
						setYaw(yaw() + delta.x());

						if (pitch() > max_pitch_angle)
						{
							setPitch(max_pitch_angle);
						}

						if (pitch() < -max_pitch_angle)
						{
							setPitch(-max_pitch_angle);
						}

						window->moveCursorTo(interface_event.windowWidth() / 2.0f,
											 interface_event.windowHeight() / 2.0f);
					}
				}
			}
			break;
		case InterfaceEvent::EventPush :
			{
				if (interface_event.mouseButton() == InterfaceEvent::MouseLeftButton)
				{
					window->moveCursorTo(interface_event.windowWidth() / 2.0f,
										 interface_event.windowHeight() / 2.0f);
					window->setUseCursor(false);
				}
			}
			break;
		case InterfaceEvent::EventRelease :
			{
				if (interface_event.mouseButton() == InterfaceEvent::MouseLeftButton)
				{
					window->setUseCursor(true);
				}
			}
			break;
		case InterfaceEvent::EventKeyUp :
			{
				if (interface_event.key() == 'w')
				{
					keys_[KeyFront] = false;
				}
				if (interface_event.key() == 's')
				{
					keys_[KeyBack] = false;
				}
				if (interface_event.key() == 'a')
				{
					keys_[KeyLeft] = false;
				}
				if (interface_event.key() == 'd')
				{
					keys_[KeyRight] = false;
				}
				if (interface_event.key() == 'q')
				{
					keys_[KeyUp] = false;
				}
				if (interface_event.key() == 'e')
				{
					keys_[KeyDown] = false;
				}
			}
			break;
		case InterfaceEvent::EventKeyDown :
			{
				if (interface_event.key() == 'w')
				{
					keys_[KeyFront] = true;
				}
				if (interface_event.key() == 's')
				{
					keys_[KeyBack] = true;
				}
				if (interface_event.key() == 'a')
				{
					keys_[KeyLeft] = true;
				}
				if (interface_event.key() == 'd')
				{
					keys_[KeyRight] = true;
				}
				if (interface_event.key() == 'q')
				{
					keys_[KeyUp] = true;
				}
				if (interface_event.key() == 'e')
				{
					keys_[KeyDown] = true;
				}
			}
			break;

		default :
			{}
		};
		//std::cout << gui_event->getTime() << " Event Type : " << (Int) gui_event->getEventType() << std::endl;

		//std::cout << "X : " << gui_event->getX() << " - " << gui_event->getXnormalized() << " - " << gui_event->getTime()
		//<< " [" << gui_event->getXmin() << "," << gui_event->getXmax() << "]" << std::endl;

		//std::cout << "Y : " << gui_event->getY() << " - " << gui_event->getYnormalized() << " - " << gui_event->getScrollingDeltaX()
		//	<< " [" << gui_event->getYmin() << "," << gui_event->getYmax() << "]" << std::endl;

	}

	void FpsCamera::update()
	{
		if (anyKeyPressed())
		{
			Vector3D move_vector;

			if (keys_[KeyFront])
				move_vector += Vector3D(sin(yaw()), 0.0f, -cos(yaw()));

			if (keys_[KeyBack])
				move_vector += Vector3D(-sin(yaw()), 0.0f, cos(yaw()));

			if (keys_[KeyLeft])
				move_vector += Vector3D(-cos( yaw() ), 0.0f, -sin( yaw() ) );

			if (keys_[KeyRight])
				move_vector += Vector3D(cos(yaw()), 0.0f, sin(yaw()));

			if (keys_[KeyUp])
				move_vector += Vector3D( 0.0f, 1.0f, 0.0f );

			if (keys_[KeyDown])
				move_vector += Vector3D( 0.0f, -1.0f, 0.0f );

			setPosition(position() + move_vector * (Real)CoreEngine::instance()->frameDeltaTime() * (*move_factor_));

			//std::cout << "Camera [" << position().x() << "," << position().y() << "," << position().z() << "]"
			//						<< " [" << yaw() << "," << pitch() << "," << roll() <<  "]" << std::endl;
		}

		if (matrix_needs_update_)
		{

			view_matrix = Matrix::translate(-position_) *
				Matrix(
				Quaternion::rotate(yaw()   , Vector3D(0.0f, 1.0f, 0.0f)) *
				Quaternion::rotate(-pitch(), Vector3D(1.0f, 0.0f, 0.0f)) *
				Quaternion::rotate(roll()  , Vector3D(0.0f, 0.0f, 1.0f))
				);

			matrix_needs_update_ = false;
		}
	}
} // namespace rengine
