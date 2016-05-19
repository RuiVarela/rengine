// __!!rengine_copyright!!__ //

#ifndef __RENGINE_WINDOWING_WIN32_H__
#define __RENGINE_WINDOWING_WIN32_H__

#include <rengine/windowing/Windowing.h>

#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0500
#endif
#include <windows.h>

namespace rengine
{
	class GraphicsWindowWin32 : public GraphicsWindow
	{
	public:
		GraphicsWindowWin32(SharedPointer< ContextOptions > const& context_options);
		virtual ~GraphicsWindowWin32();

		virtual Bool releaseContext();
		virtual void swapBuffers();
		virtual Bool makeCurrent();

		virtual Bool setWindowRectangle(Int const x, Int const y, Int const width, Int const height);
		virtual void setWindowDecoration(Bool const window_decoration);
		virtual void grabFocus();
		virtual void setWindowName(std::string const& window_name);
		virtual void setUseCursor(Bool use_cursor);
		virtual void moveCursorTo(Real const x, Real const y);
		virtual void close();

		virtual void checkInterfaceEvents();

		//
		// Win32 Specific
		//

        virtual LRESULT windowProcedure(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND windowHwnd() const { return hwnd; }
		HDC windowHdc() const { return hdc; }
		HGLRC windowWGLContext() const { return hglrc; }
	protected:
		HWND hwnd;
        HDC hdc;
        HGLRC hglrc;

		void initialize();
		void deinitialize();
		Bool initialized;

		Bool window_creator;
		Bool do_deinitialize;

		Int screen_x;
		Int screen_y;
		Uint screen_width;
		Uint screen_height;

		Bool determineWindowPositionAndStyle(Uint const screen_number,
			Int const client_x, Int const client_y, Uint const client_width, Uint const client_height,
			Int& x, Int& y, Uint& width, Uint& height,
			Uint& window_style, Uint& window_style_extended);

		Bool setPixelFormat();

		WNDPROC old_window_procedure; // for embedded windows
		HCURSOR current_cursor;

	};
} // namespace rengine
#endif // __RENGINE_WINDOWING_WIN32_H__
