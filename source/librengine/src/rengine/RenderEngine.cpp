// __!!rengine_copyright!!__ //

#include <rengine/lang/Platform.h>
#include <rengine/CoreEngine.h>
#include <rengine/outputstream/OutputStream.h>
#include <rengine/Configuration.h>
#include <rengine/RenderEngine.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/string/String.h>

#include <rengine/state/BaseStates.h>
#include <rengine/state/Texture.h>
#include <rengine/state/Streams.h>
#include <rengine/state/DrawResource.h>
#include <rengine/state/FrameBuffer.h>

#include <rengine/outputstream/Log.h>

#include <rengine/geometry/VertexBuffer.h>
#include <rengine/lang/debug/Debug.h>

#include <stack>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iomanip>

#include <ctime>
#include <cstdlib>
#include <GL/glew.h>

#include <iostream>
using namespace std;

//#define RENGINE_LOG_DRAW_STATE_ENABLED

#ifndef RENGINE_DEBUG_MODE
#define RENDER_ENGINE_SANITY_CHECK
#endif

#define RENGINE_LOG(message) \
			CoreEngine::instance()->log() << message << std::endl;

#define RENGINE_LOG_STATE \
			RENGINE_LOG("Apply State      [" << state.name() << "] " << state)

#define RENGINE_LOG_CAPABILITY \
			RENGINE_LOG("Apply Capability [" << capability << "] [" << value << "]")

#ifdef RENGINE_LOG_DRAW_STATE_ENABLED
	#define RENGINE_LOG_STATE_APPLY RENGINE_LOG_STATE
	#define RENGINE_LOG_CAPABILITY_APPLY RENGINE_LOG_CAPABILITY
#else
	#define RENGINE_LOG_STATE_APPLY
	#define RENGINE_LOG_CAPABILITY_APPLY
#endif

static const std::string log_name = "render_engine";

namespace rengine
{
	typedef std::stack< SharedPointer<Matrix> > MatrixStack;
	typedef std::stack< SharedPointer<DrawStates> > DrawStatesStack;

	struct ChannelInputBinding
	{
		ChannelInputBinding() : channel_available(true), input_available(true) {}
		Bool channel_available;
		Bool input_available;
	};

	struct RenderEngine::PrivateImplementation
	{
		PrivateImplementation() :
			model_view_matrix(new Matrix()),
			projection_matrix(new Matrix()),
			draw_states(new DrawStates()),
			clear_depth(1.0)
		{
		}

		SharedPointer<Matrix> model_view_matrix;
		SharedPointer<Matrix> projection_matrix;
		MatrixStack model_view_matrix_stack;
		MatrixStack projection_matrix_stack;

		SharedPointer<DrawStates> draw_states;
		DrawStatesStack draw_states_stack;

		SharedPointer<Program> program;

		Vector4D clear_color;
		Real64 clear_depth;
		Int clear_stencil;

		Uint viewport_x;
		Uint viewport_y;
		Uint viewport_width;
		Uint viewport_height;

		// caching and speeding

		std::vector<ChannelInputBinding> channel_input_cache;
		DrawStates::StateVector empty_state_vector;
	};

	RenderEngine::RenderEngine()
	:implementation(new PrivateImplementation())
	{
		srand(Uint (time(0)));
	}

	RenderEngine::~RenderEngine()
	{
		delete(implementation);
	}

    void RenderEngine::init()
    {
    	//glShadeModel(GL_SMOOTH);

    	implementation->model_view_matrix->setAsIdentity();
    	implementation->projection_matrix->setAsIdentity();

    	implementation->viewport_x = 0;
    	implementation->viewport_y = 0;
    	implementation->viewport_width = 1;
    	implementation->viewport_height = 1;

		setClearColor(Vector4D());
		setClearDepth(1.0);
		setClearStencil(0);

		clearDrawStates();

		clearBuffers();
    }

    void RenderEngine::shutdown()
    {
    	implementation->draw_states = 0;
    }

	void RenderEngine::preFrame()
	{
		/*
		for (Uint current_window = 0; current_window != CoreEngine::instance()->windows().size(); ++current_window)
		{
			CoreEngine::instance()->makeCurrentContext(CoreEngine::instance()->windows()[current_window].get());
		}
		*/
	}

	void RenderEngine::postFrame()
	{
		for (Uint current_window = 0; current_window != CoreEngine::instance()->windows().size(); ++current_window)
		{
			//CoreEngine::instance()->makeCurrentContext(CoreEngine::instance()->windows()[current_window].get());
			//glFinish();
			CoreEngine::instance()->windows()[current_window]->swapBuffers();
		}
	}


	//
	// Info Methods
	//

	std::string RenderEngine::vendor() const
	{
		if (glGetString(GL_VENDOR))
		{
			return std::string((Char*) glGetString(GL_VENDOR) );
		}
		return "";
	}

	std::string RenderEngine::renderer() const
	{
		if (glGetString(GL_RENDERER))
		{
			return std::string((Char*) glGetString(GL_RENDERER) );
		}
		return "";
	}

	std::string RenderEngine::version() const
	{
		if (glGetString(GL_VERSION))
		{
			return std::string((Char*) glGetString(GL_VERSION) );
		}
		return "";
	}

	std::string RenderEngine::shadingLanguageVersion() const
	{
		if (glGetString(GL_SHADING_LANGUAGE_VERSION))
		{
			return std::string((Char*) glGetString(GL_SHADING_LANGUAGE_VERSION) );
		}
		return "";
	}

	Bool RenderEngine::limitedToOpenGL21() const
	{
		return (GLEW_VERSION_2_1 && !GLEW_VERSION_3_0);
	}

	Bool RenderEngine::checkErrors(std::string const &message)
	{

		Bool error_found = true;
		switch (glGetError())
		{
			case GL_NO_ERROR:
			error_found = false;
				break;
			case GL_INVALID_ENUM:
			RLOG_ERROR(log_name, message
					<< "GL_INVALID_ENUM : An unacceptable value is specified for an enumerated argument. The offending function is ignored, having no side effect other than to set the error flag.");
				break;
			case GL_INVALID_VALUE:
			RLOG_ERROR(log_name, message
					<< "GL_INVALID_VALUE : A numeric argument is out of range. The offending function is ignored, having no side effect other than to set the error flag.");
				break;
			case GL_INVALID_OPERATION:
			RLOG_ERROR(log_name, message
					<< "GL_INVALID_OPERATION :The specified operation is not allowed in the current state. The offending function is ignored, having no side effect other than to set the error flag.");
				break;
			case GL_OUT_OF_MEMORY:
			RLOG_ERROR(log_name, message
					<< "GL_OUT_OF_MEMORY : There is not enough memory left to execute the function. The state of OpenGL is undefined, except for the state of the error flags, after this error is recorded.");
				break;
			case GL_STACK_OVERFLOW:
			RLOG_ERROR(log_name, message
					<< "GL_STACK_OVERFLOW : This function would cause a stack overflow. The offending function is ignored, having no side effect other than to set the error flag.");
				break;
			case GL_STACK_UNDERFLOW:
			RLOG_ERROR(log_name, message
					<< "GL_STACK_UNDERFLOW : This function would cause a stack underflow. The offending function is ignored, having no side effect other than to set the error flag.");
				break;
			case GL_TABLE_TOO_LARGE:
			RLOG_ERROR(log_name, message
					<< "GL_TABLE_TOO_LARGE : The specified table exceeds the implementation's maximum supported table size.  The offending command is ignored and has no other side effect than to set the error flag.");
				break;
			default:
			RLOG_ERROR(log_name, message << "Unknown gl Error.");
				break;
		}

		return error_found;
	}

	//
	// Matrix Management
	//
	void RenderEngine::setModelView(Matrix const& matrix)
	{
		if (matrix != *(implementation->model_view_matrix))
		{
			implementation->model_view_matrix->set(matrix.ptr());
		}
	}

	Matrix const& RenderEngine::modelView() const
	{
		return *(implementation->model_view_matrix);
	}

	Matrix& RenderEngine::modelView()
	{
		return *(implementation->model_view_matrix);
	}

	void RenderEngine::pushModelView()
	{
		implementation->model_view_matrix_stack.push(new Matrix(implementation->model_view_matrix->ptr()));
	}

	void RenderEngine::popModelView()
	{
		RENGINE_ASSERT(!implementation->model_view_matrix_stack.empty());
		setModelView( *implementation->model_view_matrix_stack.top() );
		implementation->model_view_matrix_stack.pop();
	}

	void RenderEngine::setProjection(Matrix const& matrix)
	{
		if (matrix != *(implementation->projection_matrix))
		{
			implementation->projection_matrix->set(matrix.ptr());
		}
	}

	Matrix const& RenderEngine::projection() const
	{
		return *(implementation->projection_matrix);
	}

	Matrix& RenderEngine::projection()
	{
		return *(implementation->projection_matrix);
	}

	void RenderEngine::pushProjection()
	{
		implementation->projection_matrix_stack.push( new Matrix(implementation->projection_matrix->ptr()) );
	}

	void RenderEngine::popProjection()
	{
		RENGINE_ASSERT(!implementation->projection_matrix_stack.empty());
		setProjection(*implementation->projection_matrix_stack.top());
		implementation->projection_matrix_stack.pop();
	}

	void RenderEngine::setViewport(Uint const x, Uint const y, Uint const width, Uint const height)
	{
    	implementation->viewport_x = x;
    	implementation->viewport_y = y;
    	implementation->viewport_width = width;
    	implementation->viewport_height = height;

		glViewport(x, y, width, height);
	}

	Uint RenderEngine::getViewportX() const
	{
		return implementation->viewport_x;
	}

	Uint RenderEngine::getViewportY() const
	{
		return implementation->viewport_y;
	}

	Uint RenderEngine::getViewportWidth() const
	{
		return implementation->viewport_width;
	}

	Uint RenderEngine::getViewportHeight() const
	{
		return implementation->viewport_height;
	}

	//
	// Base Buffer Management
	//
	void RenderEngine::clearBuffers(BufferType const& buffers)
	{
		glClear(buffers);
	}

	void RenderEngine::setClearColor(Vector4D const& color)
	{
		implementation->clear_color = color;
		glClearColor(color.r(), color.g(), color.b(), color.a());
	}

	Vector4D const& RenderEngine::getClearColor() const
	{
		return implementation->clear_color;
	}

	void RenderEngine::setClearDepth(Real64 const value)
	{
		implementation->clear_depth = value;
		glClearDepth(value);
	}

	Real64 RenderEngine::getClearDepth() const
	{
		return implementation->clear_depth;
	}

	void RenderEngine::setClearStencil(Int const value)
	{
		implementation->clear_stencil =  value;
		glClearStencil(value);
	}

	Int RenderEngine::getClearStencil() const
	{
		return implementation->clear_stencil;
	}

	//
	// StateMachine
	//

	void RenderEngine::clearDrawStates()
	{
		implementation->draw_states->clear();

		implementation->program = 0;
		implementation->draw_states->setState(new ProgramUnit(), DrawStates::Off);

		applyCapability(DrawStates::Blend, DrawStates::Off);
		applyCapability(DrawStates::CullFace, DrawStates::Off);
		applyCapability(DrawStates::DepthTest, DrawStates::Off);
		applyCapability(DrawStates::StencilTest, DrawStates::Off);
	}

	DrawStates const& RenderEngine::getDrawStates() const
	{
		return *implementation->draw_states;
	}

	void RenderEngine::pushDrawStates()
	{
		implementation->draw_states_stack.push( new DrawStates(*(implementation->draw_states)) );
	}

	void RenderEngine::popDrawStates()
	{
		RENGINE_ASSERT(!implementation->draw_states_stack.empty());
		apply( *(implementation->draw_states_stack.top()) );

		implementation->draw_states_stack.pop();
	}

	void RenderEngine::reportDrawStates()
	{
		CoreEngine::instance()->log() << *implementation->draw_states;
	}

	void RenderEngine::apply(DrawStates const& states)
	{
		//
		// Apply States
		//
		for (DrawStates::StateMap::const_iterator state_iterator = states.getStates().begin();
			 state_iterator != states.getStates().end();
			 ++state_iterator)
		{
			applyState(*(state_iterator->second.first), state_iterator->second.second);
		}

		//
		// Apply aggregated states
		//

		//	Textures
		Bool reset_textures = true;
		if (states.hasState(DrawStates::Texture2D))
		{
			DrawStates::StateVector const& textures = states.getAggregationStates(DrawStates::Texture2D);
			if (textures.size())
			{
				reset_textures = false;
				applyTextures(states.getAggregationStates(DrawStates::Texture2D), true);
			}
		}

		if (reset_textures)
		{
			implementation->draw_states->createOrZeroAggregationState(DrawStates::Texture2D);
		}


//		for (DrawStates::StateVectorMap::const_iterator state_vector = states.getAggregationStates().begin();
//			 state_vector != states.getAggregationStates().end();
//			 ++state_vector)
//		{
//
//		}

		//
		// Apply capabilities
		//
		for(DrawStates::CapabilityValueMap::const_iterator capability_iterator = states.getCapabilities().begin();
			capability_iterator != states.getCapabilities().end();
			++capability_iterator)
		{
			if ( !DrawStates::isStateAggregation( capability_iterator->first ) )
			{
				applyCapability(capability_iterator->first, capability_iterator->second);
			}
		}
	}

	void RenderEngine::applyState(DrawStates::State const& state, DrawStates::Value const& value)
	{
		if (implementation->draw_states->hasState(state.type()))
		{
			DrawStates::StateValuePair state_value = implementation->draw_states->getState(state.type());

			if ((state_value.second != value) || (state_value.first->compare(state) != 0))
			{
				SharedPointer<DrawStates::State> new_state = state.clone();
				implementation->draw_states->setStateWithoutDependencyCheck(new_state, value);
				new_state->apply(*this);
			}
		}
		else
		{
			implementation->draw_states->setStateWithoutDependencyCheck(state.clone(), value);
			implementation->draw_states->getState(state.type()).first->apply(*this);
		}
	}

	void RenderEngine::applyCapability(DrawStates::Capability const& capability, DrawStates::Value const& value)
	{
		if (!implementation->draw_states->hasCapability(capability))
		{
			implementation->draw_states->setCapability(capability, value);
			RENGINE_LOG_CAPABILITY_APPLY

			if (value)
			{
				glEnable(capability);
			}
			else
			{
				glDisable(capability);
			}

		}
		else if (implementation->draw_states->getCapability(capability) != value)
		{
			implementation->draw_states->setCapability(capability, value);
			RENGINE_LOG_CAPABILITY_APPLY

			if (value)
			{
				glEnable(capability);
			}
			else
			{
				glDisable(capability);
			}
		}
	}


	void RenderEngine::apply(BlendFunction const& state)
	{
		RENGINE_LOG_STATE_APPLY
		glBlendFunc(state.getSource(), state.getDestination());
	}

	void RenderEngine::apply(BlendEquation const& state)
	{
		RENGINE_LOG_STATE_APPLY
		glBlendEquation(state.get());
	}

	void RenderEngine::apply(BlendColor const& state)
	{
		RENGINE_LOG_STATE_APPLY
		Vector4D const& v = state.get();
		glBlendColor(v.r(), v.g(), v.b(), v.a());
	}

	void RenderEngine::apply(CullFace const& state)
	{
		RENGINE_LOG_STATE_APPLY
		glCullFace(state.get());
	}

	void RenderEngine::apply(Depth const& state)
	{
		RENGINE_LOG_STATE_APPLY
	    glDepthFunc(state.getFunction());
	    glDepthRange(state.getNear(), state.getFar());
	    glDepthMask(state.getWriteFlag());
	}

	void RenderEngine::apply(ColorChannelMask const& state)
	{
		RENGINE_LOG_STATE_APPLY
		glColorMask(GLboolean(state.getRed()), GLboolean(state.getGreen()), GLboolean(state.getBlue()), GLboolean(state.getAlpha()));
	}

	void RenderEngine::apply(PolygonMode const& state)
	{
		RENGINE_LOG_STATE_APPLY

		PolygonMode::Mode const& front = state.getFront();
		PolygonMode::Mode const& back = state.getBack();

		if (front == back)
		{
			glPolygonMode(PolygonMode::FrontAndBack, front);
		}
		else
		{
			glPolygonMode(PolygonMode::Front, front);
			glPolygonMode(PolygonMode::Back, back);
		}
	}

	void RenderEngine::apply(Stencil const& state)
	{
		RENGINE_LOG_STATE_APPLY

	    glStencilFunc(state.getFunction(), state.getReference(), state.getMask());
	    glStencilOp(state.getStencilFail(), state.getStencilPassDepthFail(), state.getStencilPassDepthPass());
	    glStencilMask(state.getStencilMask());
	}

	void RenderEngine::apply(OperationBuffer const& state)
	{
		RENGINE_LOG_STATE_APPLY

		glDrawBuffer(state.getDrawBuffer());
		glReadBuffer(state.getReadBuffer());
	}

	void RenderEngine::apply(Texture2DUnit const& state)
	{
		RENGINE_LOG_STATE_APPLY

		glActiveTexture(GL_TEXTURE0 + state.getUnit());
		apply(*state.getTexture().get());
	}

	void RenderEngine::apply(ProgramUnit const& state)
	{
		RENGINE_LOG_STATE_APPLY

		implementation->program = state.get();

		if (implementation->program)
		{
			apply(*implementation->program.get());
		}
		else
		{
			glUseProgram(0);
		}
	}


	void RenderEngine::applyTextures(DrawStates::StateVector const& textures, Bool const& value)
	{
		Uint current_textures_size = implementation->draw_states->getNumberOfStates(DrawStates::Texture2D);

		// apply all target textures
		if (current_textures_size == 0)
		{
			//RENGINE_LOG("(current_textures_size == 0)");
			for (DrawStates::StateVector::const_iterator state_iterator = textures.begin(); state_iterator != textures.end(); ++state_iterator)
			{
				state_iterator->first->apply(*this);
			}

			implementation->draw_states->createOrZeroAggregationState(DrawStates::Texture2D);
			implementation->draw_states->getAggregationStates(DrawStates::Texture2D) = textures;
			return;
		}

		//RENGINE_LOG("target_textures_size == "<< target_textures_size << " - current_textures_size == " << current_textures_size);
		//enable all necessary

		Bool compare_equal = false;
		DrawStates::StateVector& current_textures = implementation->draw_states->getAggregationStates(DrawStates::Texture2D);

		for (DrawStates::StateVector::size_type i = 0; i != textures.size(); ++i)
		{
			SharedPointer<Texture2DUnit> current_texture_unit = dynamic_pointer_cast<Texture2DUnit>(textures[i].first);

			if(i < current_textures_size)
			{
				compare_equal = (textures[i].first->compare( *current_textures[i].first ) == 0);
				if (!compare_equal)
				{
					textures[i].first->apply(*this);
				}
			}
			else
			{
				textures[i].first->apply(*this);
			}
		}
		current_textures = textures;


		// check if the same texture unit was applied twice
#ifdef	RENDER_ENGINE_SANITY_CHECK
		for(Texture2DUnit::Unit i = 0; i != implementation->texture_unit_cache.size(); ++i)
		{
			for(Texture2DUnit::Unit j = i + 1; j != implementation->texture_unit_cache.size(); ++j)
			{
				if (implementation->texture_unit_cache[i] == implementation->texture_unit_cache[i])
				{
					CoreEngine::instance()->log() << "RenderEngine Warning: [" << implementation->texture_unit_cache[i] << "] Texture Unit was applied twice for the same DrawState" << std::endl;

				}
			}
		}
#endif //RENDER_ENGINE_SANITY_CHECK


	}


	//
	// Drawables
	//

	// this method applies the drawable state and calls render
	void RenderEngine::draw(Drawable& drawable)
	{
		if (drawable.hasDrawStates())
		{
			drawable.updateUniforms(*this);

			apply( *drawable.getDrawStates() );
		}

		drawable.draw(*this);
	}

	//
	// Texture
	//
	void RenderEngine::apply(Texture2D& texture)
	{
		Bool const trilinear_filtering = false;
		Real const maximum_anisotropy = 1.0f;

		ResourceId id = texture.getId(this);

		Uint change_flags = Uint( texture.changeFlags() );
		Uint texture_flags = texture.getFlags();

		if (change_flags)
		{
			// image is already uploaded?
			if (id)
			{
				glBindTexture(GL_TEXTURE_2D, id);
			}
			else
			{
				glGenTextures(1, &id);
				glBindTexture(GL_TEXTURE_2D, id);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				texture.setId(id, this);
			}

			if (change_flags & Texture2D::WrapChanged)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.getWrapS());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.getWrapT());
			}

			if (texture.isFlagSet(Texture2D::AutoFilter))
			{
				Texture2D::Filter min = Texture2D::Linear;
				Texture2D::Filter mag = Texture2D::Linear;

				if (texture.isFlagSet(Texture2D::GenerateMipmap))
				{
					if (trilinear_filtering)
					{
						min = Texture2D::LinearMipmapLinear;
					}
					else
					{
						min = Texture2D::LinearMipmapNearest;
					}
				}

				texture.setFilter(min, mag);

				// disable auto filter
				texture_flags &= ~Texture2D::AutoFilter;
				texture.setFlags(texture_flags);
			}

			if (change_flags & Texture2D::FilterChanged)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.getMagFilter());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.getMinFilter());
			}

			//
			// Upload Data
			//
			if (change_flags & Texture2D::ImageDataChanged)
			{
				if (texture.isFlagSet(Texture2D::GenerateMipmap))
				{
					CoreEngine::instance()->log() << "GL_GENERATE_MIPMAP is deprecated!" << std::endl;
					glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximum_anisotropy);
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
				}


				Int type = GL_UNSIGNED_BYTE;
				Uchar* data = 0;

				if (texture.getImage().get())
				{
					data = texture.getImage()->getData();
				}

				checkErrors("before:");
				glTexImage2D(GL_TEXTURE_2D, 0, texture.getInternalFormat(), texture.getWidth(), texture.getHeight(), 0, texture.getFormat(), type, data);
				checkErrors("after:");

				if (texture.isFlagSet(Texture2D::ReleaseImage))
				{
					texture.setImage(0);
				}
			}

			texture.clearChangeFlags();
		}
		else //if (texture.textureId())
		{
			glBindTexture(GL_TEXTURE_2D, id);
		}
	}

	Bool RenderEngine::supportsNonPowerOfTwoTextures() const
	{
		return (glewIsExtensionSupported("GL_ARB_texture_non_power_of_two") == GL_TRUE);
	}

	Bool RenderEngine::supportsTextureRectangle() const
	{
		return (glewIsExtensionSupported("GL_ARB_texture_rectangle") == GL_TRUE);
	}

	Uint RenderEngine::maximumTextureSizeSupported() const
	{
		Int max_texture_size = 0;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
		return max_texture_size;
	}

	Int RenderEngine::getTextureFormatFromChannels(Int color_channels)
	{
		Int format = GL_RGBA;

		switch (color_channels)
		{
			case 1:
			format = GL_LUMINANCE;
				break;
			case 2:
			format = GL_LUMINANCE_ALPHA;
				break;
			case 3:
			format = GL_RGB;
				break;
			case 4:
			format = GL_RGBA;
				break;
		};

		return format;
	}

	void RenderEngine::unloadTexture(Texture2D& texture)
	{
		if (texture.drawResourceLoaded(this))
		{
			ResourceId id = texture.getId(this);

			glBindTexture(GL_TEXTURE_2D, id);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 0, 0, 0, GL_RGB, GL_UNSIGNED_BYTE, 0x0);
			glBindTexture(GL_TEXTURE_2D, 0);

			glDeleteTextures(1, &id);

			texture.setId(0, this);
		}
	}

	Uchar* RenderEngine::downloadTexture2DData(Int color_channels, Int *width, Int *height, ResourceId id)
	{
		glBindTexture(GL_TEXTURE_2D, id);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, height);

		Uchar *pixels = new Uchar[*width * *height * color_channels];

		Int type = GL_UNSIGNED_BYTE;
		Int format = getTextureFormatFromChannels(color_channels);

		glGetTexImage(GL_TEXTURE_2D, 0, format, type, pixels);
		glBindTexture(GL_TEXTURE_2D, 0);

		return pixels;
	}


	//
	// Shader Handling
	//

	void RenderEngine::apply(Program& program)
	{
		RENGINE_LOG_STATE_APPLY

		if (program.changeFlags())
		{
			if (program.isChangeFlagSet(Program::VertexShaderChanged) ||
				program.isChangeFlagSet(Program::FragmentShaderChanged))
			{
				std::stringstream shader_log;

				// load shaders if needed
				Uint number_of_shaders_loaded = 0;

				Uint current_shader = Uint (Shader::Fragment);
				Uint end_shader = Uint (Shader::Vertex) + 1;

				while (current_shader != end_shader)
				{
					if (program.getShader(Shader::Type(current_shader)))
					{
						if (!program.getShader(Shader::Type(current_shader))->drawResourceLoaded(this))
						{
							std::string log;
							loadShader(*program.getShader(Shader::Type(current_shader)), log);
							log = trim(TrimBoth, log);

							if (!log.empty() && 
								!endsWith(log, "No errors.") //Mobile Intel(R) 4 Series Express Chipset Family
								)
							{
								displayShaderLog(*program.getShader(Shader::Type(current_shader)), shader_log, log);
							}

							if (!program.getShader(Shader::Type(current_shader))->drawResourceLoaded(this))
							{
								program.removeShader(Shader::Type(current_shader));
							}
						}
					}

					if (program.getShader(Shader::Type(current_shader)))
					{
						number_of_shaders_loaded += program.getShader(Shader::Type(current_shader))->drawResourceLoaded(this) ? 1 : 0;
					}

					current_shader++;
				}

				//
				// Check if needs link
				//
				if (number_of_shaders_loaded)
				{
					//
					// Rebuild program
					//
					ResourceId resource_id = program.getId(this);
					if (resource_id)
					{
						unloadProgram(program);
					}

					resource_id = glCreateProgram();
					program.setId(resource_id, this);

					if (program.getShader(Shader::Vertex))
					{
						glAttachShader(resource_id, program.getShader(Shader::Vertex)->getId(this));
					}

					if (program.getShader(Shader::Fragment))
					{
						glAttachShader(resource_id, program.getShader(Shader::Fragment)->getId(this));
					}

					//
					// Link Program and Check for ok
					//
					std::string log;
					linkProgram(program, log);
					log = trim(TrimBoth, log);

					if (!log.empty() && 
						!endsWith(log, "No errors.") //Mobile Intel(R) 4 Series Express Chipset Family
						)
					{
						shader_log << log << std::endl;
					}

					//
					// Get Uniforms location
					//
					for (Program::Uniforms::size_type i = 0; i != program.uniforms().size(); ++i)
					{
						GLint uniform_id = glGetUniformLocation(resource_id, program.uniforms()[i]->name().c_str() );
						if (uniform_id == -1)
						{
							program.uniforms()[i]->changeFlags() |= Uniform::NotFound;
							shader_log << "Uniform " << program.uniforms()[i]->name().c_str() << " not found." << std::endl;
						}
						else
						{
							program.uniforms()[i]->setId(ResourceId(uniform_id), this);
						}
					}

					//
					// Get Input Locations
					//
					for (Program::Connections::size_type i = 0; i != program.inputs().size(); ++i)
					{
						//std::cout << "Testing : |" << program.inputs()[i].name.c_str() << "|" << std::endl;
						program.inputs()[i].id = glGetAttribLocation(resource_id, program.inputs()[i].name.c_str() );

						if (program.inputs()[i].id == -1)
						{
							shader_log << "Program input " << program.inputs()[i].name << " not found." << std::endl;
						}
					}

					//
					// Get Frag Data Location
					//
					for (Program::Connections::size_type i = 0; i != program.outputs().size(); ++i)
					{
						program.outputs()[i].id  = glGetFragDataLocation(resource_id, program.outputs()[i].name.c_str() );
						if (program.outputs()[i].id == -1)
						{
							shader_log << "Program output " << program.outputs()[i].name << " not found." << std::endl;
						}
					}

					program.prepareConnections();
				}

				if (!shader_log.str().empty())
				{
					displayProgramLog(program, shader_log.str());
				}
			}

			glUseProgram( program.getId(this) );

			//
			// Apply Uniform Values
			//

			if (program.isChangeFlagSet(Program::VertexShaderChanged) ||
				program.isChangeFlagSet(Program::FragmentShaderChanged) ||
				program.isChangeFlagSet(Program::UniformsChanged))
			{
				updateUniforms(program);
			}


			program.clearChangeFlags();
		}
		else
		{
			glUseProgram( program.getId(this) );
		}
	}

	void RenderEngine::updateUniforms(Program& program)
	{
		for (Program::Uniforms::size_type i = 0; i != program.uniforms().size(); ++i)
		{
			Uniform* uniform = program.uniforms()[i].get();

			if (!uniform->isChangeFlagSet(Uniform::NotFound) &&
				 uniform->isChangeFlagSet(Uniform::ValueChanged))
			{
				switch (uniform->type())
				{
					case Uniform::FloatUniform:
					glUniform1fv(uniform->getId(this), uniform->size(), (GLfloat*) uniform->data());
						break;

					case Uniform::FloatVec2Uniform:
					glUniform2fv(uniform->getId(this), uniform->size(), (GLfloat*) uniform->data());
						break;

					case Uniform::FloatVec3Uniform:
					glUniform3fv(uniform->getId(this), uniform->size(), (GLfloat*) uniform->data());
						break;
					case Uniform::FloatVec4Uniform:
					glUniform4fv(uniform->getId(this), uniform->size(), (GLfloat*) uniform->data());
						break;


					case Uniform::IntUniform:
					glUniform1iv(uniform->getId(this), uniform->size(), (GLint*) uniform->data());
						break;

					case Uniform::IntVec2Uniform:
					glUniform2iv(uniform->getId(this), uniform->size(), (GLint*) uniform->data());
						break;

					case Uniform::IntVec3Uniform:
					glUniform3iv(uniform->getId(this), uniform->size(), (GLint*) uniform->data());
						break;

					case Uniform::IntVec4Uniform:
					glUniform4iv(uniform->getId(this), uniform->size(), (GLint*) uniform->data());
						break;


					case Uniform::Mat4x4Uniform:
					glUniformMatrix4fv(uniform->getId(this), uniform->size(), GL_FALSE, (GLfloat*) uniform->data());
						break;


					default:
					CoreEngine::instance()->log() << "Unsupported Uniform Type: " << uniform->type() << std::endl;
						break;

				}

				uniform->clearChangeFlags();
			}

		}
	}

	void RenderEngine::linkProgram(Program& program, std::string& log)
	{
		ResourceId resource_id = program.getId(this);

		glLinkProgram(resource_id);

		GLint linked = GL_FALSE;
		glGetProgramiv(resource_id, GL_LINK_STATUS, &linked);
		std::stringstream shader_log;
		if (!linked)
		{
			shader_log << "Program Link Failed" << std::endl;
		}

		Int infolog_length = 0;
		glGetProgramiv(resource_id, GL_INFO_LOG_LENGTH, &infolog_length);
		if (infolog_length > 0)
		{
			Int chars_written = 0;
			Char *info_log;

			info_log = new Char[infolog_length + 1];
			glGetProgramInfoLog(resource_id, infolog_length, &chars_written, info_log);
			std::string info(info_log);
			delete[] (info_log);

			trim(info);
			if (!info.empty())
			{
				shader_log << "Info Log :" << std::endl << info << std::endl;
			}
		}

		log = shader_log.str();
	}

	void RenderEngine::loadShader(Shader& shader, std::string& log)
	{
		Char const* source_as_char= shader.source().c_str();

		ResourceId resource_id = glCreateShader( GLenum(shader.type()) );

		glShaderSource(resource_id, 1, &source_as_char, 0);
		glCompileShader(resource_id);

		GLint compiled = GL_FALSE;
		glGetShaderiv(resource_id, GL_COMPILE_STATUS, &compiled);

		std::stringstream shader_log;
		if (!compiled)
		{
			std::string type_str = "Vertex";
			if (shader.type() == Shader::Fragment)
			{
				type_str = "Fragment";
			}

			shader_log << type_str << " Shader Compilation Failed" << std::endl;
		}

		Int infolog_length = 0;
		glGetShaderiv(resource_id, GL_INFO_LOG_LENGTH, &infolog_length);
	    if (infolog_length > 0)
	    {
		    Int chars_written = 0;
		    Char *info_log;

	        info_log = new Char[infolog_length + 1];
	        glGetShaderInfoLog(resource_id, infolog_length, &chars_written, info_log);
	        std::string info(info_log);
	        delete[](info_log);

	        trim(info);

	        if (!info.empty())
	        {
				std::string type_str = "Vertex";
				if (shader.type() == Shader::Fragment)
				{
					type_str = "Fragment";
				}
				shader_log << type_str << " Info Log :" << std::endl << info << std::endl;
	        }
	    }

	    shader.setId(resource_id, this);

		log = shader_log.str();
		if (!compiled)
		{

			unloadShader(shader);
		}
	}

	void RenderEngine::unloadShader(Shader& shader)
	{
		if ( shader.drawResourceLoaded(this) )
		{
			glDeleteShader( shader.getId(this) );
			shader.setId(0, this);
		}
	}

	void RenderEngine::unloadProgram(Program& program)
	{
		ResourceId resource_id = program.getId(this);

		if (resource_id)
		{
			GLint count = 0;
			GLsizei const max_count = 10;
			GLuint shaders[max_count];
			glGetAttachedShaders(resource_id, max_count, &count, &shaders[0]);


			for (GLsizei i = 0; i != count; ++i)
			{
				glDetachShader(resource_id, shaders[i]);
			}

			glDeleteProgram(resource_id);
			glUseProgram(0);
			program.setId(0, this);
		}
	}

	void RenderEngine::displayShaderLog(Shader& shader, std::stringstream& shader_log, std::string& info)
	{
		shader_log << "--- Shader begin --- " << std::endl;
		Uint line_number = 1;

		std::stringstream string_stream;
		string_stream << shader.source();
		std::string line;
		while(std::getline(string_stream, line))
		{
			shader_log << std::setw(4) << std::setfill('0') << line_number << "  " << line << std::endl;
			++line_number;
		}

		shader_log << "--- Shader end --- " << std::endl;
		shader_log << info << std::endl;
	}

	void RenderEngine::displayProgramLog(Program& program, std::string const& log)
	{
		std::string location = CoreEngine::instance()->resourceManager().resourceLocation<Program>(&program);
		RLOG_ERROR(log_name, "Log for effect [" << location << "]" << std::endl << log);
	}

	void RenderEngine::reportProgram(Program& program)
	{
		std::stringstream output;

		std::string location = CoreEngine::instance()->resourceManager().resourceLocation<Program>(&program);
		output << "Report for effect [" << location << "]" << std::endl;

		if (!program.inputs().empty())
		{
			output << "---   Inputs    --- " << std::endl;
			for (Program::Connections::size_type i = 0; i != program.inputs().size(); ++i)
			{
				Program::Connection const& connection = program.inputs()[i];

				output << "[" << connection.id << "] " << connection.type << " | " << connection.name << " | " << VertexBuffer::semanticToString( connection.semantic ) << std::endl;
			}
		}

		if (!program.outputs().empty())
		{
			output << "---  Outputs    --- " << std::endl;
			for (Program::Connections::size_type i = 0; i != program.outputs().size(); ++i)
			{
				Program::Connection const& connection = program.outputs()[i];

				output << "[" << connection.id << "] " << connection.type << " | " << connection.name << " | " << VertexBuffer::semanticToString( connection.semantic ) << std::endl;
			}
		}

		std::string info;

		if (program.getShader(Shader::Vertex))
		{
			output << "---   Vertex    --- " << std::endl;
			displayShaderLog(*program.getShader(Shader::Vertex), output, info);
		}

		if (program.getShader(Shader::Fragment))
		{
			output << "---  Fragment   --- " << std::endl;
			displayShaderLog(*program.getShader(Shader::Fragment), output, info);
		}


		RLOG_INFO(log_name, output.str() << std::endl);

	}

	//
	// Vertex Buffer Object
	//

	void RenderEngine::loadVertexBufferObject(VertexBufferObject& vertex_buffer_object, VertexBuffer const& vertex_buffer, Drawable::DrawMode const mode)
	{
		ResourceId resource_id = vertex_buffer_object.getId(this);

		if (resource_id == 0)
		{
			glGenBuffers(1, &resource_id);
			vertex_buffer_object.setId(resource_id, this);
		}

		glBindBuffer(GL_ARRAY_BUFFER, resource_id);

		//glBufferData(GL_ARRAY_BUFFER, vertex_buffer.vertexSize() * vertex_buffer.size(), NULL, GL_STATIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertex_buffer.vertexSize() * vertex_buffer.size(), vertex_buffer.data(), mode);
	}

	void RenderEngine::bindVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer)
	{
		//TODO: performance improvement needed
		ResourceId resource_id = vertex_buffer_object.getId(this);
		glBindBuffer(GL_ARRAY_BUFFER, resource_id);

		if (!implementation->program || !implementation->program->hasInputSemantics())
		{
			for (VertexBuffer::Channels::size_type i = 0; i != vertex_buffer.channels().size(); ++i )
			{
				glVertexAttribPointer(i, vertex_buffer.channels()[i].number_of_components,
						                 GL_FLOAT, GL_FALSE, vertex_buffer.vertexSize(),
						                 VertexBuffer::DataPointer(0) + vertex_buffer.channels()[i].offset);
				glEnableVertexAttribArray(i);
			}

			return;
		}

		// semantic binding
		Program* program = implementation->program.get();

		// mark all channels as available
		implementation->channel_input_cache.clear();
		implementation->channel_input_cache.resize(vertex_buffer.channels().size());

		// bind channels with semantics
		for (Program::Connections::const_iterator connection = program->inputs().begin(); connection != program->inputs().end(); ++connection)
		{
			if (connection->semantic != VertexBuffer::None)
			{
				for (VertexBuffer::Channels::size_type i = 0; i != vertex_buffer.channels().size(); ++i )
				{
					VertexBuffer::Channel const& channel = vertex_buffer.channels()[i];

					if ( implementation->channel_input_cache[i].channel_available && (connection->id >= 0) && (channel.semantic == connection->semantic) )
					{
						 implementation->channel_input_cache[i].channel_available = false;
						 if (connection->id < Int( vertex_buffer.channels().size() ))
						 {
							 implementation->channel_input_cache[connection->id].input_available = false;
						 }


						glVertexAttribPointer(connection->id, channel.number_of_components, GL_FLOAT, GL_FALSE, vertex_buffer.vertexSize(), VertexBuffer::DataPointer(0) + channel.offset);
						glEnableVertexAttribArray(connection->id);

						break;
					}
				}
			}
		}

		// bind the remaining channels
		for (VertexBuffer::Channels::size_type i = 0; i != vertex_buffer.channels().size(); ++i )
		{
			if ( implementation->channel_input_cache[i].channel_available )
			{
				VertexBuffer::Channel const& channel = vertex_buffer.channels()[i];
				implementation->channel_input_cache[i].channel_available = false;

				Int input_available = -1;


				// find next input available
				for (VertexBuffer::Channels::size_type j = 0; j != vertex_buffer.channels().size(); ++j )
				{
					if ( implementation->channel_input_cache[j].input_available )
					{
						input_available = j;
						break;
					}
				}

				RENGINE_ASSERT(input_available >= 0);

				implementation->channel_input_cache[input_available].input_available = false;

				glVertexAttribPointer(input_available, channel.number_of_components, GL_FLOAT, GL_FALSE, vertex_buffer.vertexSize(), VertexBuffer::DataPointer(0) + channel.offset);
				glEnableVertexAttribArray(input_available);
			}
		}


	}

	void RenderEngine::unbindVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer)
	{
		for (VertexBuffer::Channels::size_type i = 0; i != vertex_buffer.channels().size(); ++i)
		{
			glDisableVertexAttribArray(i);
		}
	}

	void RenderEngine::bindIndexBufferObject(VertexBufferObject const& vertex_buffer_object, Drawable::IndexVector const& index_buffer)
	{
		ResourceId resource_id = vertex_buffer_object.getId(this);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resource_id);
	}

	void RenderEngine::unbindIndexBufferObject(VertexBufferObject const& vertex_buffer_object, Drawable::IndexVector const& index_buffer)
	{

	}

	void RenderEngine::drawVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer)
	{
		glDrawArrays(GL_TRIANGLES, 0, vertex_buffer.size());
	}

	void RenderEngine::drawVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer, VertexBufferObject const& index_vertex_buffer_object, Drawable::IndexVector const& index_buffer)
	{
		glDrawElements(GL_TRIANGLES, index_buffer.size(), GL_UNSIGNED_INT, VertexBuffer::DataPointer(0));
	}

	void RenderEngine::loadIndexBufferObject(VertexBufferObject& vertex_buffer_object, Drawable::IndexVector const& index_buffer, Drawable::DrawMode const mode)
	{
		ResourceId resource_id = vertex_buffer_object.getId(this);

		if (resource_id == 0)
		{
			glGenBuffers(1, &resource_id);
			vertex_buffer_object.setId(resource_id, this);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, resource_id);

		//glBufferData(GL_ARRAY_BUFFER, vertex_buffer.vertexSize() * vertex_buffer.size(), NULL, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Drawable::IndexType) * index_buffer.size(), &index_buffer[0], mode);

	}

	void RenderEngine::unloadVertexBufferObject(VertexBufferObject& vertex_buffer_object)
	{
		if (vertex_buffer_object.getId(this) != 0)
		{
			glDeleteBuffers(1, &vertex_buffer_object.getId(this));
			vertex_buffer_object.setId(0, this);
		}
	}

	//
	// FrameBufferObject Handling
	// RenderBuffer Handling
	//
	void RenderEngine::bind(RenderBuffer& render_buffer)
	{
		ResourceId resource_id = render_buffer.getId(this);

		if (resource_id == 0)
		{
			glGenRenderbuffers(1, &resource_id);
			render_buffer.setId(resource_id, this);

			glBindRenderbuffer(GL_RENDERBUFFER, resource_id);

			if (render_buffer.samples() == RenderBuffer::NotDefined || render_buffer.samples() == 0)
			{
				glRenderbufferStorage(GL_RENDERBUFFER, render_buffer.internalFormat(), render_buffer.width(), render_buffer.height());
			}
			else
			{
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, render_buffer.samples(), render_buffer.internalFormat(), render_buffer.width(), render_buffer.height());
			}
		}

		glBindRenderbuffer(GL_RENDERBUFFER, resource_id);
	}

	void RenderEngine::unloadRenderBuffer(RenderBuffer& render_buffer)
	{
		if (render_buffer.getId(this) != 0)
		{
			glDeleteRenderbuffers(1, &render_buffer.getId(this));
			render_buffer.setId(0, this);
		}
	}

	void RenderEngine::unloadFrameBuffer(FrameBuffer& frame_buffer)
	{
		if (frame_buffer.getId(this) != 0)
		{
			glDeleteFramebuffers(1, &frame_buffer.getId(this));
			frame_buffer.setId(0, this);
		}
	}

	void RenderEngine::bind(FrameBuffer& frame_buffer)
	{
		ResourceId resource_id = frame_buffer.getId(this);

		if (resource_id == 0)
		{
			glGenFramebuffers(1, &resource_id);
			frame_buffer.setId(resource_id, this);

			// create the frame buffer
			glBindFramebuffer(GL_FRAMEBUFFER_EXT, resource_id);

			GLenum static color_buffers[] = { GL_COLOR_ATTACHMENT0 , GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 , GL_COLOR_ATTACHMENT3 ,
					                          GL_COLOR_ATTACHMENT4 , GL_COLOR_ATTACHMENT5 , GL_COLOR_ATTACHMENT6 , GL_COLOR_ATTACHMENT7 ,
					                          GL_COLOR_ATTACHMENT8 , GL_COLOR_ATTACHMENT9 , GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
					                          GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15
							                };

			Uint color_attachments = 0;

			//attach elements
			for (FrameBuffer::Attachments::size_type i = 0; i != frame_buffer.attachments().size(); ++i)
			{
				FrameBuffer::Attachment& attachment = frame_buffer.attachments()[i];
				Uint attachment_target = Uint( attachment.target );

				if (attachment_target == FrameBuffer::Color)
				{
					attachment_target += color_attachments++;
				}

				if (attachment.type == FrameBuffer::TextureAttachment)
				{
					apply(*attachment.texture);
					glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, attachment_target, GL_TEXTURE_2D, attachment.texture->getId(this), 0);
				}
				else
				{
					bind(*attachment.render_buffer);
					glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, attachment_target, GL_RENDERBUFFER_EXT, attachment.render_buffer->getId(this));
				}
			}

			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if (status != GL_FRAMEBUFFER_COMPLETE)
			{
				std::string log;

				switch (status)
				{
//					case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
//					log = "Not all attached images have the same width and height.";
//						break;
//					case GL_FRAMEBUFFER_INCOMPLETE_UNSUPPORTED:
//					log = "The combination of internal formats of the attached images violates an implementation-dependent set of restrictions.";
//						break;
					case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
					log = "Not all framebuffer attachment points are framebuffer attachment complete. ";
						break;
					case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
					log = "No images are attached to the framebuffer.";
						break;
					default:
					log = "Unknown error on Framebuffer binding.";
					break;
				}

				CoreEngine::instance()->log() << log << std::endl;
			}

			if (color_attachments > 0)
			{
				glDrawBuffers(color_attachments, color_buffers);
			}

		}

		glBindFramebuffer(GL_FRAMEBUFFER_EXT, resource_id);
	}

	void RenderEngine::unbind(FrameBuffer& frame_buffer)
	{
		glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	}

	void RenderEngine::blit(FrameBuffer& src, FrameBuffer& dest, Buffer const& buffer, BlitFilter const& filter)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, src.getId(this));
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, dest.getId(this));

		glBlitFramebuffer(0, 0,  src.width(), src.height(),
				          0, 0, dest.width(), dest.height(),
				          buffer, filter);


		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
	}

	void RenderEngine::blitFrom(FrameBuffer& src, Buffer const& buffer, BlitFilter const& filter)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, src.getId(this));
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);

		glBlitFramebuffer(0, 0, src.width(), src.height(),
				          0, 0, src.width(), src.height(),
				          buffer, filter);

		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
	}

	void RenderEngine::blitTo(FrameBuffer& dest, Buffer const& buffer, BlitFilter const& filter)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, dest.getId(this));

		glBlitFramebuffer(0, 0, dest.width(), dest.height(),
				          0, 0, dest.width(), dest.height(),
				          buffer, filter);

		glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
	}




	//
	// Deprecated Shit
	//

//	void RenderEngine::bindVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer)
//	{
//		ResourceId resource_id = vertex_buffer_object.getId(this);
//
//		glBindBuffer(GL_ARRAY_BUFFER, resource_id);
//
//		for (VertexBuffer::Channels::size_type i = 0; i != vertex_buffer.channels().size(); ++i )
//		{
//			VertexBuffer::Channel const& channel = vertex_buffer.channels()[i];
//
//			switch (channel.smantic)
//			{
//				case VertexBuffer::Position:
//				{
//					glEnableClientState(GL_VERTEX_ARRAY);
//					glVertexPointer(channel.number_of_components, GL_FLOAT, vertex_buffer.vertexSize(), VertexBuffer::DataPointer(0) + channel.offset);
//
//				}
//					break;
//				case VertexBuffer::Color:
//				{
//					glEnableClientState(GL_COLOR_ARRAY);
//					glColorPointer(channel.number_of_components, GL_FLOAT, vertex_buffer.vertexSize(), VertexBuffer::DataPointer(0) + channel.offset);
//				}
//					break;
//				case VertexBuffer::Normal:
//				{
//					glEnableClientState(GL_NORMAL_ARRAY);
//					glNormalPointer(GL_FLOAT, vertex_buffer.vertexSize(), VertexBuffer::DataPointer(0) + channel.offset);
//				}
//					break;
//				case VertexBuffer::TexCoords:
//				{
//					// texture coods is not fully supported
//					glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//					glClientActiveTexture(GL_TEXTURE0);
//					glTexCoordPointer(channel.number_of_components, GL_FLOAT, vertex_buffer.vertexSize(), VertexBuffer::DataPointer(0) + channel.offset);
//				}
//					break;
//				default:
//					break;
//
//			}
//		}
//	}
//
//	void RenderEngine::unbindVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer)
//	{
//		for (VertexBuffer::Channels::size_type i = 0; i != vertex_buffer.channels().size(); ++i)
//		{
//			VertexBuffer::Channel const& channel = vertex_buffer.channels()[i];
//
//			switch (channel.smantic)
//			{
//				case VertexBuffer::Position:
//				{
//					glDisableClientState(GL_VERTEX_ARRAY);
//				}
//					break;
//				case VertexBuffer::Color:
//				{
//					glDisableClientState(GL_COLOR_ARRAY);
//				}
//					break;
//				case VertexBuffer::Normal:
//				{
//					glDisableClientState(GL_NORMAL_ARRAY);
//				}
//					break;
//				case VertexBuffer::TexCoords:
//				{
//					glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//				}
//					break;
//				default:
//					break;
//			}
//		}
//	}
//
//	void RenderEngine::renderLine(Vector3D const& origin, Vector3D const& destination, Vector4D const& color, Real const line_width)
//	{
//		if (glIsEnabled(GL_TEXTURE_2D))
//		{
//			glDisable(GL_TEXTURE_2D);
//		}
//
//		glColor4fv(color.ptr());
//		glLineWidth(line_width);
//
//		glBegin(GL_LINES);
//			glVertex3fv(origin.ptr());
//			glVertex3fv(destination.ptr());
//		glEnd();
//	}
//
//	void RenderEngine::renderBoundingBox(BoundingBox const& bounding_box, Bool flip_normals)
//	{
//
//		Real flip_normals_factor = (flip_normals ? -1.0f : 1.0f);
//
//		glBegin(GL_QUADS);
//
//		// bottom
//		glNormal3f(0.0f, -1.0f * flip_normals_factor, 0.0f);
//		glVertex3fv(bounding_box.corner(4).ptr());
//		glVertex3fv(bounding_box.corner(5).ptr());
//		glVertex3fv(bounding_box.corner(1).ptr());
//		glVertex3fv(bounding_box.corner(0).ptr());
//
//		// top
//		glNormal3f(0.0f, 1.0f * flip_normals_factor, 0.0f);
//		glVertex3fv(bounding_box.corner(2).ptr());
//		glVertex3fv(bounding_box.corner(3).ptr());
//		glVertex3fv(bounding_box.corner(7).ptr());
//		glVertex3fv(bounding_box.corner(6).ptr());
//
//		// left
//		glNormal3f(-1.0f * flip_normals_factor, 0.0f, 0.0f);
//		glVertex3fv(bounding_box.corner(0).ptr());
//		glVertex3fv(bounding_box.corner(2).ptr());
//		glVertex3fv(bounding_box.corner(6).ptr());
//		glVertex3fv(bounding_box.corner(4).ptr());
//
//		// right
//		glNormal3f(1.0f * flip_normals_factor, 0.0f, 0.0f);
//		glVertex3fv(bounding_box.corner(1).ptr());
//		glVertex3fv(bounding_box.corner(5).ptr());
//		glVertex3fv(bounding_box.corner(7).ptr());
//		glVertex3fv(bounding_box.corner(3).ptr());
//
//		// front
//		glNormal3f(0.0f, 0.0f, -1.0f * flip_normals_factor);
//		glVertex3fv(bounding_box.corner(0).ptr());
//		glVertex3fv(bounding_box.corner(1).ptr());
//		glVertex3fv(bounding_box.corner(3).ptr());
//		glVertex3fv(bounding_box.corner(2).ptr());
//
//		// back
//		glNormal3f(0.0f, 0.0f, 1.0f * flip_normals_factor);
//		glVertex3fv(bounding_box.corner(4).ptr());
//		glVertex3fv(bounding_box.corner(6).ptr());
//		glVertex3fv(bounding_box.corner(7).ptr());
//		glVertex3fv(bounding_box.corner(5).ptr());
//
//		glEnd();
//	}
//
//
//
//	void RenderEngine::renderAxis(Real lenght)
//	{
//		// draw a x-axis with cone at end
//		glColor3f(1.0f, 0.0f, 0.0f);
//		glPushMatrix();
//		glRotatef(90.0f, 0.0f, 1.0f, 0.0f); // Z->X
//		glBegin(GL_LINES);
//		glVertex3f(0.0f, 0.0f, 0.0f);
//		glVertex3f(0.0f, 0.0f, lenght);
//		glEnd();
//		glTranslatef(0.0f, 0.0f, lenght - 0.2f);
//		//  glutWireCone( 0.04f, 0.2f, 12, 9 );
//		glPopMatrix();
//
//		// draw a y-axis with cone at end
//		glColor3f(0.0f, 1.0f, 0.0f);
//		glPushMatrix();
//		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); // Z->Y
//		glBegin(GL_LINES);
//		glVertex3f(0.0f, 0.0f, 0.0f);
//		glVertex3f(0.0f, 0.0f, lenght);
//		glEnd();
//		glTranslatef(0.0f, 0.0f, lenght - 0.2f);
//		//  glutWireCone( 0.04f, 0.2f, 12, 9 );
//		glPopMatrix();
//
//		// draw a z-axis with cone at end
//		glColor3f(0.0f, 0.0f, 1.0f);
//		glPushMatrix();
//		glBegin(GL_LINES);
//		glVertex3f(0.0f, 0.0f, 0.0f);
//		glVertex3f(0.0f, 0.0f, lenght);
//		glEnd();
//		glTranslatef(0.0f, 0.0f, lenght - 0.2f);
//		//  glutWireCone( 0.04f, 0.2f, 12, 9 );
//		glPopMatrix();
//	}
//
//void RenderEngine::renderInfinitePlane( Plane const& plane, Vec3 const &point, Real const lenght )
//{
// //   Vec3 projected_point = plane.pointProjection( CoreEngine::instance()->getCamera()->position() );
//    Vec3 projected_point = plane.pointProjection( point );
//
//
//    Quat rotation;
//    rotation.makeRotate( Vec3d( 0.0f, 0.0f, 1.0f ), plane.getNormal() );
//
//    Vec3 point_1 = rotation * Vec3(-lenght, -lenght, 0.0f);
//    Vec3 point_2 = rotation * Vec3(lenght, -lenght, 0.0f);
//    Vec3 point_3 = rotation * Vec3(lenght, lenght, 0.0f);
//    Vec3 point_4 = rotation * Vec3(-lenght, lenght, 0.0f);
//
//    glPushMatrix();
//
//    glTranslatef( projected_point.x(), projected_point.y(), projected_point.z() );
//    glBegin( GL_QUADS );
//    glVertex3fv(point_1.ptr());
//    glVertex3fv(point_2.ptr());
//    glVertex3fv(point_3.ptr());
//    glVertex3fv(point_4.ptr());
//    glEnd();
//
//    glPopMatrix();
//}
//	Vector4D RenderEngine::randomColor() const
//	{
//		Vector4D color;
//
//		Uchar red = rand() % 256;
//		Uchar green = rand() % 256;
//		Uchar blue = rand() % 256;
//
//		color.x() = Real(red) / 256.0f;
//		color.y() = Real(green) / 256.0f;
//		color.z() = Real(blue) / 256.0f;
//		color.w() = 0.1f;
//
//		return color;
//	}

//	ResourceId RenderEngine::uploadTexture2D(void *pixels, Int width, Int height, Int color_channels, Bool compress, Bool mipmaps, Bool filtering, Bool repeate, ResourceId id)
//	{
//
//		Bool const trilinear_filtering = false;
//		Real const maximum_anisotropy = 1.0f;
//
//		if (id == 0)
//		{
//			glGenTextures(1, &id);
//		}
//
//		glBindTexture(GL_TEXTURE_2D, id);
//		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering ? GL_LINEAR : GL_NEAREST );
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeate ? GL_REPEAT : GL_CLAMP_TO_EDGE );
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeate ? GL_REPEAT : GL_CLAMP_TO_EDGE );
//
//		if (mipmaps)
//		{
//			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maximum_anisotropy);
//			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
//
//			if (trilinear_filtering)
//			{
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//			}
//			else
//			{
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
//			}
//		}
//		else
//		{
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_NEAREST );
//		}
//
//		Int format = getTextureFormatFromChannels(color_channels);
//		Int type = GL_UNSIGNED_BYTE;
//		Int internal_format = GL_RGBA8;
//
//		switch (color_channels)
//		{
//			case 1:
//			internal_format = compress ? GL_COMPRESSED_LUMINANCE_ARB : GL_LUMINANCE8;
//				break;
//			case 2:
//			internal_format = compress ? GL_COMPRESSED_LUMINANCE_ALPHA_ARB : GL_LUMINANCE8_ALPHA8;
//				break;
//			case 3:
//			internal_format = compress ? GL_COMPRESSED_RGB_ARB : GL_RGB8; //GL_COMPRESSED_RGB_S3TC_DXT1_EXT
//				break;
//			case 4:
//			internal_format = compress ? GL_COMPRESSED_RGBA_ARB : GL_RGBA8; //GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
//				break;
//		};
//
//		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, pixels);
//		glBindTexture(GL_TEXTURE_2D, 0);
//
//		return id;
//	}
//
//	void RenderEngine::updateTexture2D(void *pixels, Int width, Int height, Int color_channels, Bool filtering, ResourceId id)
//	{
//		glBindTexture(GL_TEXTURE_2D, id);
//
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering ? GL_LINEAR : GL_NEAREST );
//		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
//
//		Int type = GL_UNSIGNED_BYTE;
//		Int format = getTextureFormatFromChannels(color_channels);
//
//		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, type, pixels);
//		glBindTexture(GL_TEXTURE_2D, 0);
//	}

//	void RenderEngine::renderMesh(Mesh & mesh, Drawable::DrawMode const draw_mode)
//	{
//		if (draw_mode == Drawable::StaticDraw ||
//			draw_mode == Drawable::DynamicDraw ||
//			draw_mode == Drawable::StreamDraw)
//		{
//
//		}
//		else if (draw_mode == Drawable::ImmediateDraw)
//		{
//			Mesh::IndexType index;
//
//			glBegin(GL_TRIANGLES);
//			for (uint triangle = 0; triangle < mesh.numberOfFaces(); triangle++)
//			{
//				//if (data.meshes[ii].texCoords)
//				//{
//				//	glMultiTexCoord2fvARB( GL_TEXTURE0_ARB, data.meshes[ii].texCoords[vert]);
//				//	glMultiTexCoord3fvARB( GL_TEXTURE1_ARB, &(data.meshes[ii].tangentSpace[gg*3 + 0][0]));
//				//	glMultiTexCoord3fvARB( GL_TEXTURE2_ARB, &(data.meshes[ii].tangentSpace[gg*3 + 0][3]));
//				//}
//
//				index = mesh.index()[triangle * 3 + 0];
//				if (mesh.color().size() > index)
//				{
//					glColor4fv(mesh.color()[index].ptr());
//				}
//				for (uint texture = 0; texture != mesh.numberOfTextureArrays(); ++texture)
//				{
//					glMultiTexCoord2fv(GL_TEXTURE0 + texture, mesh.textureCoordinate(texture)[index].ptr());
//				}
//				glNormal3fv(mesh.normal()[index].ptr());
//				glVertex3fv(mesh.vertex()[index].ptr());
//
//
//				index = mesh.index()[triangle * 3 + 1];
//				if (mesh.color().size() > index)
//				{
//					glColor4fv(mesh.color()[index].ptr());
//				}
//				for (uint texture = 0; texture != mesh.numberOfTextureArrays(); ++texture)
//				{
//					glMultiTexCoord2fv(GL_TEXTURE0 + texture, mesh.textureCoordinate(texture)[index].ptr());
//				}
//				glNormal3fv(mesh.normal()[index].ptr());
//				glVertex3fv(mesh.vertex()[index].ptr());
//
//
//				index = mesh.index()[triangle * 3 + 2];
//				if (mesh.color().size() > index)
//				{
//					glColor4fv(mesh.color()[index].ptr());
//				}
//				for (uint texture = 0; texture != mesh.numberOfTextureArrays(); ++texture)
//				{
//					glMultiTexCoord2fv(GL_TEXTURE0 + texture, mesh.textureCoordinate(texture)[index].ptr());
//				}
//				glNormal3fv(mesh.normal()[index].ptr());
//				glVertex3fv(mesh.vertex()[index].ptr());
//			}
//
//			glEnd();
//
//			// restore the previous flat color if needed
//			if (implementation->draw_states->hasState(DrawStates::FlatColor) && (mesh.color().size() > 0))
//			{
//				FlatColor* flat_color = dynamic_cast<FlatColor*>( implementation->draw_states->getState(DrawStates::FlatColor).first.get() );
//
//				glColor4fv(flat_color->get().ptr());
//			}
//		}
//		else if (draw_mode == Drawable::DisplayListDraw)
//		{
//			glCallList(mesh.displayListId());
//		}
//		else if (draw_mode == Drawable::ArraysDraw)
//		{
//			glEnableClientState(GL_VERTEX_ARRAY);
//			glVertexPointer(3, GL_FLOAT, 0, mesh.vertexArray());
//
//			if (mesh.color().size())
//			{
//				glEnableClientState(GL_COLOR_ARRAY);
//				glColorPointer(4, GL_FLOAT, 0, mesh.colorArray());
//			}
//
//			if (mesh.normal().size())
//			{
//				glEnableClientState(GL_NORMAL_ARRAY);
//				glNormalPointer(GL_FLOAT, 0, mesh.normalArray());
//			}
//
//			// TODO:please finish me!!! implement texture draw arrays;
//
//			glDrawElements(GL_TRIANGLES, mesh.numberOfIndexes(), GL_UNSIGNED_INT, mesh.indexArray());
//
//			if (mesh.color().size())
//			{
//				glDisableClientState(GL_COLOR_ARRAY);
//			}
//
//			if (mesh.normal().size())
//			{
//				glDisableClientState(GL_NORMAL_ARRAY);
//			}
//			glDisableClientState(GL_VERTEX_ARRAY);
//		}
//
//	}

} // namespace rengine

