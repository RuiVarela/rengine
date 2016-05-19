#include <rengine/util/Bootstrap.h>

class MainScene : public Scene, public InterfaceEventHandler
{
public:
	typedef std::vector< SharedPointer<Mesh> > MeshVector;
	MainScene() {}
	virtual ~MainScene() {}

	virtual void init()
	{
	//std::string const image_file = "data/images/checkers.png";
	//std::string const image_file = "data/images/wall/brick05.jpg";
	//std::string const image_file = "data/images/world.jpg";
	std::string const image_file = "data/images/lena_std.bmp";
	//std::string const image_file = "data/images/pic5.png";


	float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
	float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
	CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);


	original_image = CoreEngine::instance()->resourceManager().load<Texture2D>(image_file);
	original_image->setFilter(Texture2D::Nearest, Texture2D::Nearest);

	float top_right_x = float(original_image->getWidth());
	float top_right_y = float(CoreEngine::instance()->mainWindow()->contextOptions().height);
	float x = 0.0f;
	float y = top_right_y - float(original_image->getHeight());
	processed_image = new Texture2D(DrawResource::Rgb8);

	SharedPointer<Program> decal_program = CoreEngine::instance()->resourceManager().load<Program>("data/shaders/core/DecalTexture.eff");
	original_quadrilateral = new Quadrilateral();
	original_quadrilateral->setCornersVertex(Vector3D(x, y, 0.0f), Vector3D(top_right_x, top_right_y, 0.0f));
	original_quadrilateral->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
	original_quadrilateral->states()->setProgram(decal_program);
	original_quadrilateral->states()->setTexture(original_image);


	x += float(original_image->getWidth());
	top_right_x += float(original_image->getWidth());

	output_quadrilateral = new Quadrilateral();
	output_quadrilateral->setCornersVertex(Vector3D(x, y, 0.0f), Vector3D(top_right_x, top_right_y, 0.0f));
	output_quadrilateral->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));
	output_quadrilateral->states()->setProgram(decal_program);
	output_quadrilateral->states()->setTexture(processed_image);

	SharedPointer<KernelImageProcessor> kernel_image_processor;
	SharedPointer<SeparableKernelImageProcessor> separable_kernel_image_processor;
	SharedPointer<Kernel1D> kernel_1d;

	processing_chain = new ProcessingChain();
	//processing_chain->addProcessingStage(new CopyImageProcessor());
	processing_chain->addProcessingStage(new GrayscaleImageProcessor());
	//processing_chain->addProcessingStage(new ThresholdImageProcessor());

//	processing_chain->addProcessingStage(new KernelImageProcessor(Kernel1D::gaussian(5, Kernel::Horizontal, 0.3)));
//	processing_chain->addProcessingStage(new KernelImageProcessor(Kernel1D::gaussian(5, Kernel::Vertical, 0.3)));


//	processing_chain->addProcessingStage(new KernelImageProcessor( Kernel2D::gaussian(9) ));

//	processing_chain->addProcessingStage(new KernelImageProcessor( Kernel2D::laplacian(true) ));

	processing_chain->addProcessingStage(new SeparableKernelImageProcessor(Kernel2D::sobel(3, Kernel2D::Vertical),
																		   Kernel2D::sobel(3, Kernel2D::Horizontal)));
//
//	processing_chain->addProcessingStage(new SeparableKernelImageProcessor(Kernel2D::prewitt(Kernel2D::Vertical),
//																		   Kernel2D::prewitt(Kernel2D::Horizontal)));

//	processing_chain->addProcessingStage(new SeparableKernelImageProcessor(Kernel2D::scharr(Kernel2D::Vertical),
//																		   Kernel2D::scharr(Kernel2D::Horizontal)));

	processing_chain->addProcessingStage(new BinarizationImageProcessor(0.20f));


	processing_chain->addInput(original_image);
	processing_chain->addOutput(processed_image);
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
	CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);

	(*processing_chain)( CoreEngine::instance()->renderEngine() );

	Matrix projection = Matrix::ortho2D(0, width, 0, height);
	CoreEngine::instance()->renderEngine().pushDrawStates();
	output_quadrilateral->states()->getProgram()->uniform("mvp").set(projection);
	CoreEngine::instance()->renderEngine().draw( *original_quadrilateral );
	CoreEngine::instance()->renderEngine().draw( *output_quadrilateral );
	CoreEngine::instance()->renderEngine().popDrawStates();
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window)
	{
	}

private:


	SharedPointer<ProcessingChain> processing_chain;
	SharedPointer<Texture2D> original_image;
	SharedPointer<Quadrilateral> original_quadrilateral;

	SharedPointer<Texture2D> processed_image;
	SharedPointer<Quadrilateral> output_quadrilateral;
};

RENGINE_BOOT();
