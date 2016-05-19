#include <rengine/util/Bootstrap.h>

Vector3D const initial_position(0.0f, 0.0f, 2.0f);

class MainScene : public Scene, public InterfaceEventHandler
{
public:
	typedef std::vector< SharedPointer<Mesh> > MeshVector;
	MainScene() {}
	virtual ~MainScene() {}

	virtual void init()
	{
		camera = new FpsCamera();
		camera->setPosition(initial_position);
		CoreEngine::instance()->eventManager().addGuiEventHandler(camera);
		CoreEngine::instance()->setCamera(camera);
		camera->connect();
		CoreEngine::instance()->system()("/set camera_mouse_sensitivity 0.5");


		SharedPointer<Quadrilateral> quadrilateral = new Quadrilateral(Vector3D(-0.5f, -0.5f, 0.0f), Vector3D(0.5f, 0.5f, 0.0f));
		SharedPointer<Box> box = new Box();
		SharedPointer<Sphere> sphere = new Sphere(1.0f, 10, 10);

		SharedPointer<Cylinder> cylinder = new Cylinder();
		cylinder->setCenter(Vector3D(0.0f, 0.0f, 0.0f));
		cylinder->setBottomRadius(0.2f);
		cylinder->setTopRadius(0.2f);
		cylinder->computeGeometry();

		SharedPointer<Cone> cone = new Cone();
		cone->setHeight(1.0f);
		cone->setSlices(18);
		cone->setStacks(10);
		cone->computeGeometry();

		SharedPointer<Torus> torus = new Torus();
		torus->setRings(50);
		torus->setSides(20);
		torus->computeGeometry();

		SharedPointer<Capsule> capsule = new Capsule();
		capsule->setSlices(25);
		capsule->setStacks(15);
		capsule->computeGeometry();

		shapes.push_back(quadrilateral);
		shapes.push_back(box);
		shapes.push_back(sphere);
		shapes.push_back(cylinder);
		shapes.push_back(cone);
		shapes.push_back(torus);
		shapes.push_back(capsule);

		current_shape = 6;

		shapes[0]->setDrawMode(Drawable::StaticDraw);
		shapes[1]->setDrawMode(Drawable::StaticDraw);
		shapes[2]->setDrawMode(Drawable::StaticDraw);
		shapes[3]->setDrawMode(Drawable::StaticDraw);
		shapes[4]->setDrawMode(Drawable::StaticDraw);
		shapes[5]->setDrawMode(Drawable::StaticDraw);
		shapes[6]->setDrawMode(Drawable::StaticDraw);


		states.setState(new Depth);
		states.setState(new CullFace);
		states.setState(new BlendFunction());
		//states.setState(new PolygonMode(PolygonMode::FrontAndBack, PolygonMode::Line));

		program = CoreEngine::instance()->resourceManager().load<Program>("data/shaders/BasicEffect.eff");
		//program->uniform("light_direction").set( Vector4D(0.5f, -0.2f, 0.0f, 0.0f) );

		states.setProgram(program);

		CoreEngine::instance()->renderEngine().apply(states);

		//	CoreEngine::instance()->renderEngine().reportProgram(*program);
		//	CoreEngine::instance()->renderEngine().reportDrawStates();
	}

	virtual void shutdown()
	{
		program = 0;
	}

	virtual void update()
	{
		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;

		float const fovy = 45.0f;
		float const aspect = width / height;
		float const z_near = 0.01f;
		float const z_far = 1000.0f;

		CoreEngine::instance()->camera()->setProjectionAsPerspective(fovy, aspect, z_near, z_far);
		//	CoreEngine::instance()->camera()->setProjectionAsOrtho(0.0f, width, 0.0f, height, -100.0f, 100.0f);

	}
	virtual void render()
	{
		CoreEngine::instance()->renderEngine().clearBuffers();

		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
		CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);


		program->uniform("model_view_projection").set(Matrix44(), 0);
		program->uniform("model_view_projection").set(camera->viewMatrix(), 1);
		program->uniform("model_view_projection").set(camera->projectionMatrix(), 2);

		CoreEngine::instance()->renderEngine().pushDrawStates();

		CoreEngine::instance()->renderEngine().apply(states);

		CoreEngine::instance()->renderEngine().draw( *shapes[current_shape] );

		CoreEngine::instance()->renderEngine().popDrawStates();
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		if (interface_event.eventType() == InterfaceEvent::EventKeyDown)
		{
			if (interface_event.key() == InterfaceEvent::KeyRight)
			{
				unsigned int shapes_size = (unsigned int) (shapes.size());
				current_shape = (current_shape + 1) % shapes_size;
			}
			else if (interface_event.key() == InterfaceEvent::KeyLeft)
			{
				unsigned int shapes_size = (unsigned int) (shapes.size());
				current_shape = (current_shape + shapes_size - 1) % shapes_size;
			}
			else if (interface_event.key() == InterfaceEvent::KeySpace)
			{
				camera->setPosition(initial_position);
				camera->setPitch(0.0f);
				camera->setYaw(0.0f);
				camera->setRoll(0.0f);
			}
			else if (interface_event.key() == 'p')
			{
				togglePolygonMode();
			}
		}
	}

	void togglePolygonMode()
	{
		if (!states.hasState(DrawStates::PolygonMode))
		{
			states.setState(new PolygonMode(PolygonMode::FrontAndBack, PolygonMode::Fill));
		}

		PolygonMode* polygon_mode = dynamic_cast<PolygonMode*> (states.getState(DrawStates::PolygonMode).first.get());
		switch (polygon_mode->getMode(PolygonMode::FrontAndBack))
		{
		case PolygonMode::Fill:
			{
				polygon_mode->setMode(PolygonMode::FrontAndBack, PolygonMode::Line);
				break;
			}
		case PolygonMode::Line:
			{
				polygon_mode->setMode(PolygonMode::FrontAndBack, PolygonMode::Point);
				break;
			}
		case PolygonMode::Point:
			{
				polygon_mode->setMode(PolygonMode::FrontAndBack, PolygonMode::Fill);
				break;
			}
		}
	}
private:
	MeshVector shapes;
	unsigned int current_shape;
	DrawStates states;

	SharedPointer<FpsCamera> camera;
	SharedPointer<Program> program;
};

RENGINE_BOOT();
