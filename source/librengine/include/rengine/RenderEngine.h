// __!!rengine_copyright!!__ //

#ifndef __RENGINE_RENDERENGINE_H__
#define __RENGINE_RENDERENGINE_H__

#include <rengine/geometry/Model.h>
#include <rengine/math/Matrix.h>
#include <rengine/math/BoundingVolume.h>
#include <rengine/math/Vector.h>

#include <rengine/state/DrawStates.h>

namespace rengine
{
	class Drawable;

	class VertexBuffer;
	class VertexBufferObject;
	class RenderBuffer;
	class FrameBuffer;

	class DrawStates;
	class BlendFunction;
	class BlendEquation;
	class BlendColor;
	class CullFace;
	class Depth;
	class AlphaFunction;
	class ColorChannelMask;
	class PolygonMode;
	class Stencil;
	class OperationBuffer;
	class Texture2D;
	class Texture2DUnit;
	class ProgramUnit;
	class Program;
	class Shader;


	class RenderEngine
	{
	public:
		enum Buffer
		{
			ColorBuffer		= 0x00004000,
			DepthBuffer		= 0x00000100,
			StencilBuffer	= 0x00000400
		};

		enum BlitFilter
		{
			Nearest			= 0x2600,
			Linear			= 0x2601
		};

		typedef Uint BufferType;


		RenderEngine();
		~RenderEngine();

		void init();
		void shutdown();

		void preFrame();
		void postFrame();

		void setModelView(Matrix const& matrix);
		Matrix const& modelView() const;
		Matrix& modelView();
		void pushModelView();
		void popModelView();

		void setProjection(Matrix const& matrix);
		Matrix const& projection() const;
		Matrix& projection();
		void pushProjection();
		void popProjection();

		void setViewport(Uint const x, Uint const y, Uint const width, Uint const height);
		Uint getViewportX() const;
		Uint getViewportY() const;
		Uint getViewportWidth() const;
		Uint getViewportHeight() const;

		//
		// Base Buffer Management
		//
		void clearBuffers(BufferType const& buffers = (ColorBuffer | DepthBuffer));

		void setClearColor(Vector4D const& color);
		Vector4D const& getClearColor() const;

		void setClearDepth(Real64 const value);
		Real64 getClearDepth() const;

		void setClearStencil(Int const value);
		Int getClearStencil() const;



		//
		// StateMachine
		//
		DrawStates const& getDrawStates() const;
		void clearDrawStates();
		void pushDrawStates();
		void popDrawStates();
		void reportDrawStates();

		void apply(DrawStates const& states);
		void applyState(DrawStates::State const& state, DrawStates::Value const& value);
		void applyCapability(DrawStates::Capability const& capability, DrawStates::Value const& value);

		//double dispatch functions
		void apply(BlendFunction const& state);
		void apply(BlendEquation const& state);
		void apply(BlendColor const& state);
		void apply(CullFace const& state);
		void apply(Depth const& state);
		void apply(AlphaFunction const& state);
		void apply(ColorChannelMask const& state);
		void apply(PolygonMode const& state);
		void apply(Stencil const& state);
		void apply(OperationBuffer const& state);
		void apply(Texture2DUnit const& state);
		void apply(ProgramUnit const& state);

		// Called by applyState
		void applyTextures(DrawStates::StateVector const& textures, Bool const& value);

		//
		// Drawables
		//

		// this method applies the drawable state and calls render
		void draw(Drawable& drawable);

		//
		// Texture handling
		//
		void apply(Texture2D& texture);
		Int getTextureFormatFromChannels(Int color_channels);
		Bool supportsNonPowerOfTwoTextures() const;
		Bool supportsTextureRectangle() const;
		Uint maximumTextureSizeSupported() const;
		void unloadTexture(Texture2D& texture); // set id = 0;
		// returns RGBA pixel data. You must delete the pixel data!
		Uchar* downloadTexture2DData(Int color_channels, Int *width, Int *height, ResourceId id);

		//
		// Shader Handling
		//
		void apply(Program& program);
		void loadShader(Shader& shader, std::string& log);
		void unloadShader(Shader& shader);
		void unloadProgram(Program& program);
		void linkProgram(Program& program, std::string& log);
		void updateUniforms(Program& program);
		void displayShaderLog(Shader& shader, std::stringstream& shader_log, std::string& info);
		void displayProgramLog(Program& program, std::string const& log);
		void reportProgram(Program& program);

		//
		// VertexBufferObject Handling
		//
		void loadVertexBufferObject(VertexBufferObject& vertex_buffer_object, VertexBuffer const& vertex_buffer, Drawable::DrawMode const mode);
		void loadIndexBufferObject(VertexBufferObject& vertex_buffer_object, Drawable::IndexVector const& index_buffer, Drawable::DrawMode const mode);

		void bindVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer);
		void unbindVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer);

		void bindIndexBufferObject(VertexBufferObject const& vertex_buffer_object, Drawable::IndexVector const& index_buffer);
		void unbindIndexBufferObject(VertexBufferObject const& vertex_buffer_object, Drawable::IndexVector const& index_buffer);

		void drawVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer);
		void drawVertexBufferObject(VertexBufferObject const& vertex_buffer_object, VertexBuffer const& vertex_buffer, VertexBufferObject const& index_vertex_buffer_object, Drawable::IndexVector const& index_buffer);

		void unloadVertexBufferObject(VertexBufferObject& vertex_buffer_object);

		//
		// FrameBufferObject Handling
		// RenderBuffer Handling
		//
		void bind(RenderBuffer& render_buffer);
		void unloadRenderBuffer(RenderBuffer& render_buffer);

		void unloadFrameBuffer(FrameBuffer& frame_buffer);
		void bind(FrameBuffer& frame_buffer);
		void unbind(FrameBuffer& frame_buffer);

		void blit(FrameBuffer& src, FrameBuffer& dest, Buffer const& buffer, BlitFilter const& filter = Nearest);
		void blitFrom(FrameBuffer& src, Buffer const& buffer, BlitFilter const& filter = Nearest); 	// blits to main frame buffer
		void blitTo(FrameBuffer& dest, Buffer const& buffer, BlitFilter const& filter = Nearest); // blits from main frame buffer

		static Bool checkErrors(std::string const &message = "");

		//
		// Info Methods
		//
		std::string vendor() const;
		std::string renderer() const;
		std::string version() const;
		std::string shadingLanguageVersion() const;

		Bool limitedToOpenGL21() const;
	private:
		RenderEngine(RenderEngine const& copy);

		struct PrivateImplementation;
		PrivateImplementation *implementation;
	};


	//
	// implementation
	//
	RENGINE_INLINE RenderEngine::RenderEngine(RenderEngine const& copy)
	{
	}

} // end of namespace

#endif // __RENGINE__RENDERENGINE_H__
