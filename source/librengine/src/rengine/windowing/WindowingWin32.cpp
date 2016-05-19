// __!!rengine_copyright!!__ //

#include <rengine/windowing/WindowingWin32.h>

#include <iostream>
#include <sstream>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GL/wglew.h>

#include <windowsx.h>

#ifndef MAPVK_VSC_TO_VK_EX
#define MAPVK_VSC_TO_VK_EX 0x3
#endif //MAPVK_VSC_TO_VK_EX

namespace rengine
{
	//
	// Loggin / Errors
	//
	static std::ostream* logger = 0;
	static std::string getError(HRESULT error_code)
	{
		if (error_code == 0)
		{
			return std::string();
		}

		std::stringstream string_stream;
		string_stream <<  "Windows Error #" << error_code << ": ";

		LPVOID lpMsgBuf;
		if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, error_code, 0, (LPTSTR) &lpMsgBuf, 0, 0) != 0)
		{
			string_stream << LPTSTR(lpMsgBuf);
			::LocalFree(lpMsgBuf);
		}
		return string_stream.str();
	}

	//
	// Win32WindowingSystem
	//

	class Win32WindowingSystem : public GraphicsWindow::WindowingSystemInterface
	{
	public:
		typedef std::map<Int, Int> Win32KeyMap;
		struct OpenGLRenderContext
		{
			OpenGLRenderContext();
			~OpenGLRenderContext();

			HWND hwnd;
			HDC hdc;
			HGLRC hglrc;
		};

		typedef std::map<HWND, GraphicsWindowWin32*> HwndGraphicsWindowMap;

		Win32WindowingSystem();
		virtual ~Win32WindowingSystem();
		virtual Uint numberOfScreens(GraphicsWindow::ScreenIdentifier const& screen_identifier = GraphicsWindow::ScreenIdentifier());
		virtual Uint screenColorDepth(GraphicsWindow::ScreenIdentifier const& screen_identifier);
		virtual Uint screenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier);
		virtual void screenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint& width, Uint& height);
		virtual Bool setScreenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint width, Uint height);
		virtual void screenPosition(GraphicsWindow::ScreenIdentifier const& screen_identifier, Int& x, Int& y, Uint& width, Uint& height);
		virtual Bool setScreenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier, Real64 refresh_rate);
		virtual GraphicsWindow* createGraphicsWindow(SharedPointer< GraphicsWindow::ContextOptions > const& context_options);

		typedef std::vector<DISPLAY_DEVICE> DisplayDevices;

		DisplayDevices enumerateDisplayDevices() const;
		Bool screenInformation(GraphicsWindow::ScreenIdentifier const& screen_identifier, DISPLAY_DEVICE& display_device, DEVMODE& device_mode) const;
		Bool changeScreenInformation(GraphicsWindow::ScreenIdentifier const& screen_identifier, DISPLAY_DEVICE& display_device, DEVMODE& device_mode);

		Bool registerWindow(HWND const hwnd, GraphicsWindowWin32* window);
		void unregisterWindow(HWND const hwnd);
		GraphicsWindowWin32* findWindow(HWND const hwnd);

		std::string const& windowClassName() const { return window_class_name; }

		Bool getSampleOpenGlContext(OpenGLRenderContext& context, SharedPointer< GraphicsWindow::ContextOptions > const& context_options, Int const x, Int const y);

		void initializeGlew();

		Int mapKey(Int key);
	private:
		Win32WindowingSystem(Win32WindowingSystem const& copy) {}
		Win32WindowingSystem& operator = (Win32WindowingSystem const& copy) { return *this; }

		void registerWindowClass();
		void unregisterWindowClass();
		Bool window_classes_registered;

		std::string window_class_name;

		HwndGraphicsWindowMap windows_map;
		Win32KeyMap keymap;

		Bool glew_was_initialized;
	};

	static Win32WindowingSystem* getWindowingSystemInterface()
	{
		static Win32WindowingSystem* windowing_system_interface = new Win32WindowingSystem();
		return windowing_system_interface;
	}

	static LRESULT CALLBACK windowProcedureDispatcher(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		GraphicsWindowWin32* window = getWindowingSystemInterface()->findWindow(hwnd);
		return window ? window->windowProcedure(hwnd, uMsg, wParam, lParam) : ::DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	Win32WindowingSystem::OpenGLRenderContext::OpenGLRenderContext()
	{
		hwnd = 0;
		hdc = 0;
		hglrc = 0;
	}

	Win32WindowingSystem::OpenGLRenderContext::~OpenGLRenderContext()
	{
		if (hglrc)
		{
			::wglMakeCurrent(hdc, NULL);
			::wglDeleteContext(hglrc);
		}

		if (hdc)
		{
			::ReleaseDC(hwnd, hdc);
		}

		if (hwnd)
		{
			::DestroyWindow(hwnd);
		}
	}

	Bool Win32WindowingSystem::getSampleOpenGlContext(OpenGLRenderContext& context, SharedPointer< GraphicsWindow::ContextOptions > const& context_options, Int const x, Int const y)
	{
		context.hdc = 0;
		context.hwnd = 0;
		context.hglrc = 0;

		context.hwnd = ::CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
			getWindowingSystemInterface()->windowClassName().c_str(),
			NULL,
			WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DISABLED,
			x,
			y,
			1,
			1,
			NULL,
			NULL,
			::GetModuleHandle(NULL),
			NULL);

		if (!context.hwnd)
		{
			*logger << "WindowingSystem : Unable to create window for Sample OpenGl Context for screen number : " << context_options->screen_number
				<< std::endl << getError( ::GetLastError() ) << std::endl;
		}

		PIXELFORMATDESCRIPTOR pixel_format =
		{
			sizeof(PIXELFORMATDESCRIPTOR),
			1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL,
			PFD_TYPE_RGBA,
			24,
			0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			24,
			0,
			0,
			PFD_MAIN_PLANE,
			0,
			0, 0, 0
		};

		context.hdc = ::GetDC(context.hwnd);
		if (!context.hdc)
		{
			*logger << "WindowingSystem : Unable to get window device context for Sample OpenGl Context for screen number : " << context_options->screen_number
				<< std::endl << getError( ::GetLastError() ) << std::endl;

			::DestroyWindow(context.hwnd);
			return false;
		}

		Int pixel_format_index = ::ChoosePixelFormat(context.hdc, &pixel_format);
		if (pixel_format_index == 0)
		{
			*logger << "WindowingSystem : Unable to choose pixel format for Sample OpenGl Context for screen number : " << context_options->screen_number
				<< std::endl << getError( ::GetLastError() ) << std::endl;

			::ReleaseDC(context.hwnd, context.hdc);
			::DestroyWindow(context.hwnd);
			return false;
		}

		if (!::SetPixelFormat(context.hdc, pixel_format_index, &pixel_format))
		{
			*logger << "WindowingSystem : Unable to set pixel format for Sample OpenGl Context for screen number : " << context_options->screen_number
				<< std::endl << getError( ::GetLastError() ) << std::endl;

			::ReleaseDC(context.hwnd, context.hdc);
			::DestroyWindow(context.hwnd);
			return false;
		}

		context.hglrc  = ::wglCreateContext(context.hdc);
		if (context.hglrc ==0)
		{
			*logger << "WindowingSystem : Unable to create OpenGl rendering context for Sample OpenGl Context for screen number : " << context_options->screen_number
				<< std::endl << getError( ::GetLastError() ) << std::endl;

			::ReleaseDC(context.hwnd, context.hdc);
			::DestroyWindow(context.hwnd);
			return false;
		}

		if (!::wglMakeCurrent(context.hdc, context.hglrc))
		{
			return false;
		}

		return true;
	}

	Bool Win32WindowingSystem::registerWindow(HWND const hwnd, GraphicsWindowWin32 *window)
	{
		if (findWindow(hwnd))
		{
			return false;
		}
		else
		{
			windows_map[hwnd] = window;
			return true;
		}
	}

	void Win32WindowingSystem::unregisterWindow(HWND const hwnd)
	{
		HwndGraphicsWindowMap::iterator found = windows_map.find(hwnd);
		if (found != windows_map.end())
		{
			windows_map.erase(found);
		}
	}

	GraphicsWindowWin32* Win32WindowingSystem::findWindow(HWND const hwnd)
	{
		HwndGraphicsWindowMap::iterator found = windows_map.find(hwnd);

		if (found != windows_map.end())
		{
			return found->second;
		}
		else
		{
			return 0;
		}
	}

	Win32WindowingSystem::Win32WindowingSystem()
	{
		window_class_name = "Rengine Win32WindowingSystem Window Class Name";
		window_classes_registered = false;
		glew_was_initialized = false;

		keymap[VK_ESCAPE       ] = InterfaceEvent::KeyEscape;
		keymap[VK_F1           ] = InterfaceEvent::KeyF1;
		keymap[VK_F2           ] = InterfaceEvent::KeyF2;
		keymap[VK_F3           ] = InterfaceEvent::KeyF3;
		keymap[VK_F4           ] = InterfaceEvent::KeyF4;
		keymap[VK_F5           ] = InterfaceEvent::KeyF5;
		keymap[VK_F6           ] = InterfaceEvent::KeyF6;
		keymap[VK_F7           ] = InterfaceEvent::KeyF7;
		keymap[VK_F8           ] = InterfaceEvent::KeyF8;
		keymap[VK_F9           ] = InterfaceEvent::KeyF9;
		keymap[VK_F10          ] = InterfaceEvent::KeyF10;
		keymap[VK_F11          ] = InterfaceEvent::KeyF11;
		keymap[VK_F12          ] = InterfaceEvent::KeyF12;
		keymap[0xC0            ] = '`';
		keymap[0xBD            ] = '-';
		keymap[0xBB            ] = '=';
		keymap[VK_BACK         ] = InterfaceEvent::KeyBackSpace;
		keymap[VK_TAB          ] = InterfaceEvent::KeyTab;
		keymap[0xDB            ] = '[';
		keymap[0xDD            ] = ']';
		keymap[0xDC            ] = '\\';
		keymap[VK_CAPITAL      ] = InterfaceEvent::KeyCapsLock;
		keymap[0xBA            ] = ';';
		keymap[0xDE            ] = '\'';
		keymap[VK_RETURN       ] = InterfaceEvent::KeyReturn;
		keymap[VK_LSHIFT       ] = InterfaceEvent::KeyShiftLeft;
		keymap[0xBC            ] = ',';
		keymap[0xBE            ] = '.';
		keymap[0xBF            ] = '/';
		keymap[VK_RSHIFT       ] = InterfaceEvent::KeyShiftRight;
		keymap[VK_LCONTROL     ] = InterfaceEvent::KeyControlLeft;
		keymap[VK_LWIN         ] = InterfaceEvent::KeySuperLeft;
		keymap[VK_SPACE        ] = ' ';
		keymap[VK_LMENU        ] = InterfaceEvent::KeyAltLeft;
		keymap[VK_RMENU        ] = InterfaceEvent::KeyAltRight;
		keymap[VK_RWIN         ] = InterfaceEvent::KeySuperRight;
		keymap[VK_APPS         ] = InterfaceEvent::KeyMenu;
		keymap[VK_RCONTROL     ] = InterfaceEvent::KeyControlRight;
		keymap[VK_SNAPSHOT     ] = InterfaceEvent::KeyPrint;
		keymap[VK_SCROLL       ] = InterfaceEvent::KeyScrollLock;
		keymap[VK_PAUSE        ] = InterfaceEvent::KeyPause;
		keymap[VK_HOME         ] = InterfaceEvent::KeyHome;
		keymap[VK_PRIOR        ] = InterfaceEvent::KeyPageUp;
		keymap[VK_END          ] = InterfaceEvent::KeyEnd;
		keymap[VK_NEXT         ] = InterfaceEvent::KeyPageDown;
		keymap[VK_DELETE       ] = InterfaceEvent::KeyDelete;
		keymap[VK_INSERT       ] = InterfaceEvent::KeyInsert;
		keymap[VK_LEFT         ] = InterfaceEvent::KeyLeft;
		keymap[VK_UP           ] = InterfaceEvent::KeyUp;
		keymap[VK_RIGHT        ] = InterfaceEvent::KeyRight;
		keymap[VK_DOWN         ] = InterfaceEvent::KeyDown;
		keymap[VK_NUMLOCK      ] = InterfaceEvent::KeyNumLock;
		keymap[VK_DIVIDE       ] = InterfaceEvent::KeyKeypadDivide;
		keymap[VK_MULTIPLY     ] = InterfaceEvent::KeyKeypadMultiply;
		keymap[VK_SUBTRACT     ] = InterfaceEvent::KeyKeypadSubtract;
		keymap[VK_ADD          ] = InterfaceEvent::KeyKeypadAdd;
		keymap[VK_NUMPAD7      ] = InterfaceEvent::KeyKeypadHome;
		keymap[VK_NUMPAD8      ] = InterfaceEvent::KeyKeypadUp;
		keymap[VK_NUMPAD9      ] = InterfaceEvent::KeyKeypadPageUp;
		keymap[VK_NUMPAD4      ] = InterfaceEvent::KeyKeypadLeft;
		keymap[VK_NUMPAD5      ] = InterfaceEvent::KeyKeypadBegin;
		keymap[VK_NUMPAD6      ] = InterfaceEvent::KeyKeypadRight;
		keymap[VK_NUMPAD1      ] = InterfaceEvent::KeyKeypadEnd;
		keymap[VK_NUMPAD2      ] = InterfaceEvent::KeyKeypadDown;
		keymap[VK_NUMPAD3      ] = InterfaceEvent::KeyKeypadPageDown;
		keymap[VK_NUMPAD0      ] = InterfaceEvent::KeyKeypadInsert;
		keymap[VK_DECIMAL      ] = InterfaceEvent::KeyKeypadDelete;
		keymap[VK_CLEAR        ] = InterfaceEvent::KeyClear;
	}

	Win32WindowingSystem::~Win32WindowingSystem()
	{
		unregisterWindowClass();
	}

	Int Win32WindowingSystem::mapKey(Int key)
	{
		Win32KeyMap::iterator found = keymap.find(key);

		if (found != keymap.end())
		{
			return found->second;
		}

		return key;
	}

	Win32WindowingSystem::DisplayDevices Win32WindowingSystem::enumerateDisplayDevices() const
	{
		DisplayDevices devices;

		Bool searching = true;
		for (Uint device_number = 0; searching; ++device_number)
		{
			DISPLAY_DEVICE device;
			device.cb = sizeof(device);

			if (! ::EnumDisplayDevices(0, device_number, &device, 0) )
			{
				searching = false;
			}
			else if ( !(device.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) &&
					   (device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) )
			{
				devices.push_back(device);
			}
		}

		return devices;
	}

	Bool Win32WindowingSystem::screenInformation(GraphicsWindow::ScreenIdentifier const& screen_identifier, DISPLAY_DEVICE& display_device, DEVMODE& device_mode) const
	{
		if (screen_identifier.display_number != 0)
		{
			return false;
		}

		DisplayDevices devices = enumerateDisplayDevices();

		if ( screen_identifier.screen_number >= Int(devices.size()) )
		{
			return false;
		}

		display_device = devices[ screen_identifier.screen_number];

		device_mode.dmSize = sizeof(device_mode);
		device_mode.dmDriverExtra = 0;

		if (!::EnumDisplaySettings(display_device.DeviceName, ENUM_CURRENT_SETTINGS, &device_mode))
		{
			*logger << "WindowingSystem : Unable to query information for screen number : " << screen_identifier.screen_number
					  << std::endl << getError( ::GetLastError() ) << std::endl;
			return false;
		}

		return true;
	}

	Bool Win32WindowingSystem::changeScreenInformation(GraphicsWindow::ScreenIdentifier const& screen_identifier, DISPLAY_DEVICE& display_device, DEVMODE& device_mode)
	{
		HRESULT result = ::ChangeDisplaySettingsEx(display_device.DeviceName, &device_mode, NULL, CDS_TEST, NULL);
		if (result == DISP_CHANGE_SUCCESSFUL)
		{
			result = ::ChangeDisplaySettingsEx(display_device.DeviceName, &device_mode, NULL, 0, NULL);
			if (result == DISP_CHANGE_SUCCESSFUL)
			{
				return true;
			}
		}

		*logger << "WindowingSystem : Unable to change the screen settings for screen number : " << screen_identifier.screen_number << std::endl;
		return false;
	}

	Uint Win32WindowingSystem::numberOfScreens(GraphicsWindow::ScreenIdentifier const& screen_identifier)
	{
		return screen_identifier.display_number == 0 ? ::GetSystemMetrics(SM_CMONITORS) : 0;
	}

	Uint Win32WindowingSystem::screenColorDepth(GraphicsWindow::ScreenIdentifier const& screen_identifier)
	{
		DISPLAY_DEVICE display_device;
		DEVMODE device_mode;

		if (screenInformation(screen_identifier, display_device, device_mode))
		{
			return device_mode.dmBitsPerPel;
		}

		return 0;
	}

	void Win32WindowingSystem::screenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint& width, Uint& height)
	{
		DISPLAY_DEVICE display_device;
		DEVMODE device_mode;

		if ( screenInformation(screen_identifier, display_device, device_mode) )
		{
			width  = device_mode.dmPelsWidth;
			height = device_mode.dmPelsHeight;
		}
		else
		{
			width = 0;
			height = 0;
		}
	}

	Uint Win32WindowingSystem::screenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier)
	{
		DISPLAY_DEVICE display_device;
		DEVMODE device_mode;

		if (screenInformation(screen_identifier, display_device, device_mode))
		{
			return device_mode.dmDisplayFrequency;
		}

		return 0;
	}

	void Win32WindowingSystem::screenPosition(GraphicsWindow::ScreenIdentifier const& screen_identifier, Int& x, Int& y, Uint& width, Uint& height)
	{
		DISPLAY_DEVICE display_device;
		DEVMODE device_mode;

		if (screenInformation(screen_identifier, display_device, device_mode))
		{
			x = device_mode.dmPosition.x;
			y = device_mode.dmPosition.y;
			width = device_mode.dmPelsWidth;
			height = device_mode.dmPelsHeight;
		}
		else
		{
			x = 0;
			y = 0;
			width   = 0;
			height  = 0;
		}
	}

	Bool Win32WindowingSystem::setScreenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint width, Uint height)
	{
		DISPLAY_DEVICE display_device;
		DEVMODE device_mode;

		if (screenInformation(screen_identifier, display_device, device_mode))
		{
			device_mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
			device_mode.dmPelsWidth = width;
			device_mode.dmPelsHeight = height;

			return changeScreenInformation(screen_identifier, display_device, device_mode);
		}

		return false;
	}

	Bool Win32WindowingSystem::setScreenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier, Real64 refresh_rate)
	{
		DISPLAY_DEVICE display_device;
		DEVMODE device_mode;

		Uint width, height;
		screenResolution(screen_identifier, width, height);

		if (screenInformation(screen_identifier, display_device, device_mode))
		{
			device_mode.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;
			device_mode.dmPelsWidth = width;
			device_mode.dmPelsHeight = height;
			device_mode.dmDisplayFrequency = (DWORD)refresh_rate;

			return changeScreenInformation(screen_identifier, display_device, device_mode);
		}

		return false;
	}

	void Win32WindowingSystem::initializeGlew()
	{
		if (!glew_was_initialized)
		{
			GLenum glew_state = glewInit();
			if (glew_state != GLEW_OK)
			{
				*logger << "WindowingSystem : Error, unable to initialize glew : " << glewGetErrorString(glew_state) << std::endl;
				return;
			}
			else
			{
				glew_was_initialized = true;
			}
		}
	}

	void Win32WindowingSystem::registerWindowClass()
	{
		if (!window_classes_registered)
		{
			window_classes_registered = true;

			std::stringstream string_stream;
			string_stream << "Win32WindowingSystem WindowClassName Process" << ::GetCurrentProcessId() ;
			window_class_name = string_stream.str();

			WNDCLASSEX window_class;
			window_class.cbSize        = sizeof(window_class);
			window_class.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			window_class.lpfnWndProc   = windowProcedureDispatcher;
			window_class.cbClsExtra    = 0;
			window_class.cbWndExtra    = 0;
			window_class.hInstance     = ::GetModuleHandle(NULL);
			window_class.hIcon         = ::LoadIcon(window_class.hInstance, "APP_ICON");
			window_class.hCursor       = ::LoadCursor(NULL, IDC_ARROW);
			window_class.hbrBackground = 0;
			window_class.lpszMenuName  = 0;
			window_class.lpszClassName = window_class_name.c_str();
			window_class.hIconSm       = 0;

			if (::RegisterClassEx(&window_class) == 0)
			{
				*logger << "WindowingSystem : Unable to register window class." << std::endl << getError( ::GetLastError() ) << std::endl;
			}
			else
			{
				window_classes_registered = true;
			}
		}
	}

	void Win32WindowingSystem::unregisterWindowClass()
	{
		if (window_classes_registered)
		{
			::UnregisterClass(window_class_name.c_str(), ::GetModuleHandle(NULL));
			window_classes_registered = false;
		}
	}

	GraphicsWindow* Win32WindowingSystem::createGraphicsWindow(SharedPointer< GraphicsWindow::ContextOptions > const& context_options)
	{
		if (context_options.get())
		{
			registerWindowClass();
			return new GraphicsWindowWin32(context_options);
		}

		return 0;
	}

	//
	// GraphicsWindowWin32
	//

	GraphicsWindowWin32::GraphicsWindowWin32(SharedPointer< ContextOptions > const& context_options)
	{
		hwnd = 0;
		hdc = 0;
		hglrc = 0;

		old_window_procedure = 0;
		current_cursor = 0;

		initialized = false;
		do_deinitialize = false;

		if (context_options.get())
		{
			this->context_options = context_options;
			window_id = windowing_counter++;
			initialize();
		}
	}

	GraphicsWindowWin32::~GraphicsWindowWin32()
	{
		deinitialize();
	}

	void GraphicsWindowWin32::initialize()
	{
		if (initialized)
		{
			return;
		}

		HWND window_handle = 0;
		window_creator = (window_handle == 0);
		Bool inherited_pixel_format = false; // for embedded windows

		if (window_creator)
		{
			Uint window_style;
			Uint window_style_extended;

			Int window_x;
			Int window_y;
			Uint window_width;
			Uint window_height;

			if (!determineWindowPositionAndStyle(context_options->screen_number,
				context_options->x, context_options->y, context_options->width, context_options->height,
				window_x, window_y, window_width, window_height,
				window_style, window_style_extended))
			{
				*logger << "WindowingSystem : Unable to determine the window position and style." << std::endl;
				return;
			}

			hwnd = ::CreateWindowEx(
				window_style_extended,
				getWindowingSystemInterface()->windowClassName().c_str(),
				context_options->window_name.c_str(),
				window_style,
				window_x, window_y, window_width, window_height,
				0, 0, ::GetModuleHandle(0), 0
				);

			if (!hwnd)
			{
				*logger << "WindowingSystem : Unable to create window for screen number : " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
				return;
			}

			hdc = ::GetDC(hwnd);
			if (!hdc)
			{
				*logger << "WindowingSystem : Unable to get window device context : " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
				deinitialize();
				return;
			}

			if (!setPixelFormat())
			{
				*logger << "WindowingSystem : Unable to set pixel format : " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
				deinitialize();
				return;
			}

			if (!setWindowRectangle(context_options->x, context_options->y, context_options->width, context_options->height))
			{
				*logger << "WindowingSystem : Unable to set window rectangle" << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
				deinitialize();
				return;
			}
		}
		else // embedded window
		{
			hwnd = window_handle;

			hdc = ::GetDC(hwnd);
			if (!hdc)
			{
				*logger << "WindowingSystem : Unable to get window device context : " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
				deinitialize();
				return;
			}

			if (inherited_pixel_format)
			{
				if (!setPixelFormat())
				{
					*logger << "WindowingSystem : Unable to set pixel format : " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
					deinitialize();
					return;
				}
			}
			else
			{
				hglrc = ::wglCreateContext(hdc);
				if (!hglrc)
				{
					*logger << "WindowingSystem : Unable to create OpenGL rendering context: " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
					deinitialize();
					return;
				}
			}

			// change the window current procedure. keep the old one for procedure chaining, and restore.
			::SetLastError(0);
			old_window_procedure = (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, LONG_PTR(windowProcedureDispatcher));
			Uint error = ::GetLastError();

			if (!old_window_procedure && error)
			{
				*logger << "WindowingSystem : Unable to change window procedure: " << context_options->screen_number << std::endl << getError( error ) << std::endl;
				deinitialize();
				return;
			}
		}

		getWindowingSystemInterface()->registerWindow(hwnd, this);

		is_valid = true;
		initialized = true;
		do_deinitialize = false;

		setUseCursor(context_options->use_cursor);
		makeCurrent();

		if (wglewIsSupported("WGL_ARB_swap_control") ||
			wglewIsSupported("WGL_EXT_swap_control"))
		{
			wglSwapIntervalEXT( context_options->v_sync ? 1 : 0 );
		}
	}

	void GraphicsWindowWin32::deinitialize()
	{
		if (hdc)
		{
			releaseContext();

			if (hglrc)
			{
				::wglDeleteContext(hglrc);
				hglrc = 0;
			}

			::ReleaseDC(hwnd, hdc);
			hdc = 0;
		}

		if (old_window_procedure)
		{
			::SetLastError(0);

			WNDPROC window_procedure = (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, LONG_PTR(old_window_procedure));
			Uint error = ::GetLastError();

			if (!window_procedure && error)
			{
				*logger << "WindowingSystem : Unable to change window procedure: " << context_options->screen_number << std::endl << getError( error ) << std::endl;
			}
			old_window_procedure = 0;
		}

		if (hwnd)
		{
			getWindowingSystemInterface()->unregisterWindow(hwnd);
			if (window_creator)
			{
				::DestroyWindow(hwnd);
			}
			hwnd = 0;
		}

		initialized = false;
		is_valid = false;
	}

	static std::vector<Int> createPixelFormatSpecs(SharedPointer< GraphicsWindow::ContextOptions > const& context_options, Bool allow_swap_exchange = true)
	{
		std::vector<Int> attributes;

		// atribute id | attribute value
		attributes.push_back(WGL_DRAW_TO_WINDOW_ARB);	attributes.push_back(GL_TRUE);
		attributes.push_back(WGL_SUPPORT_OPENGL_ARB);	attributes.push_back(GL_TRUE);
		attributes.push_back(WGL_ACCELERATION_ARB);		attributes.push_back(WGL_FULL_ACCELERATION_ARB);
		attributes.push_back(WGL_PIXEL_TYPE_ARB);		attributes.push_back(WGL_TYPE_RGBA_ARB);

		attributes.push_back(WGL_COLOR_BITS_ARB);		attributes.push_back(context_options->red + context_options->green + context_options->blue);
		attributes.push_back(WGL_RED_BITS_ARB);			attributes.push_back(context_options->red);
		attributes.push_back(WGL_GREEN_BITS_ARB);		attributes.push_back(context_options->green);
		attributes.push_back(WGL_BLUE_BITS_ARB);		attributes.push_back(context_options->blue);
		attributes.push_back(WGL_DEPTH_BITS_ARB);		attributes.push_back(context_options->depth);

		if (context_options->double_buffer)
		{
			attributes.push_back(WGL_DOUBLE_BUFFER_ARB);		attributes.push_back(GL_TRUE);
			if (allow_swap_exchange)
			{
				attributes.push_back(WGL_SWAP_METHOD_ARB);		attributes.push_back(WGL_SWAP_EXCHANGE_ARB);
			}
		}


		if (context_options->alpha)
		{
			attributes.push_back(WGL_ALPHA_BITS_ARB);			attributes.push_back(context_options->alpha);
		}

		if (context_options->stencil)
		{
			attributes.push_back(WGL_STENCIL_BITS_ARB);			attributes.push_back(context_options->stencil);
		}

		if (context_options->sample_buffers)
		{
			attributes.push_back(WGL_SAMPLE_BUFFERS_ARB);		attributes.push_back(context_options->sample_buffers);
		}

		if (context_options->samples)
		{
			attributes.push_back(WGL_SAMPLE_BUFFERS_ARB);		attributes.push_back(context_options->samples);
		}

		if (context_options->quad_buffer_stereo)
		{
			attributes.push_back(WGL_STEREO_ARB);				attributes.push_back(GL_TRUE);
		}

		attributes.push_back(0); attributes.push_back(0);// end

		return attributes;
	}


	static Int ChooseMatchingPixelFormat(HDC hdc, int screenNum, 
										const std::vector<Int>& formatSpecifications,
										SharedPointer< GraphicsWindow::ContextOptions > const& context_options)
	{
		//
		// Access the entry point for the wglChoosePixelFormatARB function
		//

		if (!wglewIsSupported("WGL_ARB_pixel_format"))
		{
			Bool swapExchange = false;
			for (Int i = 0; i != (int)formatSpecifications.size(); ++i)
			{
				if ( ((i % 2) == 0) && (formatSpecifications[i] == WGL_SWAP_METHOD_ARB) )
				{
					swapExchange = true;
				}
			}

			// = openGLContext.getTraits()
			*logger << "ChooseMatchingPixelFormat() - wglChoosePixelFormatARB extension not found, trying GDI. " << screenNum << " " << ::GetLastError() << std::endl;
			PIXELFORMATDESCRIPTOR pixelFormat = {
				sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd
				1,                     // version number
				PFD_DRAW_TO_WINDOW |   // support window
				PFD_SUPPORT_OPENGL |   // support OpenGL
				(context_options->double_buffer ? PFD_DOUBLEBUFFER : NULL) |      // double buffered ?
				(swapExchange ? PFD_SWAP_EXCHANGE : PFD_SWAP_COPY),
				PFD_TYPE_RGBA,         // RGBA type
				context_options->red + context_options->green + context_options->blue,                // color depth
				context_options->red ,0, context_options->green ,0, context_options->blue, 0,          // shift bits ignored
				context_options->alpha,          // alpha buffer ?
				0,                     // shift bit ignored
				0,                     // no accumulation buffer
				0, 0, 0, 0,            // accum bits ignored
				context_options->depth,          // 32 or 16 bit z-buffer ?
				context_options->stencil,        // stencil buffer ?
				0,                     // no auxiliary buffer
				PFD_MAIN_PLANE,        // main layer
				0,                     // reserved
				0, 0, 0                // layer masks ignored
			};
			Int pixelFormatIndex = ::ChoosePixelFormat(hdc, &pixelFormat);
			if (pixelFormatIndex == 0)
			{
				*logger << "ChooseMatchingPixelFormat() - GDI ChoosePixelFormat Failed. " << screenNum << " " << ::GetLastError() << std::endl;
				return -1;
			}

			::DescribePixelFormat(hdc, pixelFormatIndex ,sizeof(PIXELFORMATDESCRIPTOR),&pixelFormat);
			if (((pixelFormat.dwFlags & PFD_GENERIC_FORMAT) != 0)  && ((pixelFormat.dwFlags & PFD_GENERIC_ACCELERATED) == 0))
			{
				*logger << "Rendering in software: pixelFormatIndex " << pixelFormatIndex << std::endl;
			}
			return pixelFormatIndex;
		}

		Int pixelFormatIndex = 0;
		Uint numMatchingPixelFormats = 0;

		if (!wglChoosePixelFormatARB(hdc, &formatSpecifications.front(), NULL, 1, &pixelFormatIndex, &numMatchingPixelFormats))
		{
			*logger << "ChooseMatchingPixelFormat - Unable to choose the requested pixel format " <<  screenNum << " " << GetLastError() << std::endl;
			return -1;
		}

		return numMatchingPixelFormats==0 ? -1 : pixelFormatIndex;
	}



	Bool GraphicsWindowWin32::setPixelFormat()
	{
		Win32WindowingSystem::OpenGLRenderContext opengl_context;
		if (!getWindowingSystemInterface()->getSampleOpenGlContext(opengl_context, context_options, screen_x, screen_y))
		{
			return false;
		}

		getWindowingSystemInterface()->initializeGlew();

		std::vector<Int> attributes = createPixelFormatSpecs(context_options);
		Int pixel_format_index = ChooseMatchingPixelFormat(opengl_context.hdc, context_options->screen_number, attributes, context_options);
				
		if (pixel_format_index < 0)
		{
			attributes = createPixelFormatSpecs(context_options, false);
			pixel_format_index = ChooseMatchingPixelFormat(opengl_context.hdc, context_options->screen_number, attributes, context_options);	
		}

		if (pixel_format_index < 0)
		{
			Uint bits_per_component = getWindowingSystemInterface()->screenColorDepth(*context_options.get()) / 4;
			context_options->red = bits_per_component;
			context_options->green = bits_per_component;
			context_options->blue = bits_per_component;
			context_options->alpha = bits_per_component;

			attributes = createPixelFormatSpecs(context_options, false);
			pixel_format_index = ChooseMatchingPixelFormat(opengl_context.hdc, context_options->screen_number, attributes, context_options);	
		}

		if (pixel_format_index < 0)
		{
			*logger << "WindowingSystem : Unable to find a compatible pixel format: " << context_options->screen_number << std::endl;
			return false;
		}

		PIXELFORMATDESCRIPTOR pixel_format_descriptor;
		::memset(&pixel_format_descriptor, 0, sizeof(pixel_format_descriptor));
		pixel_format_descriptor.nSize    = sizeof(PIXELFORMATDESCRIPTOR);
		pixel_format_descriptor.nVersion = 1;

		if (!::SetPixelFormat(hdc, pixel_format_index, &pixel_format_descriptor))
		{
			//
			// last try
			//

			attributes = createPixelFormatSpecs(context_options, false);
			pixel_format_index = ChooseMatchingPixelFormat(opengl_context.hdc, context_options->screen_number, attributes, context_options);	

			if (pixel_format_index >= 0)
			{
				if (!::SetPixelFormat(hdc, pixel_format_index, &pixel_format_descriptor))
				{
					*logger << "WindowingSystem : Unable to choose the pixel format: " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
					return false;
				}
			}
		}

		printf("TODO: WGL_ARB_create_context (not using gl 3.2)\n");
        if (false && wglewIsSupported("WGL_ARB_create_context"))
    	{
            std::vector<Int> attributes;
            int flags = 0;

            flags |= WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
            flags |= WGL_CONTEXT_CORE_PROFILE_BIT_ARB;

            if (RENGINE_DEBUG_MODE)
            {
         	   flags |= WGL_CONTEXT_DEBUG_BIT_ARB;
            }

            attributes.push_back(WGL_CONTEXT_MAJOR_VERSION_ARB);		attributes.push_back(3);
            attributes.push_back(WGL_CONTEXT_MINOR_VERSION_ARB);		attributes.push_back(2);
            attributes.push_back(WGL_CONTEXT_FLAGS_ARB);				attributes.push_back(flags);

            attributes.push_back(0);

        	hglrc = ::wglCreateContextAttribsARB(hdc, 0, &attributes.front());
            if (!hglrc)
            {
                *logger << "WindowingSystem : Unable to create OpenGL 3.x context : " << context_options->name() << std::endl;
                deinitialize();
                return false;
            }
    	}
        else
        {
    		hglrc = ::wglCreateContext(hdc);
        }

		if (!hglrc)
		{
			*logger << "WindowingSystem : Unable to create OpenGL render context: " << context_options->screen_number << std::endl << getError( ::GetLastError() ) << std::endl;
			return false;
		}

		return true;
	}

	Bool GraphicsWindowWin32::determineWindowPositionAndStyle(Uint const screen_number,
		Int const client_x, Int const client_y, Uint const client_width, Uint const client_height,
		Int& x, Int& y, Uint& width, Uint& height,
		Uint& window_style, Uint& window_style_extended)
	{
		windowingSystemInterface()->screenPosition(ScreenIdentifier(screen_number), screen_x, screen_y, screen_width, screen_height);

		x = screen_x + client_x;
		y = screen_y + client_y;
		width = client_width;
		height = client_height;

		window_style = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		window_style_extended = 0;

		if (context_options->window_decoration)
		{
			window_style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

			if (context_options->window_supports_resize)
			{
				window_style |= WS_SIZEBOX;
			}

			window_style_extended = WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW | WS_EX_ACCEPTFILES | WS_EX_LTRREADING;

			RECT corners;
			corners.left = x;
			corners.top = y;
			corners.right  = x + width - 1;
			corners.bottom = y + height - 1;

			if (!::AdjustWindowRectEx(&corners, window_style, FALSE, window_style_extended))
			{
				*logger << "WindowingSystem : Unable to adjust window rectangle for screen number : " << screen_number
					<< std::endl << getError( ::GetLastError() ) << std::endl;
				return false;
			}

			x = corners.left;
			y = corners.top;
			width = corners.right - corners.left + 1;
			height = corners.bottom - corners.top  + 1;
		}
		return true;
	}

	static Int remapKey(WPARAM wParam, LPARAM lParam)
	{
		BYTE key_state[256];
		Bool extended_key = (lParam & 0x01000000) != 0;

		// bits 16-23 Specifies the scan code. The value depends on the OEM.
		Int virtual_key = ::MapVirtualKeyEx((lParam >> 16) & 0x00FF, MAPVK_VSC_TO_VK_EX, ::GetKeyboardLayout(0));
		if (virtual_key == 0 || !::GetKeyboardState(key_state))
		{
			return 0;
		}

		if (!extended_key)
		{
			switch(virtual_key)
			{
			case VK_END :
				virtual_key = VK_NUMPAD1;
				break;
			case VK_DOWN :
				virtual_key = VK_NUMPAD2;
				break;
			case VK_NEXT :
				virtual_key = VK_NUMPAD3;
				break;
			case VK_LEFT :
				virtual_key = VK_NUMPAD4;
				break;
			case VK_CLEAR :
				virtual_key = VK_NUMPAD5;
				break;
			case VK_RIGHT :
				virtual_key = VK_NUMPAD6;
				break;
			case VK_HOME :
				virtual_key = VK_NUMPAD7;
				break;
			case VK_UP :
				virtual_key = VK_NUMPAD8;
				break;
			case VK_PRIOR :
				virtual_key = VK_NUMPAD9;
				break;
			case VK_INSERT :
				virtual_key = VK_NUMPAD0;
				break;
			case VK_DELETE :
				virtual_key = VK_DECIMAL;
				break;
			};
		}

		switch(virtual_key)
		{
		case VK_CONTROL :
		case VK_LCONTROL :
		case VK_RCONTROL :
			virtual_key = extended_key ? VK_RCONTROL : VK_LCONTROL;
			break;

		case VK_MENU :
		case VK_LMENU :
		case VK_RMENU :
			virtual_key = extended_key ? VK_RMENU : VK_LMENU;
			break;
		};

		Int key_symbol = getWindowingSystemInterface()->mapKey(virtual_key);

		if (extended_key)
		{
			if (key_symbol == InterfaceEvent::KeyReturn)
			{
				key_symbol = InterfaceEvent::KeyKeypadEnter;
			}
		}
		else
		{
			if (key_state[VK_NUMLOCK])
			{
				switch(key_symbol)
				{
				case InterfaceEvent::KeyKeypadInsert :
					key_symbol = InterfaceEvent::KeyKeypad0;
					break;
				case InterfaceEvent::KeyKeypadEnd :
					key_symbol = InterfaceEvent::KeyKeypad1;
					break;
				case InterfaceEvent::KeyKeypadDown :
					key_symbol = InterfaceEvent::KeyKeypad2;
					break;
				case InterfaceEvent::KeyKeypadPageDown :
					key_symbol = InterfaceEvent::KeyKeypad3;
					break;
				case InterfaceEvent::KeyKeypadLeft :
					key_symbol = InterfaceEvent::KeyKeypad4;
					break;
				case InterfaceEvent::KeyKeypadBegin :
					key_symbol = InterfaceEvent::KeyKeypad5;
					break;
				case InterfaceEvent::KeyKeypadRight :
					key_symbol = InterfaceEvent::KeyKeypad6;
					break;
				case InterfaceEvent::KeyKeypadHome :
					key_symbol = InterfaceEvent::KeyKeypad7;
					break;
				case InterfaceEvent::KeyKeypadUp :
					key_symbol = InterfaceEvent::KeyKeypad8;
					break;
				case InterfaceEvent::KeyKeypadPageUp :
					key_symbol = InterfaceEvent::KeyKeypad9;
					break;
				};
			}

			if ((key_symbol & 0xff00) == 0)
			{
				Char ascii_key[2];
				Int number_of_chars = ::ToAscii(wParam, (lParam >> 16) & 0xff, key_state, reinterpret_cast<WORD*>(ascii_key), 0);
				if (number_of_chars > 0)
				{
					key_symbol = ascii_key[0];
				}
			}
		}

		return key_symbol;
	}

	LRESULT GraphicsWindowWin32::windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch(uMsg)
		{
		case WM_PAINT :
			if (window_creator)
			{
				PAINTSTRUCT paint;
				::BeginPaint(hwnd, &paint);
				::EndPaint(hwnd, &paint);
			}
			break;


		case WM_SETCURSOR :
			if (context_options->use_cursor)
			{
				::SetCursor(current_cursor);
			}
			else
			{
				::SetCursor(0);
			}
			return TRUE;
			break;

		case WM_CLOSE :
			interfaceQueue().closeWindow();
			break;

		case WM_DESTROY :
			do_deinitialize = true;
			if (window_creator)
			{
				::PostQuitMessage(0);
			}
			break;

		case WM_QUIT :
			do_deinitialize = true;
			return wParam;
			break;

		case WM_MOVE :
		case WM_SIZE :
			{
				POINT origin;
				origin.x = 0;
				origin.y = 0;

				::ClientToScreen(hwnd, &origin);
				Int window_x = origin.x - screen_x;
				Int window_y = origin.y - screen_y;

				RECT client_rect;
				::GetClientRect(hwnd, &client_rect);

				Int window_width = client_rect.right;
				Int window_height = client_rect.bottom;

				if (client_rect.bottom == 0 && client_rect.right == 0) // window minimized
				{
					window_x = 1;
					window_y = 1;
					window_width = 1;
					window_height = 1;
				}

				if (window_x != context_options->x || window_y != context_options->y
					|| window_width != context_options->width || window_height != context_options->height)
				{
					// update context options
					GraphicsWindow::setWindowRectangle(window_x, window_y, window_width, window_height);
					interfaceQueue().windowResize(context_options->x, context_options->y, context_options->width, context_options->height);
				}
			}
			break;

		case WM_MOUSEMOVE :
			{
				interfaceQueue().mouseMotion(Real( GET_X_LPARAM(lParam) ), Real( GET_Y_LPARAM(lParam) ));
			}
			break;

		case WM_MOUSEWHEEL :
			{
				interfaceQueue().mouseScroll(GET_WHEEL_DELTA_WPARAM(wParam) < 0 ? InterfaceEvent::ScrollDown : InterfaceEvent::ScrollUp);
			}
			break;

		case WM_LBUTTONDOWN :
		case WM_MBUTTONDOWN :
		case WM_RBUTTONDOWN :
			{
				::SetCapture(hwnd);

				Int button = Int(InterfaceEvent::MouseRightButton);
				if (uMsg == WM_LBUTTONDOWN)
				{
					button = Int(InterfaceEvent::MouseLeftButton);
				}
				else if (uMsg == WM_MBUTTONDOWN)
				{
					button = Int(InterfaceEvent::MouseMiddleButton);
				}
				interfaceQueue().mouseButtonPress(Real( GET_X_LPARAM(lParam) ), Real( GET_Y_LPARAM(lParam) ), button);
			}
			break;

		case WM_LBUTTONDBLCLK :
		case WM_MBUTTONDBLCLK :
		case WM_RBUTTONDBLCLK :
			{
				::SetCapture(hwnd);

				Int button = Int(InterfaceEvent::MouseRightButton);
				if (uMsg == WM_LBUTTONDBLCLK)
				{
					button = Int(InterfaceEvent::MouseLeftButton);
				}
				else if (uMsg == WM_MBUTTONDBLCLK)
				{
					button = Int(InterfaceEvent::MouseMiddleButton);
				}
				interfaceQueue().mouseDoubleButtonPress(Real( GET_X_LPARAM(lParam) ), Real( GET_Y_LPARAM(lParam) ), button);
			}
			break;

		case WM_LBUTTONUP :
		case WM_MBUTTONUP :
		case WM_RBUTTONUP :
			{
				::ReleaseCapture();

				Int button = Int(InterfaceEvent::MouseRightButton);
				if (uMsg == WM_LBUTTONUP)
				{
					button = Int(InterfaceEvent::MouseLeftButton);
				}
				else if (uMsg == WM_MBUTTONUP)
				{
					button = Int(InterfaceEvent::MouseMiddleButton);
				}
				interfaceQueue().mouseButtonRelease(Real( GET_X_LPARAM(lParam) ), Real( GET_Y_LPARAM(lParam) ), button);
			}
			break;

		case WM_KEYDOWN :
		case WM_SYSKEYDOWN :
			{
                interfaceQueue().keyPress(remapKey(wParam, lParam));
			}
			break;

		case WM_KEYUP :
		case WM_SYSKEYUP :
			{
				interfaceQueue().keyRelease(remapKey(wParam, lParam));
			}
			break;

		default:
			if (window_creator)
			{
				return ::DefWindowProc(hwnd, uMsg, wParam, lParam);
			}

			break;
		};

		if (!window_creator && old_window_procedure)
		{
			return ::CallWindowProc(old_window_procedure, hwnd, uMsg, wParam, lParam);
		}

		return 0;
	}


	Bool GraphicsWindowWin32::releaseContext()
	{
		return ::wglMakeCurrent(hdc, 0) == 1;
	}

	void GraphicsWindowWin32::swapBuffers()
	{
		if (isValid())
		{
			::SwapBuffers(hdc);
		}
	}

	Bool GraphicsWindowWin32::makeCurrent()
	{
		if (isValid())
		{
			return ::wglMakeCurrent(hdc, hglrc) == 1;
		}

		return false;
	}

	Bool GraphicsWindowWin32::setWindowRectangle(Int const x, Int const y, Int const width, Int const height)
	{
		GraphicsWindow::setWindowRectangle(x, y, width, height);

		Uint window_style;
		Uint window_style_extended;

		Int window_x;
		Int window_y;
		Uint window_width;
		Uint window_height;

		if (!determineWindowPositionAndStyle(context_options->screen_number,
			context_options->x, context_options->y, context_options->width, context_options->height,
			window_x, window_y, window_width, window_height,
			window_style, window_style_extended))
		{
			*logger << "WindowingSystem : Unable to determine the window position and style." << std::endl;
			return false;
		}

		if (!::SetWindowPos(hwnd, HWND_TOP, window_x, window_y, window_width, window_height, SWP_SHOWWINDOW | SWP_FRAMECHANGED))
		{
			*logger << "WindowingSystem : Unable to set window rectangle : " << context_options->screen_number << std::endl
				<< getError( ::GetLastError() ) << std::endl;
			return false;
		}

		interfaceQueue().windowResize(context_options->x, context_options->y, context_options->width, context_options->height);

		return true;
	}

	void GraphicsWindowWin32::setWindowDecoration(Bool const window_decoration)
	{
		GraphicsWindow::setWindowDecoration(window_decoration);

		Uint window_style;
		Uint window_style_extended;

		Int window_x;
		Int window_y;
		Uint window_width;
		Uint window_height;

		if (!determineWindowPositionAndStyle(context_options->screen_number,
			context_options->x, context_options->y, context_options->width, context_options->height,
			window_x, window_y, window_width, window_height,
			window_style, window_style_extended))
		{
			*logger << "WindowingSystem : Unable to determine the window position and style." << std::endl;
			return;
		}

		::SetLastError(0);
		Uint result = ::SetWindowLong(hwnd, GWL_STYLE, window_style);
		Uint error = ::GetLastError();
		if (result == 0 && error)
		{
			*logger << "WindowingSystem : Unable to set window style for screen : " << context_options->screen_number << std::endl
				<< getError( error ) << std::endl;
			return ;
		}

		::SetLastError(0);
		result = ::SetWindowLong(hwnd, GWL_EXSTYLE, window_style_extended);
		error = ::GetLastError();
		if (result==0 && error)
		{
			*logger << "WindowingSystem : Unable to set window extended style for screen : " << context_options->screen_number << std::endl
				<< getError( error ) << std::endl;
			return;
		}

		Uint flags = SWP_FRAMECHANGED | SWP_NOZORDER | SWP_SHOWWINDOW;

		if (!::SetWindowPos(hwnd, HWND_TOP, window_x, window_y, window_width, window_height, flags))
		{
			*logger << "WindowingSystem : Unable to set window extended style for screen : " << context_options->screen_number << std::endl
				<< getError(  ::GetLastError() ) << std::endl;
			return;
		}

		::InvalidateRect(NULL, NULL, TRUE);
		//::InvalidateRect(hwnd, NULL, TRUE);
	}

	void GraphicsWindowWin32::grabFocus()
	{
		::SetForegroundWindow(hwnd);
	}

	void GraphicsWindowWin32::setWindowName(std::string const& window_name)
	{
		GraphicsWindow::setWindowName(window_name);
		::SetWindowText(hwnd, context_options->window_name.c_str());
	}

	void GraphicsWindowWin32::setUseCursor(Bool use_cursor)
	{
		GraphicsWindow::setUseCursor(use_cursor);

		if (!current_cursor)
		{
			current_cursor = LoadCursor( NULL, IDC_ARROW);
		}

		if (context_options->use_cursor)
		{
			::SetCursor(current_cursor);
		}
		else
		{
			::SetCursor(0);
		}
	}

	void GraphicsWindowWin32::moveCursorTo(Real const x, Real const y)
	{
		POINT point;
		point.x = LONG(x);
		point.y = LONG(y);

		if (!::ClientToScreen(hwnd, &point))
		{
			*logger << "WindowingSystem : Unable to convert cursor position to screen coordinates, for screen : " << context_options->screen_number << std::endl
				<< getError(  ::GetLastError() ) << std::endl;
		}

		if (!::SetCursorPos(point.x, point.y))
		{
			*logger << "WindowingSystem : Unable to set cursor position to screen coordinates, for screen : " << context_options->screen_number << std::endl
				<< getError(  ::GetLastError() ) << std::endl;
			return;
		}

		interfaceQueue().setMousePosition(x, y);
	}

	void GraphicsWindowWin32::close()
	{
		GraphicsWindow::close();
		deinitialize();
	}

	void GraphicsWindowWin32::checkInterfaceEvents()
	{
		if (!isValid())
		{
			return;
		}

		MSG msg;
		while (::PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}

		if (do_deinitialize)
		{
			deinitialize();
		}
	}

} // namespace rengine

extern "C" void initializeGraphicsWindowWin32(void)
{
	rengine::logger = 0;
	rengine::logger = &std::cout;

	rengine::GraphicsWindow::setWindowingSystemInterface( rengine::getWindowingSystemInterface() );
}

