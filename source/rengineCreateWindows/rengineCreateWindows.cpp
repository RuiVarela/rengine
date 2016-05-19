#include <rengine/lang/debug/Debug.h>
#include <rengine/windowing/Windowing.h>
#include <rengine/math/Matrix.h>
#include <GL/glew.h>

#include <iostream>

using namespace std;
using namespace rengine;

unsigned int my_window_width = 640;
unsigned int my_window_height = 480;

float rotation_angle = 0.0f;

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

			case InterfaceEvent::EventKeyUp:
			{
				std::string name;
				InterfaceEvent::keySymbolName(interface_event.key(), name);

				std::cout << "EventKeyUp : "  << interface_event.key() << '[' << name << ']' << std::endl;

				if (interface_event.keySymbol() == InterfaceEvent::KeyEscape)
				{
					window->close();
					running = false;
				}
			}
				break;

			case InterfaceEvent::EventPush:
			{
				std::cout << "EventPush : "  << interface_event.mouseButton() << ' '
				<< interface_event.windowWidth() << ' ' << interface_event.windowHeight() << ' '
				<< interface_event.inputX() << ' ' << interface_event.inputY() << ' '
				<< interface_event.inputXnormalized() << ' ' << interface_event.inputYnormalized() << ' '
				<< std::endl;


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
private:
	bool running;
};

void initGLScene()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glViewport(0, 0, my_window_width, my_window_height);

	glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();

	float aspect_ratio = (GLfloat)my_window_width / (GLfloat)my_window_height;
	Matrix perspective = Matrix::perspective(45.0f, aspect_ratio, 0.1f, 100.0f);
	glLoadMatrixf(perspective.ptr());


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void drawGLScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -6.0f);
	glRotatef(rotation_angle, 0.0, 1.0, 0.0f);

	glBegin(GL_QUADS);									// Draw A Quad
		glColor3f(0.0f,1.0f,0.0f);						// Set The Color To Blue
		glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Right Of The Quad (Top)
		glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Left Of The Quad (Top)
		glVertex3f(-1.0f, 1.0f, 1.0f);					// Bottom Left Of The Quad (Top)
		glVertex3f( 1.0f, 1.0f, 1.0f);					// Bottom Right Of The Quad (Top)
		glColor3f(1.0f,0.5f,0.0f);						// Set The Color To Orange
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Top Right Of The Quad (Bottom)
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Top Left Of The Quad (Bottom)
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Left Of The Quad (Bottom)
		glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom Right Of The Quad (Bottom)
		glColor3f(1.0f,0.0f,0.0f);						// Set The Color To Red
		glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Front)
		glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Front)
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Left Of The Quad (Front)
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Right Of The Quad (Front)
		glColor3f(1.0f,1.0f,0.0f);						// Set The Color To Yellow
		glVertex3f( 1.0f,-1.0f,-1.0f);					// Top Right Of The Quad (Back)
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Top Left Of The Quad (Back)
		glVertex3f(-1.0f, 1.0f,-1.0f);					// Bottom Left Of The Quad (Back)
		glVertex3f( 1.0f, 1.0f,-1.0f);					// Bottom Right Of The Quad (Back)
		glColor3f(0.0f,0.0f,1.0f);						// Set The Color To Blue
		glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Left)
		glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Left Of The Quad (Left)
		glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Left Of The Quad (Left)
		glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Right Of The Quad (Left)
		glColor3f(1.0f,0.0f,1.0f);						// Set The Color To Violet
		glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Right Of The Quad (Right)
		glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Right)
		glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Left Of The Quad (Right)
		glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom Right Of The Quad (Right)
	glEnd();

	rotation_angle += 0.01f;
}

int main(int argc, char* argv[])
{
	rengine::enableApplicationDebugger();

	initializeWindowingSystem();

	{
		unsigned int number_of_screens = GraphicsWindow::windowingSystemInterface()->numberOfScreens();
		std::cout << "Number of Screens : " << number_of_screens << std::endl;

		for (unsigned int i = 0; i != number_of_screens; ++i)
		{
			GraphicsWindow::ScreenIdentifier screen_identifier(i);

			unsigned int width = 0;
			unsigned int height = 0;
			unsigned int bpp = GraphicsWindow::windowingSystemInterface()->screenColorDepth(screen_identifier);
			unsigned int refresh_rate = GraphicsWindow::windowingSystemInterface()->screenRefreshRate(screen_identifier);
			GraphicsWindow::windowingSystemInterface()->screenResolution(screen_identifier, width, height);

			std::cout << "Screen Id : " << screen_identifier.name() << std::endl;
			std::cout << "Screen [" << i << "] : " << width << "x" << height << ' ' << bpp << "bpp " << refresh_rate << "Hz" << std::endl;

			int position_x = 0;
			int position_y = 0;
			unsigned int position_width = 0;
			unsigned int position_height = 0;
			GraphicsWindow::windowingSystemInterface()->screenPosition(screen_identifier, position_x, position_y, position_width, position_height);
			std::cout << "Screen [" << i << "] : Rectangle " << position_x << ' ' << position_y << ' ' << position_width << ' ' << position_height << std::endl;
		}

		//GraphicsWindow::windowingSystemInterface()->setScreenResolution(GraphicsWindow::ScreenIdentifier(0), 800, 600);
		//GraphicsWindow::windowingSystemInterface()->setScreenResolution(GraphicsWindow::ScreenIdentifier(0), 1024, 768);

		unsigned int width = 0;
		unsigned int height = 0;
		GraphicsWindow::windowingSystemInterface()->screenResolution(GraphicsWindow::ScreenIdentifier(), width, height);


		SharedPointer<GraphicsWindow::ContextOptions> contex_options = new GraphicsWindow::ContextOptions();
		contex_options->width = my_window_width;
		contex_options->height = my_window_height;
		contex_options->x = int((width - contex_options->width) / 2);
		contex_options->y = int((height - contex_options->height) / 2);
		contex_options->use_cursor = true;
		contex_options->window_decoration = true;
		contex_options->window_supports_resize = false;
		contex_options->window_name = "My Simple Window";

		EventHandler event_handler;

		SharedPointer<GraphicsWindow> window = GraphicsWindow::windowingSystemInterface()->createGraphicsWindow(contex_options);
		initGLScene();

		while (event_handler.isRunning())
		{
			InterfaceQueue::Events events;

			window->checkInterfaceEvents();
			window->interfaceQueue().take(events);

			for (InterfaceQueue::Events::iterator iterator = events.begin(); iterator != events.end(); ++iterator)
			{
				event_handler.operator ()(*iterator->get(), window.get());
			}

			drawGLScene();
			window->swapBuffers();
		}
	}

	return 0;
}

