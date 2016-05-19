#include <rengine/util/Bootstrap.h>

class MainScene : public Scene, public InterfaceEventHandler
{
public:
	MainScene() {}
	virtual ~MainScene() {}

	virtual void init()
	{

		SharedPointer<FpsCamera> camera( new FpsCamera() );
		camera->setPosition(rengine::Vector3D(0.0f, 5.0f, 2.0f));
		CoreEngine::instance()->eventManager().addGuiEventHandler( camera );
		CoreEngine::instance()->setCamera( camera );
		camera->connect();



		std::string heightmap_filename = "";
		heightmap_filename = "data\\heightmaps\\08i__canyoncrusadedeluxe.png";
		//heightmap_filename = "data\\heightmaps\\simple_mountain.jpg";
		//heightmap_filename = "data\\heightmaps\\08i__hildebrand.png";
		//heightmap_filename = "data\\heightmaps\\level_4_512.jpg";
		//heightmap_filename = "data\\heightmaps\\level_4.jpg";
		//heightmap_filename = "data\\heightmaps\\3dtech.tga";

		heightmap_filename = convertFileNameToNativeStyle(heightmap_filename);
		texture = CoreEngine::instance()->resourceManager().load<Texture2D>(heightmap_filename);

		heightmap = new Heightmap();
		heightmap->setWidth(100.0f);
		heightmap->setDepth(100.0f);
		heightmap->setHeight(10.0f);
		heightmap->load(heightmap_filename);

		heightmap->states()->setCapability(DrawStates::DepthTest, DrawStates::On);
		heightmap->states()->setProgram( CoreEngine::instance()->resourceManager().load<Program>("data/shaders/Heightmap.eff"));

		texture_quadrilateral = new Quadrilateral();
		texture_quadrilateral->setCornersVertex(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(128.0f, 128.0f, 0.0f));
		texture_quadrilateral->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
		texture_quadrilateral->states()->setTexture(texture);
		texture_quadrilateral->states()->setProgram( CoreEngine::instance()->resourceManager().load<Program>("data/shaders/core/DecalColorMask.eff"));

		CoreEngine::instance()->system()("/set camera_move_factor 6");

	}
	virtual void shutdown(){}

	virtual void update(){ }

	virtual void render()
	{
		CoreEngine::instance()->renderEngine().clearBuffers();

		// this should be "on event handling"
		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;

		float const fovy = 45.0f;
		float const aspect = width / height;
		float const z_near = 0.01f;
		float const z_far = 1000.0f;

		CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);
		CoreEngine::instance()->camera()->setProjectionAsPerspective(fovy, aspect, z_near, z_far);

		CoreEngine::instance()->renderEngine().pushDrawStates();
		if (heightmap->states()->hasState(DrawStates::Program))
		{
			Program* program = dynamic_cast<ProgramUnit*>( heightmap->states()->getState(DrawStates::Program).first.get() )->get().get();

			program->uniform("model_view_projection").set(Matrix(), 0);
			program->uniform("model_view_projection").set(CoreEngine::instance()->camera()->viewMatrix(), 1);
			program->uniform("model_view_projection").set(CoreEngine::instance()->camera()->projectionMatrix(), 2);
		}
		CoreEngine::instance()->renderEngine().draw(*heightmap);
		CoreEngine::instance()->renderEngine().popDrawStates();




		CoreEngine::instance()->renderEngine().pushDrawStates();
		if (texture_quadrilateral->states()->hasState(DrawStates::Program))
		{
			Matrix projection = Matrix::ortho2D(0, Real(CoreEngine::instance()->mainWindow()->contextOptions().width),
				0, Real(CoreEngine::instance()->mainWindow()->contextOptions().height));

			Program* program = dynamic_cast<ProgramUnit*>( texture_quadrilateral->states()->getState(DrawStates::Program).first.get() )->get().get();
			program->uniform("mvp").set(projection);
		}
		CoreEngine::instance()->renderEngine().draw( *texture_quadrilateral );
		CoreEngine::instance()->renderEngine().popDrawStates();


		//	CoreEngine::instance()->renderEngine().setModelView( Matrix() );
		//	CoreEngine::instance()->renderEngine().setProjection( Matrix::ortho2D(0, width, 0, height) );
		//
		//	CoreEngine::instance()->renderEngine().renderBoundingBox(mesh_->boundingBox(), true);
		//	Plane plane(rengine::Vec3(0.5f, 0.5f, 0.0f), Vec3(0.0f, 2.0f, 0.0f));
		//	CoreEngine::instance()->renderEngine().renderInfinitePlane(plane, rengine::Vec3(0.0f, 3.0f, 0.0f), 5.5f);
		//	CoreEngine::instance()->renderEngine().renderAxis(1.0f);
		//	CoreEngine::instance()->writer().write(Vector2D(100.0f, 3.0f), "Vertex : " + lexical_cast<std::string> (my_model->meshes()[0]->numberOfVertex()) + " faces " +
		//			lexical_cast<std::string> (my_model->meshes()[0]->numberOfFaces()));
	}
	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window){ }
private:
	SharedPointer<Texture2D> texture;
	SharedPointer<Heightmap> heightmap;
	SharedPointer<Quadrilateral> texture_quadrilateral;
};

RENGINE_BOOT();