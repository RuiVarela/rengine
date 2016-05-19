// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>
#include <rengine/RenderEngine.h>
#include <rengine/state/Program.h>
#include <rengine/state/Streams.h>
#include <rengine/string/String.h>
#include <rengine/lang/debug/Debug.h>

#include <cstdlib>
#include <cstring>


namespace rengine
{
	//
	// Shader
	//

	Shader::Shader(Type const type) :
		type_(type)
	{

	}

	Shader::Shader(Type const type, std::string const source) :
		type_(type), source_(source)
	{

	}

	Shader::~Shader()
	{
		release();
	}

	void Shader::release()
	{
		// TODO : this should be implemented with an observer
		CoreEngine::instance()->renderEngine().unloadShader(*this);
	}

	//
	// Uniform
	//
	void Uniform::initialize()
	{
		SizeType components_ = components();
		SizeType component_size_ = componentSize();

		offset_ = component_size_ * components_;

		data_ = new DataType[offset_ * size_];
		memset(data_, 0, offset_ * size_);
	}

	Uniform::~Uniform()
	{
		if (data_)
		{
			delete[](data_);
		}
	}

	void Uniform::memcpy(SizeType const& target, void const * src, SizeType const& n)
	{
		RENGINE_ASSERT(target < size_);
		RENGINE_ASSERT(n == (componentSize() * components()));
		RENGINE_ASSERT(data_);

		::memcpy(data_ + offset_ * target, src, n);
	}

	Uniform::Type Uniform::typeFromString(std::string name)
	{
		Type type = FloatUniform;

		lowercase(name);

		if 	    (name == "float")  { type = FloatUniform; }
		else if (name == "vec2")   { type = FloatVec2Uniform; }
		else if (name == "vec3")   { type = FloatVec3Uniform; }
		else if (name == "vec4")   { type = FloatVec4Uniform; }
		else if (name == "int")    { type = IntUniform; }
		else if (name == "ivec2")  { type = IntVec2Uniform; }
		else if (name == "ivec3")  { type = IntVec3Uniform; }
		else if (name == "ivec4")  { type = IntVec4Uniform; }
		else if (name == "mat4")   { type = Mat4x4Uniform; }
		else if (name == "mat4x4") { type = Mat4x4Uniform; }

		return type;
	}

	//
	// Program
	//
	Program::Program()
	:vertex_shader(0), fragment_shader(0)
	{
	}

	Program::~Program()
	{
		release();
	}

	void Program::release()
	{
		// TODO : this should be implemented with an observer
		CoreEngine::instance()->renderEngine().unloadProgram(*this);
	}

	void Program::setShader(SharedPointer<Shader> shader)
	{
		if (shader)
		{
			if (shader->type() == Shader::Vertex)
			{
				if (shader.get() != vertex_shader.get())
				{
					vertex_shader = shader;
					changeFlags() |= VertexShaderChanged;
				}
			}
			else if (shader->type() == Shader::Fragment)
			{
				if (shader.get() != fragment_shader.get())
				{
					fragment_shader = shader;
					changeFlags() |= FragmentShaderChanged;
				}
			}
		}
	}

	void Program::removeShader(Shader::Type const& type)
	{
		if (type == Shader::Vertex)
		{
			vertex_shader = 0;
			changeFlags() |= VertexShaderChanged;
		}
		else if (type == Shader::Fragment)
		{
			fragment_shader = 0;
			changeFlags() |= FragmentShaderChanged;
		}
	}

	SharedPointer<Shader> const& Program::getShader(Shader::Type const& type) const
	{
		if (type == Shader::Vertex)
		{
			return vertex_shader;
		}
		else
		{
			return fragment_shader;
		}
	}

	SharedPointer<Shader>& Program::getShader(Shader::Type const& type)
	{
		if (type == Shader::Vertex)
		{
			return vertex_shader;
		}
		else
		{
			return fragment_shader;
		}
	}

	Bool Program::hasUniform(std::string const& name) const
	{
		Bool found = false;

		Uniforms::const_iterator i = uniforms_.begin();
		while(i != uniforms_.end())
		{
			if ( (*i)->name() == name )
			{
				found = true;
				break;
			}
			++i;
		}

		return found;
	}

	Uniform const& Program::uniform(std::string const& name) const
	{
		RENGINE_ASSERT( hasUniform(name) );

		Uniforms::const_iterator i = uniforms_.begin();
		while(i != uniforms_.end())
		{
			if ( (*i)->name() == name )
			{
				break;
			}
			++i;
		}

		return *(i->get());
	}

	Uniform& Program::uniform(std::string const& name)
	{
		RENGINE_ASSERT( hasUniform(name) );

		Uniforms::iterator i = uniforms_.begin();
		while(i != uniforms_.end())
		{
			if ( (*i)->name() == name )
			{
				break;
			}
			++i;
		}

		return *(i->get());
	}

	void sortConnections(Program::Connections& connections)
	{
		Program::Connections sorted;

		//
		// Semantics first
		//
		for (Program::Connections::size_type i = 0; i != connections.size(); ++i)
		{
			if ( (connections[i].semantic != VertexBuffer::None) &&
				 (connections[i].id > -1) // remove dead connections
			   )
			{
				sorted.push_back(connections[i]);
			}
		}


		for (Program::Connections::size_type i = 0; i != connections.size(); ++i)
		{
			if ( (connections[i].semantic == VertexBuffer::None) &&
			     (connections[i].id > -1) // remove dead connections
			   )
			{
				sorted.push_back(connections[i]);
			}
		}

		connections = sorted;
	}

	void Program::prepareConnections()
	{
		sortConnections(inputs_);
		sortConnections(outputs_);
	}

	//
	// ProgramUnit
	//
	ProgramUnit::ProgramUnit()
		:State(DrawStates::Program)
	{

	}

	ProgramUnit::ProgramUnit(SharedPointer<Program> const& program)
		:State(DrawStates::Program), program_(program)
	{
	}

	ProgramUnit::ProgramUnit(ProgramUnit const& rhs)
		:State(DrawStates::Program), program_(rhs.program_)
	{
	}

	ProgramUnit::~ProgramUnit()
	{
	}

	Int ProgramUnit::compare(DrawStates::State const& rhs) const
	{
		META_STATE_COMPARE(ProgramUnit, rhs, casted_rhs);
		META_STATE_COMPARE_PROPERTY(casted_rhs, program_.get());

		if (casted_rhs.program_)
		{
			if (casted_rhs.program_->changeFlags())
			{
				return 1;
			}
		}

		return 0;
	}

	void ProgramUnit::set(SharedPointer<Program> const& program)
	{
		program_ = program;
	}

	SharedPointer<Program> const& ProgramUnit::get() const
	{
		return program_;
	}
	SharedPointer<Program>& ProgramUnit::get()
	{
		return program_;
	}




	//
	// Support for ostream
	//

	META_STATE_HAS_OSTREAM_IMPLEMENTATION(ProgramUnit)

} // namespace rengine


