// __!!rengine_copyright!!__ //

#ifndef __RENGINE_PROGRAM_H__
#define __RENGINE_PROGRAM_H__

#include <rengine/state/DrawStates.h>
#include <rengine/RenderEngine.h>
#include <rengine/math/Matrix.h>

#include <vector>

namespace rengine
{
	//
	// Shader
	//
	class Shader : public DrawResource
	{
	public:
		enum Type
		{
			Fragment 	= 0x8B30,
			Vertex 		= 0x8B31
		};

		Shader(Type const type);
		Shader(Type const type, std::string const source);
		~Shader();

		void release();

		void setSource(std::string const& source);
		std::string const& source() const;

		Type const& type() const;
	private:
		Type type_;
		std::string source_;
	};


#define RENGINE_UNIFORM_CONSTRUCTOR(DataType, UniformType) Uniform(std::string const& name, DataType const& value) \
								  :type_(UniformType), name_(name), size_(1), data_(0), program_(0) { initialize(); set(value); }

#define RENGINE_UNIFORM_SETTER(DataType) void set(DataType const& value, Uint const& element = 0) { this->memcpy(element, &value, sizeof(value)); flagChanged(); }

#define RENGINE_UNIFORM_TYPE(DataType, UniformType) \
		RENGINE_UNIFORM_CONSTRUCTOR(DataType, UniformType); \
		RENGINE_UNIFORM_SETTER(DataType);

	class Program;

	//
	// Uniform
	//
	class Uniform : public DrawResource
	{
	public:
		static const Uint component_mask	= 0x000000FF;
		static const Uint component_shift	= 8;

		//|Id[8]|Component size[8]|Number Of Componentes[8]|
		enum Type
		{
			FloatUniform 				= (0x0001 << (component_shift + component_shift)) | (sizeof(Real) 					<< component_shift) | 0x0001,
			FloatVec2Uniform 			= (0x0002 << (component_shift + component_shift)) | (sizeof(Vector2D::ValueType) 	<< component_shift) | 0x0002,
			FloatVec3Uniform 			= (0x0003 << (component_shift + component_shift)) | (sizeof(Vector3D::ValueType) 	<< component_shift) | 0x0003,
			FloatVec4Uniform 			= (0x0004 << (component_shift + component_shift)) | (sizeof(Vector4D::ValueType) 	<< component_shift) | 0x0004,

			IntUniform 					= (0x0005 << (component_shift + component_shift)) | (sizeof(Int) 					<< component_shift) | 0x0001,
			IntVec2Uniform 				= (0x0006 << (component_shift + component_shift)) | (sizeof(Vector2Di::ValueType)	<< component_shift) | 0x0002,
			IntVec3Uniform 				= (0x0007 << (component_shift + component_shift)) | (sizeof(Vector3Di::ValueType)	<< component_shift) | 0x0003,
			IntVec4Uniform 				= (0x0008 << (component_shift + component_shift)) | (sizeof(Vector4Di::ValueType)	<< component_shift) | 0x0004,

			Mat4x4Uniform				= (0x0009 << (component_shift + component_shift)) | (sizeof(Matrix44::ValueType)	<< component_shift) | 0x0010
		};

		enum Flag
		{
			ValueChanged				= 1,
			NotFound					= 2
		};

		typedef Uint SizeType;
		typedef Uchar DataType;

		~Uniform();

		Uniform(std::string const& name, Type const type, SizeType const size);

		RENGINE_UNIFORM_TYPE(Real, FloatUniform)
		RENGINE_UNIFORM_TYPE(Vector2D, FloatVec2Uniform)
		RENGINE_UNIFORM_TYPE(Vector3D, FloatVec3Uniform)
		RENGINE_UNIFORM_TYPE(Vector4D, FloatVec4Uniform)

		RENGINE_UNIFORM_TYPE(Int, IntUniform)
		RENGINE_UNIFORM_TYPE(Vector2Di, IntVec2Uniform)
		RENGINE_UNIFORM_TYPE(Vector3Di, IntVec3Uniform)
		RENGINE_UNIFORM_TYPE(Vector4Di, IntVec4Uniform)

		RENGINE_UNIFORM_TYPE(Matrix44, Mat4x4Uniform)

		void setName(std::string const& uniform_name);

		Type const& type() const;
		std::string const& name() const;

		void setProgram(Program* program);

		SizeType componentSize() const;
		SizeType components() const;
		SizeType const& size() const;
		DataType* data();

		static Type typeFromString(std::string name);
	private:
		void initialize();
		void memcpy(SizeType const& target, void const * src, SizeType const& n);
		void flagChanged();

		Type const type_;
		std::string const name_;
		SizeType const size_;
		DataType* data_;
		Program* program_;
		SizeType offset_;
	};

	//
	// Program
	//
	class Program : public DrawResource
	{
	public:
		struct Connection
		{
			Int id;
			std::string name;
			std::string type;
			VertexBuffer::Semantic semantic;
		};

		typedef std::vector< SharedPointer<Uniform> > Uniforms;
		typedef std::vector<Connection> Connections;

		enum Flag
		{
			None						= 0,
			UniformsChanged				= 1,
			VertexShaderChanged			= 2,
			FragmentShaderChanged		= 4
		};

		Program();
		Program(Program const& rhs);
		~Program();

		void release();

		void setShader(SharedPointer<Shader> shader);
		void removeShader(Shader::Type const& type);

		SharedPointer<Shader> const& getShader(Shader::Type const& type) const;
		SharedPointer<Shader>& getShader(Shader::Type const& type);

		void addUniform(SharedPointer<Uniform> uniform);
		Uniforms const& uniforms() const;
		Uniforms& uniforms();

		Uniform const& uniform(std::string const& name) const;
		Uniform& uniform(std::string const& name);

		Bool hasInputSemantics() const;
		void addInput(Connection const& input);
		Connections const& inputs() const;
		Connections& inputs();

		void addOutput(Connection const& output);
		Connections const& outputs() const;
		Connections& outputs();

		void prepareConnections();

		Bool hasUniform(std::string const& name) const;
	private:
		SharedPointer<Shader> vertex_shader;
		SharedPointer<Shader> fragment_shader;
		Uniforms uniforms_;
		Connections inputs_;
		Connections outputs_;
	};

	//
	// ProgramUnit
	//

	class ProgramUnit : public DrawStates::State
	{
	public:
		META_STATE_FUNCTIONS(ProgramUnit);
		META_STATE_HAS_OSTREAM_DECLARATION()

		ProgramUnit();
		ProgramUnit(SharedPointer<Program> const& program);
		ProgramUnit(ProgramUnit const& rhs);
		~ProgramUnit();

		virtual Int compare(DrawStates::State const& rhs) const;


		void set(SharedPointer<Program> const& program);

		SharedPointer<Program> const& get() const;
		SharedPointer<Program>& get();
	private:
		SharedPointer<Program> program_;
	};


	//
	// Implementation
	//


	//
	// Shader
	//
	RENGINE_INLINE Shader::Type const& Shader::type() const
	{
		return type_;
	}

	RENGINE_INLINE void Shader::setSource(std::string const& source)
	{
		source_ = source;
	}

	RENGINE_INLINE std::string const& Shader::source() const
	{
		return source_;
	}

	//
	// Uniform
	//

	RENGINE_INLINE Uniform::Uniform(std::string const& name, Type const type, SizeType const size)
	 :type_(type), name_(name), size_(size), data_(0), program_(0)
	{
		initialize();
	}

	RENGINE_INLINE Uniform::Type const& Uniform::type() const
	{
		return type_;
	}

	RENGINE_INLINE std::string const& Uniform::name() const
	{
		return name_;
	}

	RENGINE_INLINE void Uniform::setProgram(Program* program)
	{
		program_ = program;
	}

	RENGINE_INLINE Uniform::SizeType Uniform::componentSize() const
	{
		return SizeType( (type() >> component_shift)  & component_mask);
	}

	RENGINE_INLINE Uniform::SizeType Uniform::components() const
	{
		return SizeType(type() & component_mask);
	}

	RENGINE_INLINE Uniform::SizeType const& Uniform::size() const
	{
		return size_;
	}

	RENGINE_INLINE Uniform::DataType* Uniform::data()
	{
		return data_;
	}

	RENGINE_INLINE void Uniform::flagChanged()
	{
		changeFlags() |= ValueChanged;

		if (program_)
		{
			program_->changeFlags() |= Program::UniformsChanged;
		}
	}

	//
	// Program
	//
	RENGINE_INLINE void Program::addUniform(SharedPointer<Uniform> uniform)
	{
		uniform->setProgram(this);
		uniforms_.push_back(uniform);
	}

	RENGINE_INLINE Program::Uniforms const& Program::uniforms() const
	{
		return uniforms_;
	}

	RENGINE_INLINE Program::Uniforms& Program::uniforms()
	{
		return uniforms_;
	}

	RENGINE_INLINE void Program::addInput(Connection const& input)
	{
		inputs_.push_back(input);
	}

	RENGINE_INLINE Program::Connections const& Program::inputs() const
	{
		return inputs_;
	}

	RENGINE_INLINE Program::Connections& Program::inputs()
	{
		return inputs_;
	}

	RENGINE_INLINE Bool Program::hasInputSemantics() const
	{
		return ((inputs_.size() > 0) && (inputs()[0].semantic != VertexBuffer::None));
	}

	RENGINE_INLINE void Program::addOutput(Connection const& output)
	{
		outputs_.push_back(output);
	}

	RENGINE_INLINE Program::Connections const& Program::outputs() const
	{
		return outputs_;
	}

	RENGINE_INLINE Program::Connections& Program::outputs()
	{
		return outputs_;
	}
}

#endif //__RENGINE_PROGRAM_H__

