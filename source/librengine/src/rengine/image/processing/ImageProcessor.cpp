// __!!rengine_copyright!!__ //

#include <rengine/image/processing/ImageProcessor.h>

#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/util/StringTable.h>
#include <rengine/state/Program.h>
#include <rengine/string/String.h>

namespace rengine
{
	ImageProcessor::~ImageProcessor()
	{
	}

	std::string ImageProcessor::effectFile() const
	{
		return "";
	}

	bool ImageProcessor::initialize()
	{
		if (initialized_)
		{
			return initialized_;
		}

		if (!quadrilateral_)
		{
			return initialized_;
		}

		OpaqueProperties properties;
		properties.push_back(OpaqueProperty("cache_option", BaseResourceLoader::DoNoCacheResources)); // Force no cache, some processors are dynamically generated
		properties.push_back(OpaqueProperty("symbols", shader_symbols_));

		SharedPointer<Program> program = CoreEngine::instance()->resourceManager().load<Program>( effectFile(), properties);

		if (program)
		{
			quadrilateral_->states()->setProgram(program);

			float width = float(inputs_[0]->getWidth());
			float height = float(inputs_[0]->getHeight());

			if (program->hasUniform("mvp"))
			{
				Matrix mvp = Matrix::ortho2D(0.0, width, 0.0f, height);
				program->uniform("mvp").set(mvp);
			}

			if (program->hasUniform("texture_0"))
			{
				program->uniform("texture_0").set(0);
			}

			if (program->hasUniform("pixel_size"))
			{
				Vector2D pixe_size(1.0f / width, 1.0f / height);
				program->uniform("pixel_size").set(pixe_size);
			}

			initialized_ = true;
		}

		return initialized_;
	}

	void ImageProcessor::operator()(RenderEngine& render_engine)
	{
		if (initialized_)
		{
			render_engine.draw(*quadrilateral_);
		}
	}

	//
	// ProcessingChain
	//

	void ProcessingChain::addProcessingStage(ProcessingStage const& stage)
	{
		stages_.push_back(stage);
	}

	ProcessingChain::ProcessingChain()
	{

	}

	static Bool usePing(Bool const& as_input, Int const& stage, Int const& total_stages)
	{
		Bool use_ping = true;

		Bool size_even = ( (total_stages % 2) == 0 );
		Bool stage_even = ( (stage % 2) == 0 );

		if (as_input)
		{
			use_ping = (size_even != stage_even);
		}
		else
		{
			use_ping = (size_even == stage_even);
		}

		return use_ping;
	}

	bool ProcessingChain::initialize()
	{
		if (initialized_)
		{
			return initialized_;
		}

		if (inputs_.size() == 0 || outputs_.size() == 0)
		{
			return initialized_;
		}

		//
		// Multiple connections is still not supports
		//
		RENGINE_ASSERT(inputs_.size() == 1);
		RENGINE_ASSERT(outputs_.size() == 1);


		states.setCapability(DrawStates::DepthTest, DrawStates::Off);

		float width = float(inputs_[0]->getWidth());
		float height = float(inputs_[0]->getHeight());
		float x = 0.0f;
		float y = 0.0f;

		ping_ = new FrameBuffer(width, height);

		if (stages_.size() == 1)
		{
			SharedPointer<Quadrilateral> quad_ = new Quadrilateral();
			quad_->setCornersVertex(Vector3D(x, y, 0.0f), Vector3D(width, height, 0.0f));
			quad_->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));

			for (Connections::size_type i = 0; i != inputs_.size(); ++i)
			{
				stages_[0]->addInput(inputs_[i]);
				quad_->states()->setTexture(inputs_[i]);
			}

			for (Connections::size_type i = 0; i != outputs_.size(); ++i)
			{
				stages_[0]->addOutput(outputs_[i]);
				ping_->attach(outputs_[i]);
			}

			stages_[0]->setQuadrilateral(quad_);
			initialized_ = stages_[0]->initialize();
		}
		else
		{
			pong_ = new FrameBuffer(width, height);

			SharedPointer<Texture2D> pong_texture_ = outputs_[0];
			pong_->attach(pong_texture_);

			ping_texture_ = new Texture2D(outputs_[0]->getInternalFormat());
			ping_->attach(ping_texture_);

			Bool no_error = true;
			for (ProcessingStages::size_type stage = 0; (stage != stages_.size()) && no_error; ++ stage)
			{
				SharedPointer<Quadrilateral> quad_ = new Quadrilateral();
				quad_->setCornersVertex(Vector3D(x, y, 0.0f), Vector3D(width, height, 0.0f));
				quad_->setCornersTextureCoordinates(Vector2D(0.0f, 0.0f), Vector2D(1.0f, 1.0f));

				//
				// Connect inputs
				//
				if (stage == 0)
				{
					for (Connections::size_type i = 0; i != inputs_.size(); ++i)
					{
						stages_[stage]->addInput(inputs_[i]);
						quad_->states()->setTexture(inputs_[i]);
					}
				}
				else
				{
					SharedPointer<Texture2D> texture = usePing(true, stage, stages_.size()) ? ping_texture_ : pong_texture_;
					quad_->states()->setTexture(texture);
					stages_[stage]->addInput(texture);
				}


				//
				// Connect Outputs
				//
				if (stage == (stages_.size() - 1))
				{
					stages_[stage]->addOutput(pong_texture_);
				}


				stages_[stage]->setQuadrilateral(quad_);
				no_error &= stages_[stage]->initialize();
			}

			initialized_ = no_error;
		}

		return initialized_;
	}

	void ProcessingChain::operator()(RenderEngine& render_engine)
	{
		if (initialize())
		{
			Uint viewport_x = render_engine.getViewportX();
			Uint viewport_y = render_engine.getViewportY();
			Uint viewport_width = render_engine.getViewportWidth();
			Uint viewport_height = render_engine.getViewportHeight();

			render_engine.pushDrawStates();
			render_engine.apply(states);

			render_engine.setViewport(0, 0, ping_->width(), ping_->height());

			if (stages_.size() == 1)
			{
				render_engine.bind(*ping_);
				render_engine.clearBuffers();

				(*stages_[0])(render_engine);

				render_engine.unbind(*ping_);

			}
			else
			{
				for (ProcessingStages::size_type stage = 0; stage != stages_.size(); ++ stage)
				{
					Bool use_ping = usePing(false, stage, stages_.size());

					render_engine.bind(use_ping ? *ping_ : *pong_);
					render_engine.clearBuffers();

					(*stages_[stage])(render_engine);

					render_engine.unbind(use_ping ? *ping_ : *pong_);
				}
			}


			render_engine.popDrawStates();

			render_engine.setViewport(viewport_x, viewport_y, viewport_width, viewport_height);
		}
	}

	//
	// ThresholdImageProcessor
	//

	ThresholdImageProcessor::ThresholdImageProcessor()
	: threshold_(0.5), value_(0.0f, 0.0f, 0.0f, 1.0f)
	{}

	ThresholdImageProcessor::ThresholdImageProcessor(Real const& threshold)
	: threshold_(threshold), value_(0.0f, 0.0f, 0.0f, 1.0f)
	{
	}

	bool ThresholdImageProcessor::initialize()
	{
		if (initialized_)
		{
			return initialized_;
		}


		if (ImageProcessor::initialize())
		{
			if (quadrilateral_->states()->hasProgram())
			{
				quadrilateral_->states()->getProgram()->uniform("threshold_value").set(value_);
				quadrilateral_->states()->getProgram()->uniform("threshold").set(threshold_);
			}
		}

		return initialized_;
	}

	//
	// BinarizationImageProcessor
	//

	BinarizationImageProcessor::BinarizationImageProcessor()
	: threshold_(0.5)
	{}

	BinarizationImageProcessor::BinarizationImageProcessor(Real const& threshold)
	: threshold_(threshold)
	{}

	bool BinarizationImageProcessor::initialize()
	{
		if (initialized_)
		{
			return initialized_;
		}


		if (ImageProcessor::initialize())
		{
			if (quadrilateral_->states()->hasProgram())
			{
				quadrilateral_->states()->getProgram()->uniform("threshold").set(threshold_);
			}
		}

		return initialized_;
	}

	//
	// KernelImageProcessor
	//

	KernelImageProcessor::KernelImageProcessor(SharedPointer<Kernel> kernel)
	:kernel_(kernel)
	{
	}

	bool KernelImageProcessor::initialize()
	{
		if (initialized_)
		{
			return initialized_;
		}


		StringElement element;
		shader_symbols_.clear();

		element.text = lexical_cast<std::string>( kernel_->dataSize() );
		shader_symbols_["kernel_array_size"] = element;

		element.text = "\n" + kernel_->toString() + "\n";
		shader_symbols_["kernel_array"] = element;

		element.text = lexical_cast<std::string>( floor( float(kernel_->size()) / 2.0f ));
		shader_symbols_["kernel_half_size"] = element;

		element.text = (kernel_->dimensions() == 1) ? "" : "/*";
		shader_symbols_["convolution_1d_begin"] = element;

		element.text = (kernel_->dimensions() == 1) ? "" : "*/";
		shader_symbols_["convolution_1d_end"] = element;

		element.text = (kernel_->dimensions() == 2) ? "" : "/*";
		shader_symbols_["convolution_2d_begin"] = element;

		element.text = (kernel_->dimensions() == 2) ? "" : "*/";
		shader_symbols_["convolution_2d_end"] = element;


		if (ImageProcessor::initialize())
		{
			if (quadrilateral_->states()->hasProgram() && (kernel_->dimensions() == 1))
			{
				Vector2D pixel_size( *( (Vector2D*) quadrilateral_->states()->getProgram()->uniform("pixel_size").data() ) );

				if (kernel_->orientation() == Kernel::Vertical)
				{
					pixel_size.x() = 0.0f;
				}

				if (kernel_->orientation() == Kernel::Horizontal)
				{
					pixel_size.y() = 0.0f;
				}

				quadrilateral_->states()->getProgram()->uniform("pixel_size").set(pixel_size);
			}
		}


		//CoreEngine::instance()->renderEngine().reportProgram( *quadrilateral_->states()->getProgram() );

		return initialized_;
	}

	SeparableKernelImageProcessor::SeparableKernelImageProcessor(SharedPointer<Kernel> kernel0, SharedPointer<Kernel> kernel1)
	:kernel0_(kernel0), kernel1_(kernel1)
	{
	}

	bool SeparableKernelImageProcessor::initialize()
	{
		if (initialized_)
		{
			return initialized_;
		}

		StringElement element;
		shader_symbols_.clear();

		element.text = lexical_cast<std::string>( kernel0_->dataSize() );
		shader_symbols_["kernel_array_size"] = element;

		element.text = "\n" + kernel0_->toString() + "\n";
		shader_symbols_["kernel0_array"] = element;

		element.text = "\n" + kernel1_->toString() + "\n";
		shader_symbols_["kernel1_array"] = element;

		element.text = lexical_cast<std::string>( floor( float(kernel0_->size()) / 2.0f ));
		shader_symbols_["kernel_half_size"] = element;

		element.text = (kernel0_->dimensions() == 1) ? "" : "/*";
		shader_symbols_["convolution_1d_begin"] = element;

		element.text = (kernel0_->dimensions() == 1) ? "" : "*/";
		shader_symbols_["convolution_1d_end"] = element;

		element.text = (kernel0_->dimensions() == 2) ? "" : "/*";
		shader_symbols_["convolution_2d_begin"] = element;

		element.text = (kernel0_->dimensions() == 2) ? "" : "*/";
		shader_symbols_["convolution_2d_end"] = element;

		ImageProcessor::initialize();

		//CoreEngine::instance()->renderEngine().reportProgram( *quadrilateral_->states()->getProgram() );

		return initialized_;
	}


	std::string CopyImageProcessor::effectFile() const
	{
		return CoreEngine::instance()->locationTable().lookUp("image_processing_copy");
	}

	std::string GrayscaleImageProcessor::effectFile() const
	{
		return CoreEngine::instance()->locationTable().lookUp("image_processing_grayscale");
	}

	std::string ThresholdImageProcessor::effectFile() const
	{
		return CoreEngine::instance()->locationTable().lookUp("image_processing_threshold");
	}

	std::string BinarizationImageProcessor::effectFile() const
	{
		return CoreEngine::instance()->locationTable().lookUp("image_processing_binarization");
	}

	std::string KernelImageProcessor::effectFile() const
	{
		return CoreEngine::instance()->locationTable().lookUp("image_processing_kernel");
	}

	std::string SeparableKernelImageProcessor::effectFile() const
	{
		return CoreEngine::instance()->locationTable().lookUp("image_processing_separable_kernel");
	}

} //namespace rengine
