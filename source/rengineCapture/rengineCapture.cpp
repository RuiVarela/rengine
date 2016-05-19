#include <rengine/util/Bootstrap.h>

class MainScene : public Scene, public InterfaceEventHandler
{
public:
	typedef std::vector< SharedPointer<Mesh> > MeshVector;
	MainScene() {}
	virtual ~MainScene() {}

	virtual void init()
	{
		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
		CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);


		video_capture = VideoCapture::create();

		if (true)
		{
			SharedPointer<ThreadedVideoCapture> threaded_video_capture = new ThreadedVideoCapture(video_capture);
			video_capture = threaded_video_capture;
		}

		VideoCapture::Devices devices = video_capture->enumerateDevices();

		for (Uint index = 0; index != Uint(devices.size()); ++index)
		{

			CoreEngine::instance()->log() << "--- Capture Device [" << devices[index].index << "] ---" << std::endl;
			CoreEngine::instance()->log() << "Location \t" << devices[index].location << std::endl;
			CoreEngine::instance()->log() << "Name \t\t" << devices[index].name << std::endl;
			CoreEngine::instance()->log() << "Driver \t\t" << devices[index].driver << std::endl;
			CoreEngine::instance()->log() << "Bus \t\t" << devices[index].bus << std::endl;

			VideoCapture::Format format;
			//format.mode = "YUY2";
			//format.mode = "YUYV";
			//format.mode = "MJPG";
			format.interval_denominator = 5;
			//format.width = 320;
			//format.height = 120;

			VideoCapture::Formats formats = video_capture->enumerateFormats(devices[index]);
			formats = VideoCapture::filterByFormat( video_capture->enumerateFormats(devices[index]) , format);
			format = VideoCapture::matchBestFormat( video_capture->enumerateFormats(devices[index]) , format);

			CoreEngine::instance()->log() << "[B] \t"
				<< format.mode << " \t"
				<< format.width << "x" << format.height << " \t"
				<< format.interval_numerator << " / " << format.interval_denominator << std::endl;

			for (Uint format_index = 0; format_index != Uint(formats.size()); ++format_index)
			{
				format = formats[format_index];
				CoreEngine::instance()->log() << "[" << format_index << "] \t"
					<< format.mode << " \t"
					<< format.width << "x" << format.height << " \t"
					<< format.interval_numerator << " / " << format.interval_denominator << std::endl;
			}
		}

		VideoCapture::CaptureOptions options;
		//options.mode = "MJPG";
		//options.mode = "YUY2";
		//options.mode = "RGB3";
		//options.width = 640;
		//options.interval_denominator = 5;

		try
		{
			video_capture->open(options);
		}
		catch (VideoCaptureException caught)
		{
			cout << caught << endl;
		}

		if (video_capture->ready())
		{
			SharedPointer<Image> image = new Image(video_capture->captureOptions().width, video_capture->captureOptions().height, 3);
			image->zeroImage();

			texture = new Texture2D(image);
			quadrilateral = new Quadrilateral();
			quadrilateral->setCornersVertex(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(Real(video_capture->captureOptions().width),
				Real(video_capture->captureOptions().height), 0.0f));
			quadrilateral->states()->setTexture(texture);

			SharedPointer<Program> decal_program = CoreEngine::instance()->resourceManager().load<Program>("data/shaders/core/DecalTexture.eff");

			quadrilateral->states()->setCapability(DrawStates::DepthTest, DrawStates::Off);
			quadrilateral->states()->setCapability(DrawStates::Blend, DrawStates::Off);
			quadrilateral->states()->setCapability(DrawStates::CullFace, DrawStates::Off);
			quadrilateral->states()->setProgram(decal_program);
		}
	}

	virtual void shutdown()
	{
		video_capture->close();
	}

	virtual void update()
	{
		try
		{
			VideoCapture::FrameOptions options;
			frame = video_capture->grab(options);

			if (frame && frame->data)
			{
				// Image object will destroy frame->data
				SharedPointer<Image> image = new Image(video_capture->captureOptions().width,
					video_capture->captureOptions().height, 3, frame->data);
				image->flip(Image::FlipVertical);
				texture->setImage(image);
			}
		}
		catch (VideoCaptureException caught)
		{
			cout << caught << endl;
		}
	}

	virtual void render()
	{

		CoreEngine::instance()->renderEngine().clearBuffers();

		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;
		CoreEngine::instance()->renderEngine().setViewport(0, 0, width, height);


		Matrix projection = Matrix::ortho2D(0, width, 0, height);
		CoreEngine::instance()->renderEngine().pushDrawStates();

		if (video_capture->ready())
		{
			quadrilateral->states()->getProgram()->uniform("mvp").set( Matrix::ortho2D(0, width, 0, height) );
			CoreEngine::instance()->renderEngine().draw( *quadrilateral );
		}

		CoreEngine::instance()->renderEngine().popDrawStates();
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
	SharedPointer<VideoCapture> video_capture;
	SharedPointer<VideoCapture::Frame> frame;

	SharedPointer<Quadrilateral> quadrilateral;
	SharedPointer<Texture2D> texture;
};

RENGINE_BOOT();