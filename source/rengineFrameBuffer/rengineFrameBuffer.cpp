#include <rengine/util/Bootstrap.h>

float buffer_width = 0.0f;
float buffer_height = 0.0f;

Vector3D const initial_position(0.0f, 0.0f, 2.0f);

class MainScene : public Scene, public InterfaceEventHandler
{
public:
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


		SharedPointer<Capsule> capsule = new Capsule();
		capsule->setSlices(25);
		capsule->setStacks(15);
		capsule->computeGeometry();

		shape = capsule;

		states.setState(new Depth);
		states.setState(new CullFace);
		states.setState(new BlendFunction());

		program = CoreEngine::instance()->resourceManager().load<Program>("data/shaders/BasicEffect.eff");
		states.setProgram(program);

		CoreEngine::instance()->renderEngine().apply(states);
		CoreEngine::instance()->renderEngine().reportDrawStates();


		mrt_states = states;


		mrt_program = CoreEngine::instance()->resourceManager().load<Program>("data/shaders/MultiRenderTarget.eff");
		mrt_states.setProgram(mrt_program);

		//
		// RenderToTexture
		//
		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
		float aspect = width / height;

		buffer_width = 400.0f;
		buffer_height = buffer_width / aspect;


		frame_buffer = new FrameBuffer(buffer_width, buffer_height);

		//	SharedPointer<RenderBuffer> depth_stencil_buffer = new RenderBuffer(DrawResource::Depth24Stencil8);
		//	frame_buffer->attach(depth_stencil_buffer);

		//	SharedPointer<Texture2D> depth_buffer = new Texture2D(DrawResource::DepthComponent24);
		//	depth_buffer->setFilter(Texture2D::Nearest, Texture2D::Nearest);
		//	frame_buffer->attach(depth_buffer);

		SharedPointer<Texture2D> depth_stencil_buffer = new Texture2D(DrawResource::DepthComponent24); //Texture2D(DrawResource::Depth24Stencil8);
		depth_stencil_buffer->setFilter(Texture2D::Nearest, Texture2D::Nearest);
		frame_buffer->attach(depth_stencil_buffer);

		SharedPointer<Texture2D> color_buffer = new Texture2D(DrawResource::Rgba8);
		color_buffer->setFilter(Texture2D::Nearest, Texture2D::Nearest);
		frame_buffer->attach(color_buffer);


		//
		// MultiSample FrameBuffer Test
		//
		multisample_frame_buffer = new FrameBuffer(buffer_width, buffer_height, 8);

		SharedPointer<RenderBuffer> multisample_depth_buffer = new RenderBuffer(DrawResource::DepthComponent24);
		multisample_frame_buffer->attach(multisample_depth_buffer);
		SharedPointer<RenderBuffer> multisample_color_buffer = new RenderBuffer(DrawResource::Rgba8);
		multisample_frame_buffer->attach(multisample_color_buffer);


		non_multisample_frame_buffer = new FrameBuffer(buffer_width, buffer_height);

		SharedPointer<Texture2D> non_multisample_color_buffer = new Texture2D(DrawResource::Rgba8);
		non_multisample_color_buffer->setFilter(Texture2D::Nearest, Texture2D::Nearest);
		non_multisample_frame_buffer->attach(non_multisample_color_buffer);

		// make sure it is initialized before the blitting operation, (it will never be binded)
		CoreEngine::instance()->renderEngine().bind(*non_multisample_frame_buffer);
		CoreEngine::instance()->renderEngine().unbind(*non_multisample_frame_buffer);


		//
		// Multi-Render Target
		//
		mrt_frame_buffer = new FrameBuffer(buffer_width, buffer_height, 16);

		SharedPointer<Texture2D> mrt_diffuse_buffer = new Texture2D(DrawResource::Rgba8);
		mrt_frame_buffer->attach(mrt_diffuse_buffer);

		SharedPointer<Texture2D> mrt_specular_buffer = new Texture2D(DrawResource::Rgba8);
		mrt_frame_buffer->attach(mrt_specular_buffer);


		//
		// Debug
		//
		SharedPointer<Program> decal_program = CoreEngine::instance()->resourceManager().load<Program>("data/shaders/core/DecalTexture.eff");

		debug_states.setCapability(DrawStates::DepthTest, DrawStates::Off);
		debug_states.setCapability(DrawStates::Blend, DrawStates::Off);
		debug_states.setCapability(DrawStates::CullFace, DrawStates::Off);
		debug_states.setProgram(decal_program);

		float y_index = 0.0f;
		float x_index = 0.0f;

		color_buffer_quad = new Quadrilateral();
		color_buffer_quad->setCornersVertex(Vector3D(x_index * buffer_width, y_index * buffer_height, 0.0f),
			Vector3D((x_index + 1.0f) * buffer_width, (y_index + 1.0f) * buffer_height, 0.0f));
		color_buffer_quad->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
		color_buffer_quad->states()->setTexture(color_buffer);

		x_index += 1.0f;

		depth_buffer_quad = new Quadrilateral();
		depth_buffer_quad->setCornersVertex(Vector3D(x_index * buffer_width, y_index * buffer_height, 0.0f),
			Vector3D((x_index + 1.0f) * buffer_width, (y_index + 1.0f) * buffer_height, 0.0f));
		depth_buffer_quad->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
		depth_buffer_quad->states()->setTexture(depth_stencil_buffer);



		y_index += 1.0f;
		x_index = 0.0f;

		multisample_color_buffer_quad = new Quadrilateral();
		multisample_color_buffer_quad->setCornersVertex(Vector3D(x_index * buffer_width, y_index * buffer_height, 0.0f),
			Vector3D((x_index + 1.0f) * buffer_width, (y_index + 1.0f) * buffer_height, 0.0f));
		multisample_color_buffer_quad->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
		multisample_color_buffer_quad->states()->setTexture(non_multisample_color_buffer);



		y_index += 1.0f;
		x_index = 0.0f;

		mrt_diffuse_buffer_quad = new Quadrilateral();
		mrt_diffuse_buffer_quad->setCornersVertex(Vector3D(x_index * buffer_width, y_index * buffer_height, 0.0f),
			Vector3D((x_index + 1.0f) * buffer_width, (y_index + 1.0f) * buffer_height, 0.0f));
		mrt_diffuse_buffer_quad->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
		mrt_diffuse_buffer_quad->states()->setTexture(mrt_diffuse_buffer);

		x_index += 1.0f;

		mrt_specular_buffer_quad = new Quadrilateral();
		mrt_specular_buffer_quad->setCornersVertex(Vector3D(x_index * buffer_width, y_index * buffer_height, 0.0f),
			Vector3D((x_index + 1.0f) * buffer_width, (y_index + 1.0f) * buffer_height, 0.0f));
		mrt_specular_buffer_quad->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
		mrt_specular_buffer_quad->states()->setTexture(mrt_specular_buffer);
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
		float const z_near = 0.5f;
		float const z_far = 10.0f;

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

		mrt_program->uniform("model_view_projection").set(Matrix44(), 0);
		mrt_program->uniform("model_view_projection").set(camera->viewMatrix(), 1);
		mrt_program->uniform("model_view_projection").set(camera->projectionMatrix(), 2);

		CoreEngine::instance()->renderEngine().pushDrawStates();
		CoreEngine::instance()->renderEngine().apply(states);

		//
		// Render Frame Buffer Object
		//
		CoreEngine::instance()->renderEngine().bind(*frame_buffer);
		CoreEngine::instance()->renderEngine().setViewport(0, 0, buffer_width, buffer_height);
		CoreEngine::instance()->renderEngine().clearBuffers();
		CoreEngine::instance()->renderEngine().draw( *shape );
		CoreEngine::instance()->renderEngine().unbind(*frame_buffer);

		//
		// MultiSample
		//
		CoreEngine::instance()->renderEngine().bind(*multisample_frame_buffer);
		CoreEngine::instance()->renderEngine().setViewport(0, 0, buffer_width, buffer_height);
		CoreEngine::instance()->renderEngine().clearBuffers();
		CoreEngine::instance()->renderEngine().draw( *shape );
		CoreEngine::instance()->renderEngine().unbind(*multisample_frame_buffer);

		CoreEngine::instance()->renderEngine().blit(*multisample_frame_buffer, *non_multisample_frame_buffer, RenderEngine::ColorBuffer);


		//
		// Multi-Render Target
		//

		CoreEngine::instance()->renderEngine().apply(mrt_states);
		CoreEngine::instance()->renderEngine().bind(*mrt_frame_buffer);
		CoreEngine::instance()->renderEngine().setViewport(0, 0, buffer_width, buffer_height);
		CoreEngine::instance()->renderEngine().clearBuffers();
		CoreEngine::instance()->renderEngine().draw( *shape );
		CoreEngine::instance()->renderEngine().unbind(*mrt_frame_buffer);


		//
		// Render on the main framebuffer
		//
		CoreEngine::instance()->renderEngine().apply(states);
		CoreEngine::instance()->renderEngine().setViewport(0, 0, Uint(width), Uint(height));
		CoreEngine::instance()->renderEngine().draw( *shape );
		CoreEngine::instance()->renderEngine().popDrawStates();



		//
		// debug buffers
		//
		debug_states.getProgram()->uniform("mvp").set( Matrix::ortho2D(0, width, 0, height) );

		CoreEngine::instance()->renderEngine().pushDrawStates();
		CoreEngine::instance()->renderEngine().apply(debug_states);
		CoreEngine::instance()->renderEngine().draw( *color_buffer_quad );
		CoreEngine::instance()->renderEngine().draw( *depth_buffer_quad );
		CoreEngine::instance()->renderEngine().draw( *multisample_color_buffer_quad );
		CoreEngine::instance()->renderEngine().draw( *mrt_diffuse_buffer_quad );
		CoreEngine::instance()->renderEngine().draw( *mrt_specular_buffer_quad );

		CoreEngine::instance()->renderEngine().popDrawStates();
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
		if (interface_event.eventType() == InterfaceEvent::EventKeyDown)
		{
			if (interface_event.key() == InterfaceEvent::KeySpace)
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
	SharedPointer<Mesh> shape;
	DrawStates states;
	DrawStates debug_states;

	SharedPointer<FpsCamera> camera;
	SharedPointer<Program> program;

	SharedPointer<FrameBuffer> frame_buffer;
	SharedPointer<Quadrilateral> color_buffer_quad;
	SharedPointer<Quadrilateral> depth_buffer_quad;

	SharedPointer<FrameBuffer> multisample_frame_buffer;
	SharedPointer<FrameBuffer> non_multisample_frame_buffer;
	SharedPointer<Quadrilateral> multisample_color_buffer_quad;

	DrawStates mrt_states;
	SharedPointer<Program> mrt_program;

	SharedPointer<FrameBuffer> mrt_frame_buffer;
	SharedPointer<Quadrilateral> mrt_diffuse_buffer_quad;
	SharedPointer<Quadrilateral> mrt_specular_buffer_quad;
};

RENGINE_BOOT();