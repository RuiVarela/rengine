// __!!rengine_copyright!!__ //

#ifndef __RENGINE_COREENGINE_H__
#define __RENGINE_COREENGINE_H__

#include <rengine/windowing/Windowing.h>
#include <vector>

namespace rengine
{
	class EngineConfiguration;
	class Timer;
	class System;
	class RenderEngine;
	class EventManager;
	class OutputStream;
	class Console;
	class CharMap;
	class Scene;
	class Camera;
	class HudWriter;
	class ResourceManager;
	class StringTable;

	class CoreEngine
	{
	public:
		CoreEngine();
		~CoreEngine();

		static void create();
		static void destroy();
		static CoreEngine* instance();

		Timer& timer();
		Timer const& timer() const;

		System& system();
		System const& system() const;

		RenderEngine& renderEngine();
		RenderEngine const& renderEngine() const;

		EventManager const& eventManager() const;
		EventManager& eventManager();

		OutputStream& log();
		OutputStream const& log() const;

		Console& console();
		Console const& console() const;

		HudWriter& writer();
		HudWriter const& writer() const;

		ResourceManager& resourceManager();
		ResourceManager const& resourceManager() const;

		EngineConfiguration& configuration();
		EngineConfiguration const& configuration() const;

		StringTable& locationTable();
		StringTable const& locationTable() const;

		void setScene(SharedPointer<Scene> const& scene);
		Scene *const scene() const;

		void setCamera(SharedPointer<Camera> const& camera);
		Camera *const camera() const;

		void requestShutdown();
		Bool shutdownRequested() const;

		// current frame number
		Int64 frameNumber() const;
		// global time at the beginning of the frame
		Real64 frameGlobalTime() const;
		// frame delat time
		Real64 frameDeltaTime() const;

		//Main loop functions
		void handleEvents();
		void update();
		void render();
		void frame();
		void renderLoop();

		Bool isStarted() const;
		void startEngine(EngineConfiguration const& engine_configuration);

		// windowing
		typedef std::vector< SharedPointer<GraphicsWindow> > GraphicsWindows;

		GraphicsWindows& windows();
		GraphicsWindows const& windows() const;

		void reportWindowingInfo();
		void addWindow(SharedPointer<GraphicsWindow::ContextOptions> context_options);
		GraphicsWindow const* mainWindow() const;
		void setUpScreens();
		/**
		 * Releases last context and makes "context" current
		 */
		Bool makeCurrentContext(GraphicsWindow* context);

	private:
		struct Implementation;
		Implementation* implementation;

		CoreEngine(CoreEngine const&)
		{
		}
		static CoreEngine* engine_instance_;

		Bool engine_started_;
		Bool shutdown_requested_;
		Real64 last_frame_time_seconds_;
		Real64 frame_delta_seconds_;
	};
}// end of namespace

#endif // __RENGINE__COREENGINE_H__
