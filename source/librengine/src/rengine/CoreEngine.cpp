// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>

#include <rengine/windowing/Windowing.h>
#include <rengine/outputstream/OutputStream.h>
#include <rengine/outputstream/Console.h>
#include <rengine/system/System.h>
#include <rengine/image/Image.h>
#include <rengine/string/String.h>
#include <rengine/lang/exception/BaseExceptions.h>
#include <rengine/RenderEngine.h>
#include <rengine/time/Timer.h>
#include <rengine/Scene.h>
#include <rengine/camera/Camera.h>
#include <rengine/event/EventManager.h>
#include <rengine/text/HudWriter.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/util/StringTable.h>
#include <rengine/Configuration.h>
#include <rengine/thread/Thread.h>

#include <rengine/state/BaseStates.h>

//soft openal
extern "C" 
{
	void alc_init(void);
	void alc_deinit(void);
}

namespace rengine
{
	CoreEngine* CoreEngine::engine_instance_ = 0;

	class CoreEngineInterfaceTimer : public InterfaceTimer
	{
	public:
		virtual ~CoreEngineInterfaceTimer()
		{
		}

		virtual Real64 elapsedTime() const
		{
			return CoreEngine::instance()->timer().elapsedTime();
		}
	};

	struct CoreEngine::Implementation
	{
		Implementation()
		{
			alc_init();
		}

		~Implementation()
		{
			alc_deinit();
		}


		Timer timer_;
		System system_;
		OutputStream output_stream_;
		Console console_;
		RenderEngine render_engine_;
		EventManager event_manager_;
		SharedPointer<Scene> scene_;
		SharedPointer<Camera> camera_;
		GraphicsWindows windows_;
		GraphicsWindow* current_context_;
		EngineConfiguration engine_configuration_;
		SharedPointer<HudWriter> writer_;
		ResourceManager resource_manager_;

		DrawStates output_draw_states;
	};

	CoreEngine::CoreEngine() :
		implementation(new Implementation()),
		engine_started_(false),
		shutdown_requested_(false)
	{
		implementation->current_context_ = 0;
		implementation->camera_ = new Camera();

		log().registerPrinter(new CoutStringPrinter());
	}

	CoreEngine::~CoreEngine()
	{
		resourceManager().clearLoaders();

		setScene(0);
		setCamera(0);
		implementation->console_.shutdown();
		implementation->event_manager_.shutdown();
		implementation->writer_ = 0;

		renderEngine().shutdown();
		windows().clear();

		delete(implementation);
	}

	void CoreEngine::create()
	{
		engine_instance_ = new CoreEngine();
	}

	void CoreEngine::destroy()
	{
		delete(engine_instance_);
	}

	void CoreEngine::setScene(SharedPointer<Scene> const& scene)
	{
		if (implementation->scene_)
		{
			implementation->scene_->shutdown();
		}

		implementation->scene_ = scene;

		if (implementation->scene_)
		{
			implementation->scene_->init();
		}
	}

	void CoreEngine::startEngine(EngineConfiguration const& engine_configuration)
	{
		implementation->engine_configuration_ = engine_configuration;


		initializeWindowingSystem();
		setUpScreens();

		//configure command handlers and register default loaders
		resourceManager().configure();

		renderEngine().init();

		Text::setDefaultEffectFile(engine_configuration.text_effect);
		implementation->writer_ = new HudWriter();

		console().initialize(engine_configuration.console_number_of_lines,
							 engine_configuration.console_background,
							 engine_configuration.console_floating_background,
							 engine_configuration.console_effect);

		eventManager().configure();

		console().registerSystemFeed(&system());

		log().clearPrinters();
		log().registerPrinter(new LoggerStringPrinter());
		log().registerPrinter(new ConsolePrinter(&console()));

		shutdown_requested_ = false;
		timer().start();

		reportWindowingInfo();

		log() << "Graphics report :" << std::endl;
		log() << "Renderer : " << renderEngine().renderer() << std::endl;
		log() << "Vendor : " << renderEngine().vendor() << std::endl;
		log() << "Version : " << renderEngine().version() << std::endl;
		log() << "Shading Language Version : " << renderEngine().shadingLanguageVersion() << std::endl;


		log() << "Texture Rectangle support : " << (renderEngine().supportsTextureRectangle() ? "Yes" : "no") << std::endl;
		log() << "Texture Non Power Of Two : " << (renderEngine().supportsNonPowerOfTwoTextures() ? "Yes" : "No") << std::endl;
		log() << "Texture Maximum size : " << renderEngine().maximumTextureSizeSupported() << std::endl;
		log() << "OpenGl 2.1 Limitation : " << (renderEngine().limitedToOpenGL21() ? "Yes" : "No") << std::endl;

		
	
		implementation->output_draw_states.setCapability(DrawStates::CullFace, DrawStates::Off);
		implementation->output_draw_states.setCapability(DrawStates::DepthTest, DrawStates::Off);


		last_frame_time_seconds_ = 0.0;
		frame_delta_seconds_ = 0.0;
	}

	void CoreEngine::setUpScreens()
	{
		//reportWindowingInfo();
		// create one screen

		GraphicsWindow::WindowingSystemInterface* windowing_interface = GraphicsWindow::windowingSystemInterface();
		if (!windowing_interface)
		{
			log() << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
			return;
		}

		GraphicsWindow::ScreenIdentifier screen_identifier(0);
		Uint width, height;
		windowing_interface->screenResolution(screen_identifier, width, height);

		Uint window_width = configuration().window_width;
		Uint window_height = configuration().window_height;

		if (configuration().fullscreen)
		{
			window_width = width;
			window_height = height;
		}

		SharedPointer<GraphicsWindow::ContextOptions> context_options( new GraphicsWindow::ContextOptions );
		context_options->red = 8;
		context_options->green = 8;
		context_options->blue = 8;
		context_options->alpha = 8;

		context_options->width = window_width;
		context_options->height = window_height;

		context_options->x = (width - context_options->width) / 2;
		context_options->y = (height - context_options->height) / 2;

		context_options->window_supports_resize = false;
		context_options->window_name = "raw engine.";
		context_options->window_decoration = !configuration().fullscreen;
		context_options->v_sync = configuration().vsync;

		context_options->stencil = 0;
		context_options->depth = 24;

		context_options->double_buffer = true;


		addWindow(context_options);
	}

	void CoreEngine::reportWindowingInfo()
	{
		log() << "Windowing Info report: " << std::endl;

		GraphicsWindow::WindowingSystemInterface* windowing_interface = GraphicsWindow::windowingSystemInterface();
		if (!windowing_interface)
		{
			log() << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
			return;
		}

		Uint number_of_screens = windowing_interface->numberOfScreens();
		log() << "Number of screens detected : " << number_of_screens << std::endl;

		for (Uint current_screen_number = 0; current_screen_number != number_of_screens; ++current_screen_number)
		{
			GraphicsWindow::ScreenIdentifier screen_identifier(current_screen_number);
			Uint width, height;

			windowing_interface->screenResolution(screen_identifier, width, height);
			log() << "Screen [" << current_screen_number << "] " << width << "x" << height << std::endl;
		}
	}

	void CoreEngine::addWindow(SharedPointer<GraphicsWindow::ContextOptions> context_options)
	{
		GraphicsWindow::WindowingSystemInterface* windowing_interface = GraphicsWindow::windowingSystemInterface();

		if (!windowing_interface)
		{
			log() << "Error, no WindowSystemInterface available, cannot create windows." << std::endl;
			return;
		}

		GraphicsWindow* window( GraphicsWindow::createGraphicsWindow(context_options) );
		if (window)
		{
			SharedPointer<InterfaceTimer> interface_timer = new CoreEngineInterfaceTimer();
			window->interfaceQueue().setTimer(interface_timer);
			windows().push_back( window );
		}
	}
	GraphicsWindow const* CoreEngine::mainWindow() const
	{
		if (windows().empty())
		{
			return 0;
		}

		return windows().front().get();
	}

	void CoreEngine::frame()
	{
		handleEvents();
		update();

		//if (!skip_render_)
		{
			render();
		}
		//else
		// keep it stable for input
		//if (timer().elapsedSinceFrameStart() < (1.0 / 1000.0))
		//{
		//	Thread::microSleep(1 * 1000);
		//}


		last_frame_time_seconds_ = timer().advanceOperation();
		frame_delta_seconds_ = timer().operationTime();
	}

	void CoreEngine::renderLoop()
	{
		while (!shutdownRequested())
		{
			frame();
		}

		setScene(0);
	}

	Bool CoreEngine::makeCurrentContext(GraphicsWindow* context)
	{
		if (implementation->current_context_ != context)
		{
			Bool release_state = (implementation->current_context_ && implementation->current_context_->releaseContext());
			Bool aquire_state = (context && context->makeCurrent());

			if (aquire_state)
			{
				implementation->current_context_ = context;
			}

			return (release_state && aquire_state);
		}

		return true;
	}

	void CoreEngine::handleEvents()
	{
		// loop each window

		for (Uint current_window = 0; current_window != windows().size(); ++current_window)
		{
			InterfaceQueue::Events events;
			windows()[current_window]->checkInterfaceEvents();
			windows()[current_window]->interfaceQueue().take(events);

			//handle all events
			for (InterfaceQueue::Events::iterator events_iterator = events.begin();
					events_iterator != events.end();
					++events_iterator)
			{
				eventManager()(*events_iterator->get(), windows()[current_window].get());
			}
		}
	}

	void CoreEngine::update()
	{
		if (implementation->camera_)
		{
			camera()->update();
		}

		if (implementation->scene_)
		{
			scene()->update();
		}
	}

	void CoreEngine::render()
	{
		renderEngine().preFrame();

		// render scene
		if (implementation->scene_)
		{
			scene()->render();
		}

		renderEngine().pushDrawStates();
		renderEngine().apply(implementation->output_draw_states);

		// render console
		if ( console().state() > Console::closed )
		{
			renderEngine().draw( console() );
		}


		writer().clear();
		writer().write(Vector2D(0.0f, 0.0f), " frame : " + lexical_cast<std::string>( frameNumber() ) + " "
											 " fps : " + lexical_cast<std::string>( timer().getFps() ) +
				                         //    " frame time: " + lexical_cast<std::string>( frameDeltaTime()) +
				                             " global time: " + lexical_cast<std::string>( frameGlobalTime() ) );

		renderEngine().draw( writer() );

		renderEngine().popDrawStates();

		renderEngine().postFrame();
		renderEngine().checkErrors("End Of Frame : ");
	}

	CoreEngine* CoreEngine::instance()
	{
		return engine_instance_;
	}

	Bool CoreEngine::isStarted() const
	{
		return engine_started_;
	}

	Timer& CoreEngine::timer()
	{
		return implementation->timer_;
	}

	Timer const& CoreEngine::timer() const
	{
		return implementation->timer_;
	}

	System& CoreEngine::system()
	{
		return implementation->system_;
	}

	System const& CoreEngine::system() const
	{
		return implementation->system_;
	}

	RenderEngine& CoreEngine::renderEngine()
	{
		return implementation->render_engine_;
	}

	RenderEngine const& CoreEngine::renderEngine() const
	{
		return implementation->render_engine_;
	}

	Console& CoreEngine::console()
	{
		return implementation->console_;
	}

	Console const& CoreEngine::console() const
	{
		return implementation->console_;
	}

	OutputStream& CoreEngine::log()
	{
		return implementation->output_stream_;
	}

	OutputStream const& CoreEngine::log() const
	{
		return implementation->output_stream_;
	}

	HudWriter& CoreEngine::writer()
	{
		return *implementation->writer_.get();
	}

	HudWriter const& CoreEngine::writer() const
	{
		return *implementation->writer_.get();
	}

	ResourceManager& CoreEngine::resourceManager()
	{
		return implementation->resource_manager_;
	}

	ResourceManager const& CoreEngine::resourceManager() const
	{
		return implementation->resource_manager_;
	}

	EngineConfiguration& CoreEngine::configuration()
	{
		return implementation->engine_configuration_;
	}

	EngineConfiguration const& CoreEngine::configuration() const
	{
		return implementation->engine_configuration_;
	}

	CoreEngine::GraphicsWindows& CoreEngine::windows()
	{
		return implementation->windows_;
	}

	CoreEngine::GraphicsWindows const& CoreEngine::windows() const
	{
		return implementation->windows_;
	}

	Scene *const CoreEngine::scene() const
	{
		return implementation->scene_.get();
	}

	Int64 CoreEngine::frameNumber() const
	{
		return implementation->timer_.operation();
	}

	Real64 CoreEngine::frameGlobalTime() const
	{
		return last_frame_time_seconds_;
	}

	Real64 CoreEngine::frameDeltaTime() const
	{
		return frame_delta_seconds_;
	}

	void CoreEngine::setCamera(SharedPointer<Camera> const& camera)
	{
		implementation->camera_ = camera;
	}

	Camera *const CoreEngine::camera() const
	{
		return implementation->camera_.get();
	}

	EventManager const& CoreEngine::eventManager() const
	{
		return implementation->event_manager_;
	}

	EventManager& CoreEngine::eventManager()
	{
		return implementation->event_manager_;
	}

	void CoreEngine::requestShutdown()
	{
		shutdown_requested_ = true;
	}

	Bool CoreEngine::shutdownRequested() const
	{
		return shutdown_requested_;
	}

	StringTable& CoreEngine::locationTable()
	{
		return implementation->engine_configuration_.location_table;
	}

	StringTable const& CoreEngine::locationTable() const
	{
		return implementation->engine_configuration_.location_table;
	}

} // namespace rengine

