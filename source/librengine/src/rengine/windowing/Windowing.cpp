// __!!rengine_copyright!!__ //

#include <rengine/windowing/Windowing.h>

#include <sstream>

namespace rengine
{
	//
	// InterfaceEvent
	//

#define IEV_NAME_TO_STRING_TEST(string_var, name) case name: { string_var = #name; } break;
#define IEV_STRING_TO_ENUM_TEST(string_var, uint_var, name) else if (string_var == #name) { uint_var = name; }
// cat enum_KeySymbol.txt | cut -d '=' -f1 | tr -d '\t' | tr -d ' ' | sed -e 's/^/IEV_NAME_TO_STRING_TEST(as_string, /' | sed -e 's/$/);/'

	Bool InterfaceEvent::keySymbolName(Uint const key_symbol, std::string& as_string)
	{
		switch(key_symbol)
		{
			IEV_NAME_TO_STRING_TEST(as_string, KeySpace);
			IEV_NAME_TO_STRING_TEST(as_string, KeyBackSpace);
			IEV_NAME_TO_STRING_TEST(as_string, KeyTab);
			IEV_NAME_TO_STRING_TEST(as_string, KeyLinefeed);
			IEV_NAME_TO_STRING_TEST(as_string, KeyClear);
			IEV_NAME_TO_STRING_TEST(as_string, KeyReturn);
			IEV_NAME_TO_STRING_TEST(as_string, KeyPause);
			IEV_NAME_TO_STRING_TEST(as_string, KeyScrollLock);
			IEV_NAME_TO_STRING_TEST(as_string, KeySysReq);
			IEV_NAME_TO_STRING_TEST(as_string, KeyEscape);
			IEV_NAME_TO_STRING_TEST(as_string, KeyDelete);
			IEV_NAME_TO_STRING_TEST(as_string, KeyHome);
			IEV_NAME_TO_STRING_TEST(as_string, KeyLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeyUp);
			IEV_NAME_TO_STRING_TEST(as_string, KeyRight);
			IEV_NAME_TO_STRING_TEST(as_string, KeyDown);
			IEV_NAME_TO_STRING_TEST(as_string, KeyPageUp);
			IEV_NAME_TO_STRING_TEST(as_string, KeyPageDown);
			IEV_NAME_TO_STRING_TEST(as_string, KeyEnd);
			IEV_NAME_TO_STRING_TEST(as_string, KeyBegin);
			IEV_NAME_TO_STRING_TEST(as_string, KeySelect);
			IEV_NAME_TO_STRING_TEST(as_string, KeyPrint);
			IEV_NAME_TO_STRING_TEST(as_string, KeyExecute);
			IEV_NAME_TO_STRING_TEST(as_string, KeyInsert);
			IEV_NAME_TO_STRING_TEST(as_string, KeyUndo);
			IEV_NAME_TO_STRING_TEST(as_string, KeyRedo);
			IEV_NAME_TO_STRING_TEST(as_string, KeyMenu);
			IEV_NAME_TO_STRING_TEST(as_string, KeyFind);
			IEV_NAME_TO_STRING_TEST(as_string, KeyCancel);
			IEV_NAME_TO_STRING_TEST(as_string, KeyHelp);
			IEV_NAME_TO_STRING_TEST(as_string, KeyBreak);
			IEV_NAME_TO_STRING_TEST(as_string, KeyModeSwitch);
			IEV_NAME_TO_STRING_TEST(as_string, KeyNumLock);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadSpace);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadTab);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadEnter);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadF1);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadF2);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadF3);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadF4);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadHome);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadUp);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadRight);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadDown);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadPageUp);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadPageDown);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadEnd);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadBegin);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadInsert);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadDelete);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadEqual);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadMultiply);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadAdd);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadSeparator);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadSubtract);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadDecimal);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypadDivide);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad0);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad1);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad2);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad3);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad4);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad5);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad6);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad7);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad8);
			IEV_NAME_TO_STRING_TEST(as_string, KeyKeypad9);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF1);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF2);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF3);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF4);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF5);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF6);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF7);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF8);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF9);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF10);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF11);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF12);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF13);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF14);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF15);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF16);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF17);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF18);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF19);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF20);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF21);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF22);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF23);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF24);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF25);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF26);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF27);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF28);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF29);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF30);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF31);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF32);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF33);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF34);
			IEV_NAME_TO_STRING_TEST(as_string, KeyF35);
			IEV_NAME_TO_STRING_TEST(as_string, KeyShiftLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeyShiftRight);
			IEV_NAME_TO_STRING_TEST(as_string, KeyControlLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeyControlRight);
			IEV_NAME_TO_STRING_TEST(as_string, KeyCapsLock);
			IEV_NAME_TO_STRING_TEST(as_string, KeyShiftLock);
			IEV_NAME_TO_STRING_TEST(as_string, KeyMetaLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeyMetaRight);
			IEV_NAME_TO_STRING_TEST(as_string, KeyAltLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeyAltRight);
			IEV_NAME_TO_STRING_TEST(as_string, KeySuperLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeySuperRight);
			IEV_NAME_TO_STRING_TEST(as_string, KeyHyperLeft);
			IEV_NAME_TO_STRING_TEST(as_string, KeyHyperRight);

			default:
			as_string = " ";
			as_string[0] = Char(key_symbol);
				break;
		}

		return true;
	}

	Bool InterfaceEvent::keySymbolFromName(std::string const& as_string, Uint& as_uint)
	{
		if(false) {}
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeySpace)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyBackSpace)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyTab)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyLinefeed)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyClear)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyReturn)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyPause)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyScrollLock)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeySysReq)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyEscape)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyDelete)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyHome)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyUp)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyDown)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyPageUp)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyPageDown)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyEnd)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyBegin)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeySelect)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyPrint)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyExecute)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyInsert)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyUndo)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyRedo)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyMenu)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyFind)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyCancel)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyHelp)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyBreak)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyModeSwitch)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyNumLock)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadSpace)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadTab)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadEnter)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadF1)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadF2)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadF3)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadF4)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadHome)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadUp)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadDown)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadPageUp)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadPageDown)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadEnd)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadBegin)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadInsert)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadDelete)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadEqual)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadMultiply)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadAdd)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadSeparator)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadSubtract)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadDecimal)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypadDivide)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad0)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad1)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad2)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad3)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad4)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad5)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad6)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad7)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad8)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyKeypad9)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF1)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF2)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF3)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF4)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF5)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF6)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF7)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF8)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF9)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF10)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF11)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF12)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF13)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF14)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF15)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF16)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF17)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF18)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF19)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF20)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF21)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF22)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF23)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF24)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF25)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF26)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF27)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF28)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF29)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF30)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF31)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF32)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF33)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF34)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyF35)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyShiftLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyShiftRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyControlLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyControlRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyCapsLock)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyShiftLock)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyMetaLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyMetaRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyAltLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyAltRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeySuperLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeySuperRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyHyperLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, KeyHyperRight)
		else
		{
			if ((as_string.size() > 1) || (as_string.size() == 0))
			{
				return false;
			}

			as_uint = Uint(as_string[0]);
		}

		return true;
	}

	Bool InterfaceEvent::mouseButtonName(Uint const mouse_button, std::string& as_string)
	{
		switch(mouse_button)
		{
			IEV_NAME_TO_STRING_TEST(as_string, MouseNone);
			IEV_NAME_TO_STRING_TEST(as_string, MouseLeftButton);
			IEV_NAME_TO_STRING_TEST(as_string, MouseMiddleButton);
			IEV_NAME_TO_STRING_TEST(as_string, MouseRightButton);
			default:
			return false;
			break;
		}

		return true;
	}

	Bool InterfaceEvent::mouseButtonFromName(std::string const& as_string, Uint& as_uint)
	{
		if(false) {}
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, MouseNone)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, MouseLeftButton)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, MouseMiddleButton)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, MouseRightButton)
		else
		{
			return false;
		}

		return true;
	}

	Bool InterfaceEvent::scrollName(Uint const scroll, std::string& as_string)
	{
		switch(scroll)
		{
			IEV_NAME_TO_STRING_TEST(as_string, ScrollNone);
			IEV_NAME_TO_STRING_TEST(as_string, ScrollLeft);
			IEV_NAME_TO_STRING_TEST(as_string, ScrollRight);
			IEV_NAME_TO_STRING_TEST(as_string, ScrollUp);
			IEV_NAME_TO_STRING_TEST(as_string, ScrollDown);
			IEV_NAME_TO_STRING_TEST(as_string, Scroll2D);
			default:
			return false;
				break;
		}

		return true;
	}

	Bool InterfaceEvent::scrollFromName(std::string const& as_string, Uint& as_uint)
	{
		if(false) {}
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, ScrollNone)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, ScrollLeft)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, ScrollRight)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, ScrollUp)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, ScrollDown)
		IEV_STRING_TO_ENUM_TEST(as_string, as_uint, Scroll2D)
		else
		{
			return false;
		}

		return true;
	}

	Bool InterfaceEvent::eventName(Uint const event, std::string& as_string)
	{
		switch(event)
		{
			IEV_NAME_TO_STRING_TEST(as_string, EventNone);
			IEV_NAME_TO_STRING_TEST(as_string, EventPush);
			IEV_NAME_TO_STRING_TEST(as_string, EventRelease);
			IEV_NAME_TO_STRING_TEST(as_string, EventDoubleClick);
			IEV_NAME_TO_STRING_TEST(as_string, EventDrag);
			IEV_NAME_TO_STRING_TEST(as_string, EventMove);
			IEV_NAME_TO_STRING_TEST(as_string, EventKeyDown);
			IEV_NAME_TO_STRING_TEST(as_string, EventKeyUp);
			IEV_NAME_TO_STRING_TEST(as_string, EventWindowResize);
			IEV_NAME_TO_STRING_TEST(as_string, EventWindowClose);
			IEV_NAME_TO_STRING_TEST(as_string, EventScroll);
			IEV_NAME_TO_STRING_TEST(as_string, EventQuit);
			IEV_NAME_TO_STRING_TEST(as_string, EventGeneralPurpose);
			default:
			return false;
				break;
		}

		return true;
	}


	InterfaceEvent::InterfaceEvent()
		:event_type_(EventNone),
		window_x(0),
		window_y(0),
		window_width(0),
		window_height(0),
		input_min_x(0.0f),
		input_min_y(0.0f),
		input_max_x(0.0f),
		input_max_y(0.0f),
		input_x(0.0f),
		input_y(0.0f),
		mouse_button_mask(MouseNone),
		modifier_(ModifierNone),
		key_(0),
		mouse_button_(MouseNone),
		scrolling_motion_(ScrollNone),
		scroll_x_(0.0f),
		scroll_y_(0.0f)
	{
	}

	InterfaceEvent::~InterfaceEvent()
	{
	}

	//
	// InterfaceQueue
	//

	InterfaceQueue::InterfaceQueue()
		:event_state(new InterfaceEvent()),
		timer_(new InterfaceTimer())
	{}

	InterfaceQueue::~InterfaceQueue()
	{}

	Real64 InterfaceQueue::elapsedTime() const
	{
		return timer_->elapsedTime();
	}

	void InterfaceQueue::clear()
	{
		events_.clear();
	}

	InterfaceQueue::Events::size_type InterfaceQueue::copy(Events& events)
	{
		Events::size_type size = events_.size();
		if (size)
		{
			events.insert(events.end(), events_.begin(), events_.end());
			events_.clear();
		}
		return size;
	}

	InterfaceQueue::Events::size_type InterfaceQueue::take(Events& events)
	{
		Events::size_type size = events_.size();
		if (size)
		{
			events.insert(events.end(), events_.begin(), events_.end());
			events_.clear();
		}
		return size;
	}

	void InterfaceQueue::append(Events const& events)
	{
		events_.insert(events_.end(), events.begin(), events.end());
	}

	void InterfaceQueue::append(SharedPointer< InterfaceEvent > const& new_event)
	{
		events_.push_back(new_event);
	}

	InterfaceEvent* InterfaceQueue::cloneEventState()
	{
		return new InterfaceEvent( *event_state.get() );
	}

	void InterfaceQueue::setInputRange(Real const min_x, Real const min_y, Real const max_x, Real const max_y)
	{
		currentEventState()->setInputRange(min_x, min_y, max_x, max_y);
	}

	void InterfaceQueue::windowResize(Int const x, Int const y, Int const width, Int const height, Real64 const time)
	{
		event_state->setWindowRectangle(x, y, width, height);

		SharedPointer< InterfaceEvent > event( cloneEventState() );
		event->setEventType(InterfaceEvent::EventWindowResize);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::mouseScroll(InterfaceEvent::ScrollingMotion const scrolling_motion, Real64 const time)
	{
		SharedPointer< InterfaceEvent > event( cloneEventState() );
		event->setEventType(InterfaceEvent::EventScroll);
		event->setScrollingMotion(scrolling_motion);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::mouseScroll2D(Real const x, Real const y, Real64 const time)
	{
		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventScroll);
		event->setScroll2D(x, y);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::mouseMotion(Real const x, Real const y, Real64 const time)
	{
		event_state->setInputPosition(x, y);

		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(event->mouseButtonMask() ? InterfaceEvent::EventDrag : InterfaceEvent::EventMove);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::mouseButtonPress(Real const x, Real const y, Uint const button, Real64 const time)
	{
		event_state->setInputPosition(x, y);
		event_state->setMouseButtonMask(button | event_state->mouseButtonMask());

		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventPush);
		event->setMouseButton( InterfaceEvent::MouseButton(button) );
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::mouseButtonRelease(Real const x, Real const y, Uint const button, Real64 const time)
	{
		event_state->setInputPosition(x, y);
		event_state->setMouseButtonMask(~button & event_state->mouseButtonMask());

		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventRelease);
		event->setMouseButton( InterfaceEvent::MouseButton(button) );
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::mouseDoubleButtonPress(Real const x, Real const y, Uint const button, Real64 const time)
	{
		event_state->setInputPosition(x, y);
		event_state->setMouseButtonMask(button | event_state->mouseButtonMask());

		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventDoubleClick);
		event->setMouseButton( InterfaceEvent::MouseButton(button) );
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::keyPress(Int const key, Real64 const time)
	{
		switch(key)
		{
		case(InterfaceEvent::KeyShiftLeft):
			event_state->setModifier(InterfaceEvent::ModifierShiftLeft | event_state->modifier());
			break;
		case(InterfaceEvent::KeyShiftRight):
			event_state->setModifier(InterfaceEvent::ModifierShiftRight | event_state->modifier());
			break;
		case(InterfaceEvent::KeyControlLeft):
			event_state->setModifier(InterfaceEvent::ModifierControlLeft | event_state->modifier());
			break;
		case(InterfaceEvent::KeyControlRight):
			event_state->setModifier(InterfaceEvent::ModifierControlRight | event_state->modifier());
			break;
		case(InterfaceEvent::KeyMetaLeft):
			event_state->setModifier(InterfaceEvent::ModifierMetaLeft | event_state->modifier());
			break;
		case(InterfaceEvent::KeyMetaRight):
			event_state->setModifier(InterfaceEvent::ModifierMetaRight | event_state->modifier());
			break;
		case(InterfaceEvent::KeyAltLeft):
			event_state->setModifier(InterfaceEvent::ModifierAltLeft | event_state->modifier());
			break;
		case(InterfaceEvent::KeyAltRight):
			event_state->setModifier(InterfaceEvent::ModifierAltRight | event_state->modifier());
			break;
		default:
			break;
		}

		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventKeyDown);
		event->setKey(key);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::keyRelease(Int const key, Real64 const time)
	{
		switch(key)
		{
		case(InterfaceEvent::KeyShiftLeft):
			event_state->setModifier(~InterfaceEvent::ModifierShiftLeft & event_state->modifier());
			break;
		case(InterfaceEvent::KeyShiftRight):
			event_state->setModifier(~InterfaceEvent::ModifierShiftRight & event_state->modifier());
			break;
		case(InterfaceEvent::KeyControlLeft):
			event_state->setModifier(~InterfaceEvent::ModifierControlLeft & event_state->modifier());
			break;
		case(InterfaceEvent::KeyControlRight):
			event_state->setModifier(~InterfaceEvent::ModifierControlRight & event_state->modifier());
			break;
		case(InterfaceEvent::KeyMetaLeft):
			event_state->setModifier(~InterfaceEvent::ModifierMetaLeft & event_state->modifier());
			break;
		case(InterfaceEvent::KeyMetaRight):
			event_state->setModifier(~InterfaceEvent::ModifierMetaRight & event_state->modifier());
			break;
		case(InterfaceEvent::KeyAltLeft):
			event_state->setModifier(~InterfaceEvent::ModifierAltLeft & event_state->modifier());
			break;
		case(InterfaceEvent::KeyAltRight):
			event_state->setModifier(~InterfaceEvent::ModifierAltRight & event_state->modifier());
			break;
		default:
			break;
		}

		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventKeyUp);
		event->setKey(key);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::closeWindow(Real64 time)
	{
		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventWindowClose);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::quitApplication(Real64 time)
	{
		SharedPointer< InterfaceEvent > event( cloneEventState());
		event->setEventType(InterfaceEvent::EventQuit);
		event->setTime(time);
		append(event);
	}

	void InterfaceQueue::setMousePosition(Real const x, Real const y)
	{
		currentEventState()->setInputPosition(x, y);
	}

	//
	// GraphicsWindow
	//

	GraphicsWindow::ScreenIdentifier::ScreenIdentifier()
	{
		this->display_number = 0;
		this->screen_number = 0;
	}

	GraphicsWindow::ScreenIdentifier::ScreenIdentifier(Int screen_number)
	{
		this->display_number = 0;
		this->screen_number = screen_number;
	}

	GraphicsWindow::ScreenIdentifier::ScreenIdentifier(std::string const& hostname, Int display_number, Int screen_number)
	{
		this->hostname = hostname;
		this->display_number = display_number;
		this->screen_number = screen_number;
	}

	std::string GraphicsWindow::ScreenIdentifier::name() const
	{
        std::stringstream string_stream;
        string_stream << hostname << ':' << display_number << '.' << screen_number;
        return string_stream.str();
	}

	GraphicsWindow::ContextOptions::ContextOptions()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
		window_name = "Untitled Window";
		window_decoration = true;
		window_supports_resize = true;
		window_use_cursor = true;
		red = 8;
		blue = 8;
		green = 8;
		alpha = 8;
		depth = 24;
		stencil = 0;
		sample_buffers = 0;
		samples = 0;
		double_buffer = true;
		quad_buffer_stereo = false;
		v_sync = true;
		use_cursor = true;
	}

	SharedPointer<GraphicsWindow::WindowingSystemInterface> GraphicsWindow::windowing_system_interface;
	Uint GraphicsWindow::windowing_counter;

	GraphicsWindow::GraphicsWindow()
	{
		queue = new InterfaceQueue();
		context_options = new ContextOptions();

		is_valid = false;
		window_id = 0;
	}

	GraphicsWindow::~GraphicsWindow()
	{

	}

	void GraphicsWindow::setWindowingSystemInterface(SharedPointer< WindowingSystemInterface > const& windowing_system)
	{
		windowing_counter = 0;
		windowing_system_interface = windowing_system;
	}

	GraphicsWindow::WindowingSystemInterface* GraphicsWindow::windowingSystemInterface()
	{
		return windowing_system_interface.get();
	}

	GraphicsWindow* GraphicsWindow::createGraphicsWindow(SharedPointer< ContextOptions > const& context_options)
	{
		return windowing_system_interface->createGraphicsWindow(context_options);
	}

	Bool GraphicsWindow::releaseContext()
	{
		return false;
	}

	void GraphicsWindow::swapBuffers()
	{

	}

	Bool GraphicsWindow::makeCurrent()
	{
		return false;
	}


	Bool GraphicsWindow::setWindowRectangle(Int const x, Int const y, Int const width, Int const height)
	{
		context_options->x = x;
		context_options->y = y;
		context_options->width = width;
		context_options->height = height;
		return true;
	}

	void GraphicsWindow::setWindowDecoration(Bool const window_decoration)
	{
		context_options->window_decoration = window_decoration;
	}

	void GraphicsWindow::grabFocus()
	{

	}

	void GraphicsWindow::setWindowName(std::string const& window_name)
	{
		context_options->window_name = window_name;
	}

	void GraphicsWindow::setUseCursor(Bool use_cursor)
	{
		context_options->use_cursor = use_cursor;
	}

	void GraphicsWindow::moveCursorTo(Real const x, Real const y)
	{
	}

	void GraphicsWindow::close()
	{
	}

	void GraphicsWindow::checkInterfaceEvents()
	{
	}

}  // namespace rengine
