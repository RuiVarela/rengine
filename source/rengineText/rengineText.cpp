#include <rengine/util/Bootstrap.h>


class MainScene : public Scene, public InterfaceEventHandler
{
public:
	MainScene() {}
	virtual ~MainScene() {}

	virtual void init()
	{
		//CoreEngine::instance()->log() << std::endl << std::endl;
		//CoreEngine::instance()->renderEngine().reportDrawStates();
		//CoreEngine::instance()->renderEngine().pushDrawStates();

		//DrawStates states;
		//states.setState(new BlendFunction());
		//states.setState(new BlendEquation());
		//states.setState(new BlendColor());
		//states.setState(new CullFace());
		//states.setState(new Depth());
		//states.setState(new PolygonMode());
		//states.setState(new ColorChannelMask());


		//CoreEngine::instance()->log() << std::endl << std::endl;
		//CoreEngine::instance()->renderEngine().apply(states);
		//CoreEngine::instance()->renderEngine().reportDrawStates();

		//CoreEngine::instance()->renderEngine().popDrawStates();


		DrawStates base_states;
		base_states.setState(new BlendFunction());
		CoreEngine::instance()->renderEngine().apply(base_states);


		std::string font_filename = "";
		//font_filename = convertFileNameToNativeStyle("data/fonts/fon/doslike.fon");
		//font_filename = convertFileNameToNativeStyle("data/fonts/fon/10x20-iso8859-1.fon");
		font_filename = convertFileNameToNativeStyle("data/fonts/fon/9x15-iso8859-1.fon");
		//font_filename = convertFileNameToNativeStyle("data/fonts/fon/8x13-iso8859-1.fon");
		//font_filename = convertFileNameToNativeStyle("data/fonts/fon/fixed.fon");
		//font_filename = convertFileNameToNativeStyle("data/fonts/fon/fixedc.fon");
		//font_filename = convertFileNameToNativeStyle("data/fonts/fon/tektite.fon");



		font = CoreEngine::instance()->resourceManager().load<Font>(font_filename);

		text_0 = new Text();
		text_0->setFont(font);
		text_0->setPosition(Vector3D(0.0f, 10.0f, 0.0f));
		text_0->setText("Text String using cm metrics");
		text_0->setColor(Vector4D(1.0f, 0.0f, 0.0f, 1.0f));

		font_filename = convertFileNameToNativeStyle("data/fonts/ttf/white_rabbit.ttf");
		font = CoreEngine::instance()->resourceManager().load<Font>(font_filename);
		//font = CoreEngine::instance()->resourceManager().load<Font>("Default");

		text_1 = new Text(Text::PixelMetrics);
		text_1->setFont(font);
		text_1->setPosition(Vector3D(0.0f, 300.0f, 0.0f));
		text_1->setText("Text String using px metrics");
		text_1->setColor(Vector4D(1.0f, 1.0f, 0.0f, 1.0f));

		hud_writer = new HudWriter();

		texture_quadrilateral = new Quadrilateral();
		texture_quadrilateral->setCornersVertex(Vector3D(0.0f, 0.0f, 0.0f), Vector3D(font->texture()->getWidth(), font->texture()->getHeight(), 0.0f));
		texture_quadrilateral->states()->setTexture(font->texture());
		texture_quadrilateral->states()->setProgram( CoreEngine::instance()->resourceManager().load<Program>("data/shaders/core/Text.eff"));
	}

	virtual void shutdown()
	{
	}

	virtual void update()
	{
		hud_writer->clear();
	}

	virtual void render()
	{
		CoreEngine::instance()->renderEngine().clearBuffers();

		float const width = (float) CoreEngine::instance()->mainWindow()->contextOptions().width;
		float const height = (float) CoreEngine::instance()->mainWindow()->contextOptions().height;

		raw::render()->pushDrawStates();


		if (texture_quadrilateral->states()->hasState(DrawStates::Program))
		{
			Matrix projection = Matrix::ortho2D(0, width, 0, height);
			Program* program = dynamic_cast<ProgramUnit*>( texture_quadrilateral->states()->getState(DrawStates::Program).first.get() )->get().get();
			program->uniform("mvp").set(projection);
		}

		raw::render()->draw( *texture_quadrilateral );

		raw::render()->draw( *text_0 );
		raw::render()->draw( *text_1 );

		hud_writer->clear();
		hud_writer->write(Vector2D(200.0f, 20.0f), "my simpled message cenas" );
		raw::render()->draw( *hud_writer );


		raw::render()->popDrawStates();
	}

	virtual void operator()(InterfaceEvent const& interface_event, GraphicsWindow* window){ }
private:
	SharedPointer<Font> font;
	SharedPointer<Text> text_0;
	SharedPointer<Text> text_1;
	SharedPointer<HudWriter> hud_writer;

	SharedPointer<Quadrilateral> texture_quadrilateral;
};


RENGINE_BOOT();
