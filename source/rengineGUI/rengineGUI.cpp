#include <rengine/util/Bootstrap.h>

class MainScene : public Scene, public InterfaceEventHandler
{
public:
	MainScene() {}
	virtual ~MainScene() {}

	virtual void init()
	{
		DrawStates base_states;
		base_states.setState(new BlendFunction());
		CoreEngine::instance()->renderEngine().apply(base_states);
	}

	virtual void shutdown()
	{
	}

	virtual void update()
	{
	}

	virtual void render()
	{
		CoreEngine::instance()->renderEngine().clearBuffers();

		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;

		raw::render()->pushDrawStates();


		if (_gui->states()->hasState(DrawStates::Program))
		{
			Matrix projection = Matrix::ortho2D(0, width, 0, height);
			Program* program = dynamic_cast<ProgramUnit*>( _gui->states()->getState(DrawStates::Program).first.get() )->get().get();
			program->uniform("mvp").set(projection);
		}


		raw::render()->draw( *_gui);

		raw::render()->popDrawStates();
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		if (interface_event.eventType() == InterfaceEvent::EventKeyDown)
		{
			if (interface_event.key() == InterfaceEvent::KeySpace)
			{

			}
		}
	}

private:
	SharedPointer<InterfaceComponent> _gui;
};

RENGINE_BOOT();