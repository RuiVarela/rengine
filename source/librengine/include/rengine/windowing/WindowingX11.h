// __!!rengine_copyright!!__ //

#ifndef __RENGINE_WINDOWING_X11_H__
#define __RENGINE_WINDOWING_X11_H__

#include <rengine/windowing/Windowing.h>

#include <GL/glew.h>
#include <GL/glxew.h>

#include <X11/X.h>
#include <X11/Xutil.h>

// Xlib defines Bool
#ifdef Bool
#	undef Bool
#endif

namespace rengine
{
	class GraphicsWindowX11 : public GraphicsWindow
	{
	public:
		GraphicsWindowX11(SharedPointer< ContextOptions > const& context_options);
		virtual ~GraphicsWindowX11();

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

		Display* X11Display() { return display; }
		Display* X11EventDisplay() { return event_display; }
		Window& X11Window() { return window; }
		Window& X11ParentWindow() { return parent_window; }
		XVisualInfo* X11VisualInfo() { return visual_info; }

	protected:
		void initialize();
		void deinitialize();

        Bool window_creator;

        Bool createVisualInfo();
        Bool setX11WindowRectangle();

        Int modifierMask() const; // computes the modifier mask from x server
        void scanNumLockMask();

        void setKeyPressed(Uint key);
        void setKeyReleased(Uint key);
        Bool isKeyPressed(Uint key) const;
        Bool isModifierKey(Uint key) const;
        Bool isNumLockActive() const;

        Bool pendingInputs();
        Bool repeatedPress(XEvent &event);

        Int remapKey(XKeyEvent& keyevent);
        Int remapKey(Int keycode);

        Display* display;
        Display* event_display;
        Window window;
        Window parent_window;
        XVisualInfo* visual_info;
        GLXFBConfig *frame_buffer_config;
        unsigned int frame_buffer_configuration;
        GLXContext glx_context;

        XSetWindowAttributes set_window_attributes;

        Atom wm_delete_window;

        Uint num_lock_mask;
        Uint modifier_state;
        Char key_state[32];
	};

} // namespace rengine


#endif // __RENGINE_WINDOWING_X11_H__
