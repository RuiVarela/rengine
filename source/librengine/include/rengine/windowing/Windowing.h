// __!!rengine_copyright!!__ //

#ifndef __RENGINE_WINDOWING_H__
#define __RENGINE_WINDOWING_H__

#include <rengine/lang/Lang.h>

#include <list>
#include <string>

namespace rengine
{
	//
	// InterfaceEvent
	//

	class InterfaceEvent
	{
	public:
		enum MouseButton
		{
			MouseNone         = 0,
			MouseLeftButton   = 1,
			MouseMiddleButton = 2,
			MouseRightButton  = 4
		};

		enum Modifier
		{
			ModifierNone          = 0,
			ModifierShiftLeft     = 1,
			ModifierShiftRight    = 2,
			ModifierControlLeft   = 4,
			ModifierControlRight  = 8,
			ModifierAltLeft       = 16,
			ModifierAltRight      = 32,
			ModifierMetaLeft      = 64,
			ModifierMetaRight     = 128,
			ModifierControl       = (ModifierControlLeft | ModifierControlRight),
			ModifierShift         = (ModifierShiftLeft | ModifierShiftRight),
			ModifierAlt           = (ModifierAltLeft | ModifierAltRight),
			ModifierMeta          = (ModifierMetaLeft | ModifierMetaRight)
		};

		enum EventType
		{
			EventNone            = 0,
			EventPush            = 1,
			EventRelease         = 2,
			EventDoubleClick     = 4,
			EventDrag            = 8,
			EventMove            = 16,
			EventKeyDown         = 32,
			EventKeyUp           = 64,
			EventWindowResize    = 128,
			EventWindowClose     = 256,
			EventScroll          = 512,
			EventQuit            = 1024,
			EventGeneralPurpose  = 2048
		};

		enum KeySymbol
		{
			KeySpace           = 0x0020,

			KeyBackSpace       = 0xFF08,
			KeyTab             = 0xFF09,
			KeyLinefeed        = 0xFF0A,
			KeyClear           = 0xFF0B,
			KeyReturn          = 0xFF0D,
			KeyPause           = 0xFF13,
			KeyScrollLock      = 0xFF14,
			KeySysReq          = 0xFF15,
			KeyEscape          = 0xFF1B,
			KeyDelete          = 0xFFFF,

			KeyHome            = 0xFF50,
			KeyLeft            = 0xFF51,
			KeyUp              = 0xFF52,
			KeyRight           = 0xFF53,
			KeyDown            = 0xFF54,
			KeyPageUp          = 0xFF55,
			KeyPageDown        = 0xFF56,
			KeyEnd             = 0xFF57,
			KeyBegin           = 0xFF58,

			KeySelect          = 0xFF60,
			KeyPrint           = 0xFF61,
			KeyExecute         = 0xFF62,
			KeyInsert          = 0xFF63,
			KeyUndo            = 0xFF65,
			KeyRedo            = 0xFF66,
			KeyMenu            = 0xFF67,
			KeyFind            = 0xFF68,
			KeyCancel          = 0xFF69,
			KeyHelp            = 0xFF6A,
			KeyBreak           = 0xFF6B,
			KeyModeSwitch      = 0xFF7E,
			KeyNumLock         = 0xFF7F,

			KeyKeypadSpace     = 0xFF80,
			KeyKeypadTab       = 0xFF89,
			KeyKeypadEnter     = 0xFF8D,
			KeyKeypadF1        = 0xFF91,
			KeyKeypadF2        = 0xFF92,
			KeyKeypadF3        = 0xFF93,
			KeyKeypadF4        = 0xFF94,
			KeyKeypadHome      = 0xFF95,
			KeyKeypadLeft      = 0xFF96,
			KeyKeypadUp        = 0xFF97,
			KeyKeypadRight     = 0xFF98,
			KeyKeypadDown      = 0xFF99,
			KeyKeypadPageUp    = 0xFF9A,
			KeyKeypadPageDown  = 0xFF9B,
			KeyKeypadEnd       = 0xFF9C,
			KeyKeypadBegin     = 0xFF9D,
			KeyKeypadInsert    = 0xFF9E,
			KeyKeypadDelete    = 0xFF9F,
			KeyKeypadEqual     = 0xFFBD,
			KeyKeypadMultiply  = 0xFFAA,
			KeyKeypadAdd       = 0xFFAB,
			KeyKeypadSeparator = 0xFFAC,
			KeyKeypadSubtract  = 0xFFAD,
			KeyKeypadDecimal   = 0xFFAE,
			KeyKeypadDivide    = 0xFFAF,

			KeyKeypad0         = 0xFFB0,
			KeyKeypad1         = 0xFFB1,
			KeyKeypad2         = 0xFFB2,
			KeyKeypad3         = 0xFFB3,
			KeyKeypad4         = 0xFFB4,
			KeyKeypad5         = 0xFFB5,
			KeyKeypad6         = 0xFFB6,
			KeyKeypad7         = 0xFFB7,
			KeyKeypad8         = 0xFFB8,
			KeyKeypad9         = 0xFFB9,

			KeyF1              = 0xFFBE,
			KeyF2              = 0xFFBF,
			KeyF3              = 0xFFC0,
			KeyF4              = 0xFFC1,
			KeyF5              = 0xFFC2,
			KeyF6              = 0xFFC3,
			KeyF7              = 0xFFC4,
			KeyF8              = 0xFFC5,
			KeyF9              = 0xFFC6,
			KeyF10             = 0xFFC7,
			KeyF11             = 0xFFC8,
			KeyF12             = 0xFFC9,
			KeyF13             = 0xFFCA,
			KeyF14             = 0xFFCB,
			KeyF15             = 0xFFCC,
			KeyF16             = 0xFFCD,
			KeyF17             = 0xFFCE,
			KeyF18             = 0xFFCF,
			KeyF19             = 0xFFD0,
			KeyF20             = 0xFFD1,
			KeyF21             = 0xFFD2,
			KeyF22             = 0xFFD3,
			KeyF23             = 0xFFD4,
			KeyF24             = 0xFFD5,
			KeyF25             = 0xFFD6,
			KeyF26             = 0xFFD7,
			KeyF27             = 0xFFD8,
			KeyF28             = 0xFFD9,
			KeyF29             = 0xFFDA,
			KeyF30             = 0xFFDB,
			KeyF31             = 0xFFDC,
			KeyF32             = 0xFFDD,
			KeyF33             = 0xFFDE,
			KeyF34             = 0xFFDF,
			KeyF35             = 0xFFE0,

			KeyShiftLeft       = 0xFFE1,
			KeyShiftRight      = 0xFFE2,
			KeyControlLeft     = 0xFFE3,
			KeyControlRight    = 0xFFE4,
			KeyCapsLock        = 0xFFE5,
			KeyShiftLock       = 0xFFE6,

			KeyMetaLeft        = 0xFFE7,
			KeyMetaRight       = 0xFFE8,
			KeyAltLeft         = 0xFFE9,
			KeyAltRight        = 0xFFEA,
			KeySuperLeft       = 0xFFEB,
			KeySuperRight      = 0xFFEC,
			KeyHyperLeft       = 0xFFED,
			KeyHyperRight      = 0xFFEE
		};

		enum ScrollingMotion
		{
			ScrollNone   = 0,
			ScrollLeft   = 1,
			ScrollRight  = 2,
			ScrollUp     = 4,
			ScrollDown   = 8,
			Scroll2D     = 16
		};

		InterfaceEvent();
		~InterfaceEvent();

		static Bool eventName(Uint const event, std::string& as_string);

		static Bool keySymbolName(Uint const key_symbol, std::string& as_string);
		static Bool mouseButtonName(Uint const mouse_button, std::string& as_string);
		static Bool scrollName(Uint const scroll, std::string& as_string);

		static Bool keySymbolFromName(std::string const& as_string, Uint& as_uint);
		static Bool mouseButtonFromName(std::string const& as_string, Uint& as_uint);
		static Bool scrollFromName(std::string const& as_string, Uint& as_uint);

		EventType eventType() const  { return event_type_; }
		void setEventType(EventType const type) { event_type_ = type; }

		Real64 time() const { return time_; }
		void setTime(Real64 const time) { time_ = time; }

		void setWindowRectangle(Int const x, Int const y, Int const width, Int const height)
		{
			window_x = x;
			window_y = y;
			window_width = width;
			window_height = height;

			setInputRange(0.0f, 0.0f, Real(window_width) - Real(1.0), Real(window_height) - Real(1.0));
		}

		Int windowX() const { return window_x; }
		Int windowY() const { return window_y; }
		Int windowWidth() const { return window_width; }
		Int windowHeight() const { return window_height; }

		void setInputRange(Real const min_x, Real const min_y, Real const max_x, Real const max_y)
		{
			input_min_x = min_x;
			input_min_y = min_y;
			input_max_x = max_x;
			input_max_y = max_y;
		}

		Real minX() const { return input_min_x; }
		Real minY() const { return input_min_y; }
		Real maxX() const { return input_max_x; }
		Real maxY() const { return input_max_y; }

		void setInputPosition(Real const x, Real const y)
		{
			input_x = x;
			input_y = y;
		}

		Real inputX() const { return input_x; }
		Real inputY() const { return input_y; }

		Real inputXnormalized() const { return inputXnormalized( inputX() ); }
		Real inputYnormalized() const { return inputYnormalized( inputY() ); }

		Real inputXnormalized(Real const x) const { return 2.0f * ((x - minX()) / (maxX() - minX())) - 1.0f; }
		Real inputYnormalized(Real const y) const { return 2.0f * ((y - minY()) / (maxY() - minY())) - 1.0f; }

		void setMouseButtonMask(Uint const button) { mouse_button_mask = MouseButton(button); }
		Uint mouseButtonMask() const { return mouse_button_mask; }

		void setModifier(Uint const modifier) { modifier_ = Modifier(modifier); }
		Uint modifier() const { return modifier_; }

		void setKey(Int const key) { key_ = key; }
		Int key() const { return key_; }
		KeySymbol keySymbol() const { return KeySymbol(key_); }

		void setMouseButton(MouseButton const button) { mouse_button_ = button; }
		MouseButton mouseButton() const { return mouse_button_; }

		void setScrollingMotion(Uint const motion)
		{
			scrolling_motion_ = ScrollingMotion(motion);

			if (scrolling_motion_ & ScrollUp) { scroll_y_ = -1.0f; }
			else if (scrolling_motion_ & ScrollDown) { scroll_y_ = 1.0f; }

			if (scrolling_motion_ & ScrollRight) { scroll_x_ = 1.0f; }
			else if (scrolling_motion_ & ScrollLeft) { scroll_x_ = -1.0f; }
		}

		Uint scrollingMotion() const { return scrolling_motion_; }

		void setScroll2D(Real const x, Real const y)
		{
			scrolling_motion_ = Scroll2D;
			scroll_x_ = x;
			scroll_y_ = y;
		}

		Real scrollX() const { return scroll_x_; }
		Real scrollY() const { return scroll_y_;  }

	private:
		EventType event_type_;
		Real64 time_;

		Int window_x;
		Int window_y;
		Int window_width;
		Int window_height;

		Real input_min_x;
		Real input_min_y;
		Real input_max_x;
		Real input_max_y;

		Real input_x;
		Real input_y;

		MouseButton mouse_button_mask;
		Modifier modifier_;
		Int key_;
		MouseButton mouse_button_;

		ScrollingMotion scrolling_motion_;
		Real scroll_x_;
		Real scroll_y_;
	};

	//
	// InterfaceTimer
	//

	class InterfaceTimer
	{
	public:
		virtual ~InterfaceTimer() {}
		virtual Real64 elapsedTime() const { return 0.0; }
	};

	//
	// InterfaceQueue
	//

	class InterfaceQueue
	{
	public:
		typedef std::list< SharedPointer< InterfaceEvent > > Events;

		InterfaceQueue();
		~InterfaceQueue();

		void clear();
		Events::size_type copy(Events& events);
		Events::size_type take(Events& events); // makes a copy of the this.events_ to events and clears this.events_, return the number of elements
		void append(Events const& events);
		void append(SharedPointer< InterfaceEvent > const& new_event);

		// Extend InterfaceTimer to implement you own timming
		void setTimer(SharedPointer< InterfaceTimer > const& timer) { timer_ = timer; }
		SharedPointer< InterfaceTimer > const& timer() const { return timer_; }
		Real64 elapsedTime() const;

		void windowResize(Int const x, Int const y, Int const width, Int const height, Real64 const time);
		void windowResize(Int const x, Int const y, Int const width, Int const height) { windowResize(x, y, width, height, elapsedTime()); }

		void mouseScroll(InterfaceEvent::ScrollingMotion const scrolling_motion, Real64 const time);
		void mouseScroll(InterfaceEvent::ScrollingMotion const scrolling_motion) { mouseScroll(scrolling_motion, elapsedTime()); }

		void mouseScroll2D(Real const x, Real const y, Real64 const time);
		void mouseScroll2D(Real const x, Real const y) { mouseScroll2D(x, y, elapsedTime()); }

		void mouseMotion(Real const x, Real const y, Real64 const time);
		void mouseMotion(Real const x, Real const y) { mouseMotion(x, y, elapsedTime()); }

		void mouseButtonPress(Real const x, Real const y, Uint const button, Real64 const time);
		void mouseButtonPress(Real const x, Real const y, Uint const button) { mouseButtonPress(x, y, button, elapsedTime()); }

		void mouseButtonRelease(Real const x, Real const y, Uint const button, Real64 const time);
		void mouseButtonRelease(Real const x, Real const y, Uint const button) { mouseButtonRelease(x, y, button, elapsedTime()); }

		void mouseDoubleButtonPress(Real const x, Real const y, Uint const button, Real64 const time);
		void mouseDoubleButtonPress(Real const x, Real const y, Uint const button) { mouseDoubleButtonPress(x, y, button, elapsedTime()); }

		void keyPress(Int const key, Real64 const time);
		void keyPress(Int const key) { keyPress(key, elapsedTime()); }

		void keyRelease(Int const key, Real64 const time);
		void keyRelease(Int const key) { keyRelease(key, elapsedTime());  }

		void closeWindow(Real64 time);
		void closeWindow() { closeWindow(elapsedTime()); }

		void quitApplication(Real64 time);
		void quitApplication() { quitApplication(elapsedTime()); }

		void setInputRange(Real const min_x, Real const min_y, Real const max_x, Real const max_y);

		SharedPointer< InterfaceEvent > const& currentEventState() const { return event_state; }

		void setMousePosition(Real const x, Real const y);
	private:
		InterfaceQueue(InterfaceQueue const& interface_queue) {}
		InterfaceQueue& operator = (InterfaceQueue const& interface_queue) { return *this; }

		InterfaceEvent* cloneEventState();
		Events events_;
		SharedPointer< InterfaceEvent > event_state;
		SharedPointer< InterfaceTimer > timer_;
	};

	//
	// GraphicsWindow
	//

	class GraphicsWindow
	{
	public:
		struct ScreenIdentifier
		{
			ScreenIdentifier();
			ScreenIdentifier(Int screen_number);
			ScreenIdentifier(std::string const& hostname, Int display_number, Int screen_number);

			std::string name() const;

			std::string hostname;
			Int display_number;
			Int screen_number;
		};

		struct ContextOptions : public ScreenIdentifier
		{
			ContextOptions();

			// window rectangle
			Int x;
			Int y;
			Int width;
			Int height;

			// window options
			std::string window_name;
			Bool window_decoration;
			Bool window_supports_resize;
			Bool window_use_cursor;

			// buffers bits
			Uint red;
			Uint blue;
			Uint green;
			Uint alpha;
			Uint depth;
			Uint stencil;

			// multi samples
			Uint sample_buffers;
			Uint samples;

			Bool double_buffer;
			Bool quad_buffer_stereo;
			Bool v_sync;

			Bool use_cursor;
		};

		//
		// Windowing interface.
		//
		struct WindowingSystemInterface
		{
			virtual ~WindowingSystemInterface() {};
			virtual Uint numberOfScreens(ScreenIdentifier const& screen_identifier = ScreenIdentifier()) = 0;
			virtual Uint screenColorDepth(ScreenIdentifier const& screen_identifier) = 0;
			virtual Uint screenRefreshRate(ScreenIdentifier const& screen_identifier) = 0;
			virtual void screenResolution(ScreenIdentifier const& screen_identifier, Uint& width, Uint& height) = 0;
			virtual void screenPosition(ScreenIdentifier const& screen_identifier, Int& x, Int& y, Uint& width, Uint& height) = 0;
			virtual Bool setScreenResolution(ScreenIdentifier const& screen_identifier, Uint width, Uint height) = 0;
			virtual Bool setScreenRefreshRate(ScreenIdentifier const& screen_identifier, Real64 refresh_rate) = 0;
			virtual GraphicsWindow* createGraphicsWindow(SharedPointer< ContextOptions > const& context_options) = 0;
		};

		static void setWindowingSystemInterface(SharedPointer< WindowingSystemInterface > const& windowing_system);
		static WindowingSystemInterface* windowingSystemInterface();
        static GraphicsWindow* createGraphicsWindow(SharedPointer< ContextOptions > const& context_options);

		//
		// GraphicsWindow methods
		//

		GraphicsWindow();
		virtual ~GraphicsWindow();

		Bool isValid() const { return is_valid; }
		ContextOptions const& contextOptions() const { return *context_options.get(); }

		InterfaceQueue const& interfaceQueue() const { return *queue.get(); }
		InterfaceQueue& interfaceQueue() { return *queue.get(); }

		virtual void swapBuffers();
		virtual Bool releaseContext();
		virtual Bool makeCurrent();

		virtual Bool setWindowRectangle(Int const x, Int const y, Int const width, Int const height);
		virtual void setWindowDecoration(Bool const window_decoration);
		virtual void grabFocus();
		virtual void setWindowName(std::string const& window_name);
		virtual void setUseCursor(Bool use_cursor);
		virtual void moveCursorTo(Real const x, Real const y);
		virtual void close();

		virtual void checkInterfaceEvents();

	protected:
		static SharedPointer<WindowingSystemInterface> windowing_system_interface;
		static Uint windowing_counter;

		Bool is_valid;
		SharedPointer< ContextOptions > context_options;
		Uint window_id;

		SharedPointer< InterfaceQueue > queue;
	};

	//
	// InterfaceEventHandler
	//

	class InterfaceEventHandler
	{
	public:
		virtual ~InterfaceEventHandler() {};
		virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window) = 0;
	};

} // namespace rengine

#if   RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32

	extern "C" { void initializeGraphicsWindowWin32(void); }
	RENGINE_INLINE void initializeWindowingSystem() { initializeGraphicsWindowWin32(); }

#elif RENGINE_PLATFORM == RENGINE_PLATFORM_LINUX

	extern "C" { void initializeGraphicsWindowX11(void); }
	RENGINE_INLINE void initializeWindowingSystem() { initializeGraphicsWindowX11(); }


#elif RENGINE_PLATFORM == RENGINE_PLATFORM_APPLE
	#pragma error "Windowing system not implemented for RENGINE_PLATFORM_APPLE"
#else
	#pragma error "Windowing system not implemented for this platform"
#endif

#endif // __RENGINE_WINDOWING_H__
