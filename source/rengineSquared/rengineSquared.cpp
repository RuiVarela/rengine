#include <rengine/lang/debug/Debug.h>
#include <rengine/windowing/Windowing.h>
#include <rengine/math/Matrix.h>
#include <rengine/RenderEngine.h>
#include <GL/glew.h>
#include "imSquared.h"

#include <iostream>
#include <ctime>

using namespace std;
using namespace rengine;

static imSquared im_squared;

class EventHandler : public InterfaceEventHandler
{
public:
	EventHandler()
	{
		running = true;
	}

	virtual ~EventHandler()
	{
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		switch (interface_event.eventType())
		{
			case InterfaceEvent::EventPush:
			case InterfaceEvent::EventDrag:
			{
				m_points.clear();
				imSquared::Point point;
				point.x = interface_event.inputXnormalized();
				point.y = interface_event.inputYnormalized() * -1.0f;
				m_points.push_back(point);
			}
				break;

			case InterfaceEvent::EventRelease:
			{	
				m_points.clear();
			}
				break;


			case InterfaceEvent::EventKeyUp:
			{	
				if (interface_event.keySymbol() == InterfaceEvent::KeyEscape)
				{
					im_squared.reboot();
				}
			}
			break;


			case InterfaceEvent::EventWindowClose:
			{
				window->close();
				running = false;
			}
				break;

			default:
				break;

		};
	}

	bool isRunning() const
	{
		return running;
	}

	imSquared::Points const& points()
	{
		return m_points;
	}
private:
	bool running;
	imSquared::Points m_points;
};

int main(int argc, char* argv[])
{
	rengine::enableApplicationDebugger();

	initializeWindowingSystem();

	{
		unsigned int width = 0;
		unsigned int height = 0;
		GraphicsWindow::windowingSystemInterface()->screenResolution(GraphicsWindow::ScreenIdentifier(), width, height);


		SharedPointer<GraphicsWindow::ContextOptions> contex_options = new GraphicsWindow::ContextOptions();
		
		contex_options->width = 320;
		contex_options->height = 480;

		//contex_options->width = 768;
		//contex_options->height = 1024;

		contex_options->x = int((width - contex_options->width) / 2);
		contex_options->y = int((height - contex_options->height) / 2);
		contex_options->use_cursor = true;
		contex_options->window_decoration = true;
		contex_options->window_supports_resize = false;
		contex_options->window_name = "imSquared";
		contex_options->v_sync = true;

		EventHandler event_handler;
		SharedPointer<GraphicsWindow> window = GraphicsWindow::windowingSystemInterface()->createGraphicsWindow(contex_options);
		
		

		srand(time(NULL));

		imSquared::Configuration cfg;
		cfg.device = imSquared::Pc;
		cfg.screenWidth = contex_options->width;
		cfg.screenHeight = contex_options->height;
		cfg.columns = 10;
		cfg.rows = 13;
		cfg.level = "";
		cfg.texture_enabled = false;
		cfg.menus_enabled = true;


		im_squared.initialize(cfg);

		while (event_handler.isRunning() /*&& !im_squared.ended()*/)
		{
			im_squared.setTouches(event_handler.points());
			im_squared.update();
			im_squared.render();
			rengine::RenderEngine::checkErrors();

			window->swapBuffers();


			InterfaceQueue::Events events;
			window->checkInterfaceEvents();
			window->interfaceQueue().take(events);

			for (InterfaceQueue::Events::iterator iterator = events.begin(); iterator != events.end(); ++iterator)
				event_handler.operator ()(*iterator->get(), window.get());
		}
		
	}

	return 0;
}

