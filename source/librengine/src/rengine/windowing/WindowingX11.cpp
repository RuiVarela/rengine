// __!!rengine_copyright!!__ //

#include <rengine/windowing/WindowingX11.h>

#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <cassert>
#include <zconf.h>

//#define XRANDR_EXTENSIONS

//
// Allow resolution change
// Allow refresh rate change
//
// link with Xrandr
#ifdef XRANDR_EXTENSIONS
#include <X11/extensions/Xrandr.h>
#endif //XRANDR_EXTENSIONS

struct WMHints
{
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
    long input_mode;
    unsigned long status;
};

#define MWM_HINTS_FUNCTIONS   (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_HINTS_INPUT_MODE  (1L << 2)
#define MWM_HINTS_STATUS      (1L << 3)

#define MWM_DECOR_ALL         (1L<<0)
#define MWM_DECOR_BORDER      (1L<<1)
#define MWM_DECOR_RESIZEH     (1L<<2)
#define MWM_DECOR_TITLE       (1L<<3)
#define MWM_DECOR_MENU        (1L<<4)
#define MWM_DECOR_MINIMIZE    (1L<<5)
#define MWM_DECOR_MAXIMIZE    (1L<<6)

#define MWM_FUNC_ALL          (1L<<0)
#define MWM_FUNC_RESIZE       (1L<<1)
#define MWM_FUNC_MOVE         (1L<<2)
#define MWM_FUNC_MINIMIZE     (1L<<3)
#define MWM_FUNC_MAXIMIZE     (1L<<4)
#define MWM_FUNC_CLOSE        (1L<<5)

#define KBAutoRepeatMode	  (1L<<7)

// Xlib defines Bool
#ifdef Bool
#	undef Bool
#endif

//
// Logging / Errors
//

namespace rengine
{
    static std::ostream* logger = 0;
}

extern "C"
{
    rengine::Int X11WindowingSystemErrorHandler(Display* display, XErrorEvent* event)
    {
        *rengine::logger << "WindowingSystem Error On Display : " << display << " Event : " << event << std::endl;

        rengine::Char buffer[256];
        XGetErrorText(display, event->error_code, buffer, 256);

        *rengine::logger << buffer << std::endl;
        *rengine::logger << "Major opcode : " << (rengine::Int) event->request_code << std::endl;
        *rengine::logger << "Minor opcode : " << (rengine::Int) event->minor_code << std::endl;
        *rengine::logger << "Error code : " << (rengine::Int) event->error_code << std::endl;
        *rengine::logger << "Request serial : " << event->serial << std::endl;
        *rengine::logger << "Current serial : " << NextRequest(display) - 1 << std::endl;

        switch ( event->error_code )
        {
        case BadValue:
            *rengine::logger << " Value : " << event->resourceid << std::endl;
            break;

        case BadAtom:
            *rengine::logger << " AtomID : " << event->resourceid << std::endl;
            break;

        default:
            *rengine::logger << " ResourceID : " << event->resourceid << std::endl;
            break;
        }
        return 0;
    }

}

namespace rengine
{
    RENGINE_INLINE void waitForWindowManager()
    {
        usleep(100000);
    }

    //
    // X11WindowingSystem
    //

    class X11WindowingSystem : public GraphicsWindow::WindowingSystemInterface
    {
    public:
        typedef std::map<Int, Int> X11KeyMap;

        X11WindowingSystem();
        virtual ~X11WindowingSystem();
        virtual Uint numberOfScreens(GraphicsWindow::ScreenIdentifier const& screen_identifier = GraphicsWindow::ScreenIdentifier());
        virtual Uint screenColorDepth(GraphicsWindow::ScreenIdentifier const& screen_identifier);
        virtual Uint screenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier);
        virtual void screenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint& width, Uint& height);
        virtual Bool setScreenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint width, Uint height);
        virtual void screenPosition(GraphicsWindow::ScreenIdentifier const& screen_identifier, Int& x, Int& y, Uint& width, Uint& height);
        virtual Bool setScreenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier, Real64 refresh_rate);
        virtual GraphicsWindow* createGraphicsWindow(SharedPointer< GraphicsWindow::ContextOptions > const& context_options);

        void initializeGlew();
    private:
        X11WindowingSystem(X11WindowingSystem const& copy) {}
        X11WindowingSystem& operator = (X11WindowingSystem const& copy)
        {
            return *this;
        }
        Bool setScreenConfiguration(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint const width, Uint const height, Uint const refresh_rate);

        Bool glew_was_initialized;
    };

    static X11WindowingSystem* getWindowingSystemInterface()
    {
        static X11WindowingSystem* windowing_system_interface = new X11WindowingSystem();
        return windowing_system_interface;
    }

    X11WindowingSystem::X11WindowingSystem()
    {
        glew_was_initialized = false;
        XSetErrorHandler(X11WindowingSystemErrorHandler);
	}

    X11WindowingSystem::~X11WindowingSystem()
    {

    }

    Uint X11WindowingSystem::numberOfScreens(GraphicsWindow::ScreenIdentifier const& screen_identifier)
    {
        Display* display = XOpenDisplay(screen_identifier.name().c_str());
        if(display)
        {
            Uint number_of_screens = XScreenCount(display);
            XCloseDisplay(display);

            return number_of_screens;
        }

        *logger << "WindowingSystem Unable to find number of screens for : " << XDisplayName(screen_identifier.name().c_str()) << std::endl;
        return 0;
    }

    Uint X11WindowingSystem::screenColorDepth(GraphicsWindow::ScreenIdentifier const& screen_identifier)
    {
        Display* display = XOpenDisplay(screen_identifier.name().c_str());
        if(display)
        {
            Int depth = XDisplayPlanes(display, screen_identifier.screen_number);
            XCloseDisplay(display);
            return depth;
        }
        else
        {
            *logger << "WindowingSystem Unable to get screen color depth for: " <<
                       XDisplayName(screen_identifier.name().c_str()) << std::endl;
        }

        return 0;
    }

    void X11WindowingSystem::screenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint& width, Uint& height)
    {
        Display* display = XOpenDisplay(screen_identifier.name().c_str());
        if(display)
        {
            width = XDisplayWidth(display, screen_identifier.screen_number);
            height = XDisplayHeight(display, screen_identifier.screen_number);
            XCloseDisplay(display);
        }
        else
        {
            *logger << "WindowingSystem Unable to get screen resolution for: " <<
                       XDisplayName(screen_identifier.name().c_str()) << std::endl;
        }
    }

    Uint X11WindowingSystem::screenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier)
    {
        Uint refresh_rate = 0;

#ifdef XRANDR_EXTENSIONS
        Display* display = XOpenDisplay(screen_identifier.name().c_str());
        if(display)
        {
            XRRScreenConfiguration* screen_configuration = XRRGetScreenInfo(display, XRootWindow(display, screen_identifier.screen_number));
            if (screen_configuration)
            {
                refresh_rate = (Uint) XRRConfigCurrentRate (screen_configuration);
                XRRFreeScreenConfigInfo(screen_configuration);
            }
            XCloseDisplay(display);
        }
#endif //XRANDR_EXTENSIONS

        return refresh_rate;
    }

    void X11WindowingSystem::screenPosition(GraphicsWindow::ScreenIdentifier const& screen_identifier, Int& x, Int& y, Uint& width, Uint& height)
    {
    	//TODO: this is not implemented
        x = 0;
        y = 0;
        screenResolution(screen_identifier, width, height);
    }

    Bool X11WindowingSystem::setScreenConfiguration(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint const width, Uint const height, Uint const refresh_rate)
    {
        Bool changed = false;

#ifdef XRANDR_EXTENSIONS
        Display* display = XOpenDisplay(screen_identifier.name().c_str());
        if(display)
        {
            XRRScreenConfiguration* screen_configuration = XRRGetScreenInfo(display, XRootWindow(display, screen_identifier.screen_number));
            if (screen_configuration)
            {
                Int screen_width = Int( (width > 0) ? width : XDisplayWidth(display, screen_identifier.screen_number) );
                Int screen_height = Int( (height > 0) ? height : XDisplayHeight(display, screen_identifier.screen_number) );
                Int screen_refresh_rate = Int(refresh_rate);
                Rotation screen_rotation = 0;
                Int number_of_screens = 0;


                XRRConfigRotations(screen_configuration, &screen_rotation);
                XRRScreenSize* screen_sizes = XRRConfigSizes(screen_configuration, &number_of_screens);

                for (Int i = 0; i != number_of_screens; ++i)
                {
                    if ((screen_sizes[i].width == screen_width) && (screen_sizes[i].height == screen_height))
                    {
                        Int number_of_rates = 0;
                        short* rates = XRRConfigRates(screen_configuration, i, &number_of_rates);

                        for (Int rate_iterator = 0; rate_iterator != number_of_rates; ++rate_iterator)
                        {
                            if (rates[rate_iterator] == screen_refresh_rate)
                            {
                                 if(XRRSetScreenConfigAndRate(display, screen_configuration, DefaultRootWindow(display), i, screen_rotation, screen_refresh_rate, CurrentTime) == RRSetConfigSuccess)
                                 {
                                     changed = true;
                                 }
                            }
                        }
                    }
                }

                XRRFreeScreenConfigInfo(screen_configuration);
            }
            XCloseDisplay(display);
        }
#endif //XRANDR_EXTENSIONS

    return changed;
    }

    Bool X11WindowingSystem::setScreenResolution(GraphicsWindow::ScreenIdentifier const& screen_identifier, Uint width, Uint height)
    {
        return setScreenConfiguration(screen_identifier, width, height, 0);
    }

    Bool X11WindowingSystem::setScreenRefreshRate(GraphicsWindow::ScreenIdentifier const& screen_identifier, Real64 refresh_rate)
    {
        return setScreenConfiguration(screen_identifier, 0, 0, refresh_rate);
    }

    void X11WindowingSystem::initializeGlew()
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

    GraphicsWindow* X11WindowingSystem::createGraphicsWindow(SharedPointer< GraphicsWindow::ContextOptions > const& context_options)
    {
        if (context_options.get())
        {
            return new GraphicsWindowX11(context_options);
        }

        return 0;
    }

    //
    // GraphicsWindowX11
    //

    GraphicsWindowX11::GraphicsWindowX11(SharedPointer< ContextOptions > const& context_options)
    {
        window_creator = false;

        display = 0;
        event_display = 0;
        window = 0;
        visual_info = 0;
        frame_buffer_config = 0;
        glx_context = 0;


        modifier_state = 0;
        num_lock_mask = 0;

        if (context_options.get())
        {
            this->context_options = context_options;
            window_id = windowing_counter++;
            initialize();
        }
    }

    GraphicsWindowX11::~GraphicsWindowX11()
    {
        deinitialize();
    }

    Bool GraphicsWindowX11::createVisualInfo()
    {
        if (frame_buffer_config)
        {
            XFree(frame_buffer_config);
            frame_buffer_config = 0;
        }

        if (visual_info)
        {
            XFree(visual_info);
            visual_info = 0;
        }

        if (window)
        {
            XWindowAttributes window_attributes;
            XGetWindowAttributes(display, window, &window_attributes);

            XVisualInfo info;
            info.visualid = XVisualIDFromVisual(window_attributes.visual);
            Int matching_visual_structures;
            visual_info = XGetVisualInfo(display, VisualIDMask, &info, &matching_visual_structures);
        }
        else
        {
            std::vector<Int> attributes;

            attributes.push_back(GLX_X_RENDERABLE);			attributes.push_back(GL_TRUE);
            attributes.push_back(GLX_DRAWABLE_TYPE);		attributes.push_back(GLX_WINDOW_BIT);
            attributes.push_back(GLX_RENDER_TYPE);			attributes.push_back(GLX_RGBA_BIT);
            attributes.push_back(GLX_X_VISUAL_TYPE);		attributes.push_back(GLX_TRUE_COLOR);
            attributes.push_back(GLX_DOUBLEBUFFER);			attributes.push_back(context_options->double_buffer ? GL_TRUE : GL_FALSE);

            attributes.push_back(GLX_RED_SIZE);				attributes.push_back(context_options->red);
            attributes.push_back(GLX_GREEN_SIZE);			attributes.push_back(context_options->green);
            attributes.push_back(GLX_BLUE_SIZE);			attributes.push_back(context_options->blue);
            attributes.push_back(GLX_ALPHA_SIZE);   		attributes.push_back(context_options->alpha);
            attributes.push_back(GLX_DEPTH_SIZE);			attributes.push_back(context_options->depth);
            attributes.push_back(GLX_STENCIL_SIZE); 		attributes.push_back(context_options->stencil);

            attributes.push_back(GLX_STEREO);				attributes.push_back(context_options->quad_buffer_stereo ? GL_TRUE : GL_FALSE);

            attributes.push_back(GLX_SAMPLE_BUFFERS);		attributes.push_back((context_options->sample_buffers > 0) ? GL_TRUE : GL_FALSE);


            if ((context_options->sample_buffers > 0) && context_options->samples)
            {
                attributes.push_back(GLX_SAMPLES);      	attributes.push_back(context_options->samples);
            }


            attributes.push_back(None);

            typedef GLXFBConfig* (*MY_glXChooseFBConfig) (Display *dpy, int screen, const int *attrib_list, int *nelements);
            MY_glXChooseFBConfig my_glXChooseFBConfig = (MY_glXChooseFBConfig) glXGetProcAddress((const GLubyte*)"glXChooseFBConfig");

            typedef XVisualInfo* (*MY_glXGetVisualFromFBConfig) (Display *dpy, GLXFBConfig config);
            MY_glXGetVisualFromFBConfig my_glXGetVisualFromFBConfig = (MY_glXGetVisualFromFBConfig) glXGetProcAddress((const GLubyte*)"glXGetVisualFromFBConfig");

            int number_of_elements;
            frame_buffer_config = my_glXChooseFBConfig(display, context_options->screen_number, &attributes.front(), &number_of_elements);
            frame_buffer_configuration = 0;

            if (frame_buffer_config)
            {
                visual_info = my_glXGetVisualFromFBConfig(display, frame_buffer_config[frame_buffer_configuration]);
            }
        }

         return (visual_info != 0);
    }

    Bool GraphicsWindowX11::setX11WindowRectangle()
    {
        Uint screen_width;
        Uint screen_height;
        getWindowingSystemInterface()->screenResolution(*context_options.get(), screen_width, screen_height);

        Bool is_fullscreen = (
            (context_options->x == 0) && (context_options->y == 0) &&
            (context_options->width == (Int)screen_width) && (context_options->height == (Int)screen_height) &&
            !context_options->window_decoration );

        Atom net_wm_state_atom = XInternAtom(display, "_NET_WM_STATE", True);
        Atom net_wm_state_fullscreen_atom = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);

        if ( (net_wm_state_atom != None) && (net_wm_state_fullscreen_atom != None) )
        {
            XEvent x_event;
            x_event.xclient.type = ClientMessage;
            x_event.xclient.serial = 0;
            x_event.xclient.send_event = True;
            x_event.xclient.window = window;
            x_event.xclient.message_type = net_wm_state_atom;
            x_event.xclient.format = 32;
            x_event.xclient.data.l[0] = is_fullscreen ? 1 : 0;
            x_event.xclient.data.l[1] = net_wm_state_fullscreen_atom;
            x_event.xclient.data.l[2] = 0;

            XSendEvent(display, RootWindow(display, DefaultScreen(display)), False,  SubstructureRedirectMask | SubstructureNotifyMask, &x_event);
            return true;
        }
        return false;
    }

    void GraphicsWindowX11::initialize()
    {
        if (isValid())
        {
            return;
        }

        for (Int i = 0; i < 32; ++i)
		{
			key_state[i] = 0;
		}

        Window window_handle = 0;
        window_creator = (window_handle == 0);

        display = XOpenDisplay(context_options->name().c_str());

        if (!display)
        {
            *logger << "WindowingSystem : Unable to open display : " << context_options->name() << std::endl;
            return;
        }

        Int error, event;
        if(glXQueryExtension(display, & error, &event) == false)
        {
            *logger << "WindowingSystem : GLX extension not available : " << context_options->name() << std::endl;
            deinitialize();
            return;
        }

        if (!createVisualInfo())
        {
            Uint bits_per_component = getWindowingSystemInterface()->screenColorDepth(*context_options.get()) / 4;
			context_options->red = bits_per_component;
			context_options->green = bits_per_component;
			context_options->blue = bits_per_component;
			context_options->alpha = bits_per_component;

			if (!createVisualInfo())
			{
			    *logger << "WindowingSystem : Unable to create visual info requested : " << context_options->name() << std::endl;
                deinitialize();
                return;
			}
        }

        glx_context = glXCreateContext(display, visual_info, 0, true);
        if (!glx_context)
        {
            *logger << "WindowingSystem : Unable to create OpenGL context : " << context_options->name() << std::endl;
            deinitialize();
            return;
        }


        if (window_creator)
        {
            event_display = XOpenDisplay(context_options->name().c_str());
            parent_window = RootWindow(display, context_options->screen_number);

            XSetWindowAttributes set_window_attributes;
            set_window_attributes.colormap = XCreateColormap(display, parent_window, visual_info->visual, AllocNone);
            set_window_attributes.background_pixel = 0;
            set_window_attributes.border_pixel = 0;
            set_window_attributes.event_mask = 0;

            unsigned long mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;

            window = XCreateWindow(display, parent_window,
                             context_options->x, context_options->y, context_options->width, context_options->height, 0,
                             visual_info->depth, InputOutput,
                             visual_info->visual, mask, &set_window_attributes);

            if (!window)
            {
                *logger << "WindowingSystem : Unable to create window  : " << context_options->name() << std::endl;
                deinitialize();
                return;
            }

            XSizeHints size_hints;
            size_hints.flags = USSize | USPosition;
            size_hints.x = context_options->x;
            size_hints.y = context_options->y;
            size_hints.width = context_options->width;
            size_hints.height = context_options->height;

            XSetStandardProperties(display, window, context_options->window_name.c_str(), context_options->window_name.c_str(), None, 0, 0, &size_hints);

            wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
            XSetWMProtocols(display, window, &wm_delete_window, 1);

            XFlush(display);
            XSync(display, 0);

            XSelectInput(event_display, window,  StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                                                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                                                 KeymapStateMask | FocusChangeMask);
            XFlush(event_display);
            XSync(event_display, 0);

            if (!context_options->use_cursor)
            {
            	context_options->use_cursor = true;
            	setUseCursor(false);
            }

            setWindowDecoration(context_options->window_decoration);
        }
        else //embebeded
        {
            window = window_handle;

            event_display = XOpenDisplay(context_options->name().c_str());
            parent_window = DefaultRootWindow(display);

            XWindowAttributes window_attributes;
            XGetWindowAttributes(display, window, &window_attributes);

            context_options->x = window_attributes.x;
            context_options->y = window_attributes.y;
            context_options->width = window_attributes.width;
            context_options->height = window_attributes.height;

            context_options->window_decoration = false;
            setWindowName(context_options->window_name);
        }

		is_valid = true;

        makeCurrent();

        getWindowingSystemInterface()->initializeGlew();

        if (glxewIsSupported("GLX_ARB_create_context"))
    	{
           releaseContext();
           glXDestroyContext(display, glx_context);


           std::vector<Int> attributes;
           int flags = 0;

           flags |= GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
           flags |= GLX_CONTEXT_CORE_PROFILE_BIT_ARB;

           if (RENGINE_DEBUG_MODE)
           {
        	   flags |= GLX_CONTEXT_DEBUG_BIT_ARB;
           }

           attributes.push_back(GLX_CONTEXT_MAJOR_VERSION_ARB);		attributes.push_back(3);
           attributes.push_back(GLX_CONTEXT_MINOR_VERSION_ARB);		attributes.push_back(2);
           attributes.push_back(GLX_CONTEXT_FLAGS_ARB);				attributes.push_back(flags);

           attributes.push_back(None);

           glx_context = glXCreateContextAttribsARB(display, frame_buffer_config[frame_buffer_configuration], 0, True, &attributes.front());
           if (!glx_context)
           {
               *logger << "WindowingSystem : Unable to create OpenGL 3.x context : " << context_options->name() << std::endl;
               deinitialize();
               return;
           }

           makeCurrent();
    	}

        // It has been reported that in some case (some driver) with ATI, it doesn't give GLX_SGI_swap_control.
        // It gives WGL_EXT_swap_control yet the driver doesn't export wglSwapIntervalEXT but it does export glXSwapIntervalSGI and it works.

        if (glxewIsSupported("GLX_SGI_swap_control") || glxewIsSupported("WGL_EXT_swap_control"))
		{
			glXSwapIntervalSGI(context_options->v_sync ? 1 : 0 );
		}

        scanNumLockMask();

        // disable auto repeat
        //XKeyboardControl keyboard_control;
        //keyboard_control.auto_repeat_mode = AutoRepeatModeOn;
        //XChangeKeyboardControl(event_display, KBAutoRepeatMode, &keyboard_control);
    }

    void GraphicsWindowX11::deinitialize()
    {
    	if (frame_buffer_config)
    	{
    		XFree(frame_buffer_config);
    		frame_buffer_config = 0;
    	}

        if (visual_info)
        {
            XFree(visual_info);
            visual_info = 0;
        }

        if (event_display)
        {
            XCloseDisplay(event_display);
            event_display = 0;
        }

        if (display)
        {
            if (glx_context)
            {
                glXDestroyContext(display, glx_context);
                glx_context = 0;
            }

            if (window_creator && window)
            {
                XDestroyWindow(display, window);
                window_creator = false;
                window = 0;
            }

            XFlush(display );
            XSync(display,0 );

            XCloseDisplay(display);
            display = 0;
        }

        is_valid = false;
    }

    void GraphicsWindowX11::scanNumLockMask()
    {
    	num_lock_mask = 0;

		XModifierKeymap *modifier_key_map = XGetModifierMapping(event_display);
		KeyCode *modifier = modifier_key_map->modifiermap;

        KeyCode numlock = XKeysymToKeycode(event_display, XK_Num_Lock);

        for (Int i = 0; i < modifier_key_map->max_keypermod * 8; i++)
        {
            if (*modifier == numlock)
            {
            	num_lock_mask = 1 << (i / modifier_key_map->max_keypermod);
            }
            ++modifier;
        }

        XFreeModifiermap(modifier_key_map);
    }

    Int GraphicsWindowX11::modifierMask() const
	{
		Int mask = 0;
		XModifierKeymap *modifier_key_map = XGetModifierMapping(event_display);
		KeyCode *modifier = modifier_key_map->modifiermap;

		for (Int i = 0; i < modifier_key_map->max_keypermod * 8; i++)
		{
			if (*modifier && isKeyPressed(*modifier))
			{
				mask |= 1 << (i / modifier_key_map->max_keypermod);
			}
			++modifier;
		}

		XFreeModifiermap(modifier_key_map);
		return mask;
	}

	Int GraphicsWindowX11::remapKey(XKeyEvent& keyevent)
	{
		//KeySym key_symbol = XKeycodeToKeysym(event_display, keyevent.keycode, 0);
		KeySym key_symbol = XLookupKeysym(&keyevent, 0);

		if ( (key_symbol & 0xff00) == 0) // is ascii key?
		{
			Uchar buffer[32];
			XLookupString(&keyevent, (Char*)buffer, sizeof(buffer), 0, 0);
			key_symbol = buffer[0];
		}


		if (isNumLockActive())
		{
			switch (key_symbol)
			{
				case InterfaceEvent::KeyKeypadInsert:
				key_symbol = InterfaceEvent::KeyKeypad0;
					break;
				case InterfaceEvent::KeyKeypadEnd:
				key_symbol = InterfaceEvent::KeyKeypad1;
					break;
				case InterfaceEvent::KeyKeypadDown:
				key_symbol = InterfaceEvent::KeyKeypad2;
					break;
				case InterfaceEvent::KeyKeypadPageDown:
				key_symbol = InterfaceEvent::KeyKeypad3;
					break;
				case InterfaceEvent::KeyKeypadLeft:
				key_symbol = InterfaceEvent::KeyKeypad4;
					break;
				case InterfaceEvent::KeyKeypadBegin:
				key_symbol = InterfaceEvent::KeyKeypad5;
					break;
				case InterfaceEvent::KeyKeypadRight:
				key_symbol = InterfaceEvent::KeyKeypad6;
					break;
				case InterfaceEvent::KeyKeypadHome:
				key_symbol = InterfaceEvent::KeyKeypad7;
					break;
				case InterfaceEvent::KeyKeypadUp:
				key_symbol = InterfaceEvent::KeyKeypad8;
					break;
				case InterfaceEvent::KeyKeypadPageUp:
				key_symbol = InterfaceEvent::KeyKeypad9;
					break;
			};
		}

		return key_symbol;
	}

	Int GraphicsWindowX11::remapKey(Int keycode)
	{
	    XKeyEvent keyevent;
	    keyevent.serial = 0;
	    keyevent.send_event = True;
	    keyevent.display = event_display;
	    keyevent.window = window;
	    keyevent.subwindow = 0;
	    keyevent.time = 0;
	    keyevent.x = 0;
	    keyevent.y = 0;
	    keyevent.x_root = 0;
	    keyevent.y_root = 0;
	    keyevent.state = modifier_state;
	    keyevent.keycode = keycode;
	    keyevent.same_screen = True;

		return remapKey(keyevent);
	}

    void GraphicsWindowX11::setKeyPressed(Uint key)
	{
		key_state[(key & 0xff) / 8] |= (1 << (key & 7));
	}

	void GraphicsWindowX11::setKeyReleased(Uint key)
	{
		key_state[(key & 0xff) / 8] &= ~(1 << (key & 7));
	}

	Bool GraphicsWindowX11::isKeyPressed(Uint key) const
	{
		return (key_state[(key & 0xff) / 8] & (1 << (key & 7))) != 0;
	}

	Bool GraphicsWindowX11::isModifierKey(Uint key) const
	{
		Bool is_modifier = false;

		XModifierKeymap *modifier_key_map = XGetModifierMapping(event_display);
		KeyCode *modifier = modifier_key_map->modifiermap;

		for (Int i = 0; (i < modifier_key_map->max_keypermod * 8) && !is_modifier; i++)
		{
			if (*modifier == key)
			{
				is_modifier = true;
			}
			++modifier;
		}

		XFreeModifiermap(modifier_key_map);
		return is_modifier;
	}

	Bool GraphicsWindowX11::isNumLockActive() const
    {
		return ((modifier_state & num_lock_mask) != 0);
    }

	Bool GraphicsWindowX11::pendingInputs()
	{
		XFlush(event_display);
		if (XEventsQueued(event_display, QueuedAlready))
		{
			return true;
		}

		static struct timeval zero_time;
		Int x11_file_descriptor;
		fd_set fdset;

		x11_file_descriptor = ConnectionNumber(event_display);
		FD_ZERO(&fdset);
		FD_SET(x11_file_descriptor, &fdset);
		if (select(x11_file_descriptor + 1, &fdset, NULL, NULL, &zero_time) == 1)
		{
			return XPending(event_display);
		}

		return false;
	}

	Bool GraphicsWindowX11::repeatedPress(XEvent &event)
	{
		if (pendingInputs())
		{
			XEvent next;
			XPeekEvent(event_display, &next);
			if ((next.type == KeyPress) &&
				(next.xkey.keycode == event.xkey.keycode) &&
				(next.xmotion.time == event.xmotion.time))
			{
				XNextEvent(event_display, &next);
				return true;
			}
		}

		return false;
	}

    void GraphicsWindowX11::checkInterfaceEvents()
    {
        if (!isValid())
        {
            return;
        }

        Bool do_deinitialize = false;

        while(XPending(display))
        {
            XEvent event;
            XNextEvent(display, &event);

            switch(event.type)
            {
                case ClientMessage :
                {
                    if (Atom(event.xclient.data.l[0]) == wm_delete_window)
                    {
                        do_deinitialize = true;
                        interfaceQueue().closeWindow();
                    }
                    break;
                }

                default :
                    break;
            }
        }


        while (XPending(event_display))
		{
			XEvent event;
			XNextEvent(event_display, &event);

			switch (event.type)
			{
				case DestroyNotify:
				{
					do_deinitialize = true;
					interfaceQueue().closeWindow();
					break;
				}

				case MapNotify:
				case ConfigureNotify:
				{
					Window child_return;

					XWindowAttributes window_attributes;
					XGetWindowAttributes(display, window, &window_attributes);

					Int x = 0;
					Int y = 0;
					XTranslateCoordinates(display, window, parent_window, 0, 0, &x, &y, &child_return);

					GraphicsWindow::setWindowRectangle(x, y, window_attributes.width, window_attributes.height);
					interfaceQueue().windowResize(context_options->x, context_options->y, context_options->width, context_options->height);

					break;
				}

				case MotionNotify:
				{
					if (event.xmotion.same_screen)
					{
						Real x = Real(event.xmotion.x);
						Real y = Real(event.xmotion.y);
						interfaceQueue().mouseMotion(x, y);
					}
					break;
				}

				case ButtonPress:
				{
					switch (event.xbutton.button)
					{
						case Button1:
						{
							Real x = Real(event.xmotion.x);
							Real y = Real(event.xmotion.y);
							interfaceQueue().mouseButtonPress(x, y, InterfaceEvent::MouseLeftButton);
							break;
						}
						case Button2:
						{
							Real x = Real(event.xmotion.x);
							Real y = Real(event.xmotion.y);
							interfaceQueue().mouseButtonPress(x, y, InterfaceEvent::MouseMiddleButton);
							break;
						}
						case Button3:
						{
							Real x = Real(event.xmotion.x);
							Real y = Real(event.xmotion.y);
							interfaceQueue().mouseButtonPress(x, y, InterfaceEvent::MouseRightButton);
							break;
						}
						case Button4:
						{
							interfaceQueue().mouseScroll(InterfaceEvent::ScrollUp);
							break;
						}
						case Button5:
						{
							interfaceQueue().mouseScroll(InterfaceEvent::ScrollDown);
							break;
						}
						default:
							break;
					}

					break;
				}

				case ButtonRelease:
				{
					switch (event.xbutton.button)
					{
						case Button1:
						{
							Real x = Real(event.xmotion.x);
							Real y = Real(event.xmotion.y);
							interfaceQueue().mouseButtonRelease(x, y, InterfaceEvent::MouseLeftButton);
							break;
						}
						case Button2:
						{
							Real x = Real(event.xmotion.x);
							Real y = Real(event.xmotion.y);
							interfaceQueue().mouseButtonRelease(x, y, InterfaceEvent::MouseMiddleButton);
							break;
						}
						case Button3:
						{
							Real x = Real(event.xmotion.x);
							Real y = Real(event.xmotion.y);
							interfaceQueue().mouseButtonRelease(x, y, InterfaceEvent::MouseRightButton);
							break;
						}
						default:
							break;
					}

					break;
				}
				case UnmapNotify:
				{
					GraphicsWindow::setWindowRectangle(1, 1, 2, 2);
					interfaceQueue().windowResize(context_options->x, context_options->y, context_options->width, context_options->height);

					// dont break, so we can release keys on unmap
					// break;
				}
				case FocusOut:
				{
					modifier_state = modifierMask();

					// release normal keys
					for (Uint key = 8; key < 256; key++)
					{
						if (!isModifierKey(key) && isKeyPressed(key))
						{
							setKeyReleased(key);
							interfaceQueue().keyRelease(remapKey(key));
						}
					}

					// release modifier keys
					for (Uint key = 8; key < 256; key++)
					{
						if (isModifierKey(key) && isKeyPressed(key))
						{
							setKeyReleased(key);
							interfaceQueue().keyRelease(remapKey(key));
						}
					}

					break;
				}

				case KeymapNotify:
				case MappingNotify:
				{
					scanNumLockMask();
				    break;
				}

				case FocusIn:
				{
					// discard key events on Focus
					XEvent clear_event;
					while (XCheckMaskEvent(event_display, KeyPressMask | KeyReleaseMask, &clear_event))
					{
						continue;
					}

					break;
				}

				case KeyPress:
				{
					//*rengine::logger << "key press : " << event.xkey.keycode << ' ' << event.xmotion.time << std::endl;

					modifier_state = event.xkey.state;
					setKeyPressed(event.xkey.keycode);
					interfaceQueue().keyPress(remapKey(event.xkey));
					break;
				}

				case KeyRelease:
				{
					//	*rengine::logger << "key up : " << event.xkey.keycode << ' ' << event.xmotion.time << std::endl;

					// prevent auto repeat
					if (repeatedPress(event))
					{
						continue;
					}

					modifier_state = event.xkey.state;
					setKeyReleased(event.xkey.keycode);
					interfaceQueue().keyRelease(remapKey(event.xkey));

					break;
				}

				default:
				{
					//*logger << "default " << event.type << std::endl;
					break;
				}
			}
		}

		if (do_deinitialize)
		{
			deinitialize();
		}
	}

    Bool GraphicsWindowX11::releaseContext()
    {
        if (isValid())
        {
            return (glXMakeCurrent(display, None, 0) == True);
        }
        return false;
    }

    void GraphicsWindowX11::swapBuffers()
    {
        if (isValid())
        {
            glXSwapBuffers(display, window);
        }
    }

    Bool GraphicsWindowX11::makeCurrent()
    {
        if (isValid())
        {
            return (glXMakeCurrent(display, window, glx_context) == True);
        }
        return false;
    }

    Bool GraphicsWindowX11::setWindowRectangle(Int const x, Int const y, Int const width, Int const height)
	{
		if (isValid())
		{
			GraphicsWindow::setWindowRectangle(x, y, width, height);
			XMoveResizeWindow(display, window, x, y, width, height);

			setX11WindowRectangle();

			XFlush(display);
			XSync(display, 0);

			interfaceQueue().windowResize(context_options->x, context_options->y, context_options->width, context_options->height);
			waitForWindowManager();

			return true;
		}

		return false;
	}

    void GraphicsWindowX11::setWindowDecoration(Bool const window_decoration)
    {
        GraphicsWindow::setWindowDecoration(window_decoration);

        XMapWindow(display, window);
        setX11WindowRectangle();

        WMHints wm_hints;

        Atom atom;
        if( (atom = XInternAtom(display, "_MOTIF_WM_HINTS", 0 )) != None )
        {
            wm_hints.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
            wm_hints.functions = 0;
            wm_hints.decorations = 0;
            wm_hints.input_mode = 0;
            wm_hints.status = 0;

            if (context_options->window_decoration)
            {
            	wm_hints.decorations |= (MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MENU | MWM_DECOR_MAXIMIZE | MWM_DECOR_MINIMIZE);
            	wm_hints.functions |= (MWM_FUNC_MOVE  | MWM_FUNC_CLOSE| MWM_FUNC_MAXIMIZE | MWM_FUNC_MINIMIZE);

                if (context_options->window_supports_resize)
                {
                   wm_hints.functions |= MWM_FUNC_RESIZE ;
                   wm_hints.decorations |= MWM_DECOR_RESIZEH;
                }
            }

            XChangeProperty(display, window, atom, atom, 32, PropModeReplace, (Uchar*)&wm_hints, 5);
        }

        XSizeHints* size_hints = XAllocSizeHints();
		long int supplied_return = 0;
		XGetWMNormalHints(display, window, size_hints, &supplied_return);

		if (!context_options->window_supports_resize)
		{
			size_hints->flags |= PMinSize | PMaxSize;
			size_hints->min_height = context_options->height;
			size_hints->max_height = context_options->height;
			size_hints->min_width = context_options->width;
			size_hints->max_width = context_options->width;
		}
		else
		{
			size_hints->flags &= ~(PMinSize | PMaxSize);
			size_hints->min_height = 0;
			size_hints->max_height = 0;
			size_hints->min_width = 0;
			size_hints->max_width = 0;
		}
		XSetWMNormalHints(display, window, size_hints);
		XFree(size_hints);

        XFlush(display);
        XSync(display, 0);

        waitForWindowManager();
	}

	void GraphicsWindowX11::grabFocus()
	{
		if (isValid())
		{
			XSetInputFocus(display, window, RevertToNone, CurrentTime);
			XFlush(display);
			XSync(display, 0);
		}
	}

	void GraphicsWindowX11::setWindowName(std::string const& window_name)
	{
		GraphicsWindow::setWindowName(window_name);
		if (isValid())
		{
			XStoreName(display, window, context_options->window_name.c_str());
			XSetIconName(display, window, context_options->window_name.c_str());

			XFlush(display);
			XSync(display, 0);
		}
	}

	void GraphicsWindowX11::setUseCursor(Bool use_cursor)
	{
		if (isValid())
		{
			if (use_cursor != context_options->use_cursor)
			{
				GraphicsWindow::setUseCursor(use_cursor);

				if (use_cursor)
				{
					XUndefineCursor(display, window);
				}
				else
				{
					Char data[] = { 0 };

					XColor color;
					Pixmap pix_map = XCreateBitmapFromData(display, window, data, 1, 1);

					if (pix_map == None)
					{
						*logger << "WindowingSystem : Unable to hide cursor : " << context_options->name() << std::endl;
					}

					Cursor cursor = XCreatePixmapCursor(display, pix_map, pix_map, &color, &color, 0, 0);
					XFreePixmap(display, pix_map);

					XDefineCursor(display, window, cursor);
				}

				XFlush(display);
				XSync(display, 0);
			}
		}
	}

	void GraphicsWindowX11::moveCursorTo(Real const x, Real const y)
	{
	    XWarpPointer(display, None, window, 0, 0, 0, 0, Int(x), Int(y));
	    interfaceQueue().setMousePosition(x, y);

	    XFlush(display);
	    XSync(display, 0);
	}

	void GraphicsWindowX11::close()
	{
		GraphicsWindow::close();
		deinitialize();
	}
} // namespace rengine

extern "C" void initializeGraphicsWindowX11(void)
{
	rengine::logger = 0;
	rengine::logger = &std::cout;

	rengine::GraphicsWindow::setWindowingSystemInterface( rengine::getWindowingSystemInterface() );
}

