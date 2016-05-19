// __!!rengine_copyright!!__ //

#include <rengine/state/ShaderResourceLoader.h>
#include <rengine/file/File.h>
#include <rengine/string/String.h>
#include <rengine/math/Vector.h>
#include <rengine/CoreEngine.h>
#include <rengine/outputstream/OutputStream.h>

#include <iostream>

namespace rengine
{
	static std::string const section_marker = "//!pragma section";
	static std::string const include_marker = "//!pragma include";
	static std::string const default_marker = "//!pragma default";
	static std::string const semantic_marker = "//!pragma semantic";
	static std::string const glsl_version_marker = "#version ";
	static std::string const uniform_marker = "uniform";
	static std::string const symbol_marker = "$";

	static std::string const float_marker = "float";
	static std::string const vec2_marker = "vec2";
	static std::string const vec3_marker = "vec3";
	static std::string const vec4_marker = "vec4";
	static std::string const int_marker = "int";
	static std::string const uint_marker = "uint";
	static std::string const ivec2_marker = "ivec2";
	static std::string const uvec2_marker = "uvec2";
	static std::string const ivec3_marker = "ivec3";
	static std::string const uvec3_marker = "uvec3";
	static std::string const ivec4_marker = "ivec4";
	static std::string const uvec4_marker = "uvec4";
	static std::string const mat4_marker = "mat4";
	static std::string const mat4x4_marker = "mat4x4";
	static std::string const sampler2D_marker = "sampler2D";

	static bool read(std::istream& in, Real& value)
	{
		bool good = true;
		if (in.good())
		{
			in >> value;
		}
		else
		{
			good = false;
		}
		return good;
	}

	static bool read(std::istream& in, Int& value)
	{
		bool good = true;
		if (in.good())
		{
			in >> value;
		}
		else
		{
			good = false;
		}
		return good;
	}

	static bool read(std::istream& in, Vector2D& value)
	{
		return read(in, value.x()) && read(in, value.y());
	}

	static bool read(std::istream& in, Vector3D& value)
	{
		return read(in, value.x()) && read(in, value.y()) && read(in, value.z());
	}

	static bool read(std::istream& in, Vector4D& value)
	{
		return read(in, value.x()) && read(in, value.y()) && read(in, value.z()) && read(in, value.w());
	}

	static bool read(std::istream& in, Vector2Di& value)
	{
		return read(in, value.x()) && read(in, value.y());
	}

	static bool read(std::istream& in, Vector3Di& value)
	{
		return read(in, value.x()) && read(in, value.y()) && read(in, value.z());
	}

	static bool read(std::istream& in, Vector4Di& value)
	{
		return read(in, value.x()) && read(in, value.y()) && read(in, value.z()) && read(in, value.w());
	}

	Bool ShaderResourceLoader::suportsFormat(std::string const& extension) const
	{
		Bool can_load = false;

		if ( (extension == "fsh") ||
		     (extension == "vsh") )
		{
			can_load = true;
		}

		return can_load;
	}

	SharedPointer<Shader> ShaderResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<Shader> shader;

		std::string source;

		if (readRawText(location, source))
		{
			Shader::Type type = Shader::Fragment;

			if ( equalCaseInsensitive(getFileExtension(location), "vsh") )
			{
				type = Shader::Vertex;
			}

			if (options.hasProperty("shader_type"))
			{
				type = any_cast<Shader::Type>(options["shader_type"].value);
			}

			shader = new Shader(type, source);
		}

		return shader;
	}

	Bool ProgramResourceLoader::suportsFormat(std::string const& extension) const
	{
		Bool can_load = false;

		if ( (extension == "eff") )
		{
			can_load = true;
		}

		return can_load;
	}

	SharedPointer<Program> ProgramResourceLoader::loadImplementation(std::string const& location, OpaqueProperties const& options)
	{
		m_limitedToOpenGL21 = CoreEngine::instance()->renderEngine().limitedToOpenGL21();

		errors.clear();
		statement.clear();
		default_values.clear();
		semantics.clear();
		symbols.clear();

		if (options.hasProperty("symbols"))
		{
			symbols = any_cast<StringTable>(options["symbols"].value);
		}

		SharedPointer<Program> program = new Program();

		std::string source;
		readRawText(location, source);
		std::string base_path = getFilePath(location);
		source = preprocess(program, source, base_path);

		std::string common_section = getSection(source, "common");
		std::string input_vertex_section = getSection(source, "vertex");
		std::string input_fragment_section = getSection(source, "fragment");
		std::string varying_section = getSection(source, "varying");

		std::string vertex_section;
		std::string fragment_section;

		if (errors.empty() && !common_section.empty())
		{
			vertex_section += common_section + "\n";
			fragment_section += common_section + "\n";
		}

		if (errors.empty() && !varying_section.empty())
		{
			std::string prefix = "varying";

			if (!m_limitedToOpenGL21) prefix = "out";
			vertex_section += varyingElements(varying_section, prefix) + "\n";
			
			if (!m_limitedToOpenGL21) prefix = "in";
			fragment_section += varyingElements(varying_section, prefix) + "\n";
		}

		vertex_section += input_vertex_section;
		fragment_section += input_fragment_section;

		if (errors.empty() && !vertex_section.empty())
		{
			in_out_lines.clear();
			Connections inputs, outputs;
			collectInputOutput(vertex_section, inputs, outputs);
			program->inputs() = inputs;

			if (m_limitedToOpenGL21) 
			{
				vertex_section = replaceInOutLines(vertex_section, true);
			}
			
			SharedPointer<Shader> shader = new Shader(Shader::Vertex);
			shader->setSource(vertex_section);
			program->setShader(shader);
		}

		if (errors.empty() && !fragment_section.empty())
		{
			in_out_lines.clear();
			Connections inputs, outputs;
			collectInputOutput(fragment_section, inputs, outputs);
			
			if (m_limitedToOpenGL21) 
			{
				fragment_section = replaceInOutLines(fragment_section, false);
			}
			else
			{
				program->outputs() = outputs;
			}
			

			SharedPointer<Shader> shader = new Shader(Shader::Fragment);
			shader->setSource(fragment_section);
			program->setShader(shader);
		}

		if (!program->getShader(Shader::Vertex) && !program->getShader(Shader::Fragment) )
		{
			program = 0;
		}

		if (!errors.empty())
		{
			program = 0;

			CoreEngine::instance()->log() << "Unable to load effect [" << location << "]" << std::endl;
			CoreEngine::instance()->log() << errors << std::endl;
		}

		return program;
	}

	std::string ProgramResourceLoader::preprocess(SharedPointer<Program> program, std::string src, std::string const& base_location)
	{
		//
		// Expand symbols
		//
		src = expandSymbols(src);

		Bool needs_next_pass = true;
		Uint const maximum_passes = 50;
		Uint pass = 0;

		while (needs_next_pass && (pass < maximum_passes))
		{
			needs_next_pass = false;

			std::stringstream output_stream;
			std::stringstream string_stream;
			string_stream << src;
			std::string line;

			Action statement_action = LineCopy;
			Action current_action = LineCopy;

			while (std::getline(string_stream, line))
			{
				std::string clean_line = line;
				trim(clean_line);

				bool add_to_output = true;

				if (current_action != AssembleStatement)
				{
					if (startsWith(clean_line, include_marker)) // check for include pragmas
					{
						output_stream << pragmaInclude(clean_line, base_location) << std::endl;
						needs_next_pass = true;
						add_to_output = false;
					}
					else if (startsWith(clean_line, default_marker)) // check for default pragmas
					{
						pragmaDefault(clean_line);
					}
					else if (startsWith(clean_line, semantic_marker)) // check for semantic pragmas
					{
						pragmaSemantic(clean_line);
					}
					else if (startsWith(clean_line, uniform_marker + " "))
					{
						current_action = AssembleStatement;
						statement_action = UniformStatement;
					}
					else if (startsWith(clean_line, glsl_version_marker))
					{
						line = glslVersion(clean_line);
					}
					else if (startsWith(clean_line, "//"))
					{

					}
				}


				//
				// Assemble Current Statement
				//
				if (current_action == AssembleStatement)
				{
					addToStatement(clean_line);

					if (statementEnded())
					{
						current_action = LineCopy;
					}
				}

				//
				// Handle Statement
				//
				if ( (current_action == LineCopy) && (statement.size() != 0) )
				{
					if (statement_action == UniformStatement)
					{
						declarationUniform(program, statement);
					}

					statement.clear();
				}


				if (add_to_output)
				{
					output_stream << line << std::endl;
				}
			}

			src = output_stream.str();
			++pass;
		}

		if (pass >= maximum_passes)
		{
			errors += "Processing passes exhausted\n";
		}


		if (errors.empty())
		{
			applyDefaultValues(program);
		}

		return src;
	}

	std::string ProgramResourceLoader::expandSymbols(std::string const& src)
	{
		std::string output = src;
		for (StringTable::const_iterator i = symbols.begin(); i != symbols.end(); ++i)
		{
			std::string key = symbol_marker + i->first + symbol_marker;
			std::string value = i->second.text;

			replace(output, key, value);
		}

		return output;
	}

	void ProgramResourceLoader::addToStatement(std::string const& src)
	{
		if (!src.empty())
		{
			std::string::size_type end = src.find("//");
			statement += " " + src.substr(0, end);
			trim(statement);
		}
	}

	Bool ProgramResourceLoader::statementEnded()
	{
		return endsWith(statement, ";");
	}

	bool ProgramResourceLoader::isTypeDeclaration(std::string clean_line)
	{
		return startsWith(clean_line, float_marker) ||
			   startsWith(clean_line, vec2_marker)  || startsWith(clean_line, vec3_marker)  || startsWith(clean_line, vec4_marker) ||
			   startsWith(clean_line, int_marker)   || startsWith(clean_line, uint_marker)  ||
			   startsWith(clean_line, ivec2_marker) || startsWith(clean_line, ivec3_marker) || startsWith(clean_line, ivec4_marker) ||
			   startsWith(clean_line, uvec2_marker) || startsWith(clean_line, uvec3_marker) || startsWith(clean_line, uvec4_marker);

	}

	std::string ProgramResourceLoader::varyingElements(std::string const& src, std::string prefix)
	{
		std::stringstream output_stream;
		std::stringstream string_stream;
		string_stream << src;
		std::string line;

		Action current_action = LineCopy;

		while (std::getline(string_stream, line))
		{
			std::string clean_line = line;
			trim(clean_line);

			if (startsWith(clean_line, "//") || (current_action == AssembleStatement)) { }
			else if (isTypeDeclaration(clean_line))
			{
				current_action = AssembleStatement;
			}

			// Assemble Current Statement
			if (current_action == AssembleStatement)
			{
				addToStatement(clean_line);

				if (statementEnded())
				{
					current_action = LineCopy;
				}
			}

			// Handle Statement
			if ((current_action == LineCopy) && (statement.size() != 0))
			{
				clean_line = prefix + " " + statement;
				trim(clean_line);
				output_stream << clean_line << std::endl;

				statement.clear();
			}
		}
		return output_stream.str();
	}


	std::string ProgramResourceLoader::replaceInOutLines(std::string const src, Bool isIn)
	{
		std::string out = src;
		for (InOutLines::iterator i = in_out_lines.begin(); i != in_out_lines.end(); ++i)
		{
			if (isIn)
			{
				std::string tag = i->second;
				trim(TrimBoth, tag);

				if (startsWith(tag, "in") && (tag.size() > 2))
				{
					tag = tag.substr(2, tag.size() - 2);

					replace(out, i->second, " attribute " + tag);
				}

			}
			else
			{
				replace(out, i->second, "/* " + i->second + " */");
				replace(out, i->first + " ", "gl_FragColor ");
				replace(out, i->first + "=", "gl_FragColor=");
				replace(out, i->first + ".", "gl_FragColor.");
			}
		}

		return out;
	}

	void ProgramResourceLoader::collectInputOutput(std::string const& src, Connections& inputs, Connections& outputs)
	{
		statement.clear();

		std::stringstream string_stream;
		string_stream << src;
		std::string line;

		bool is_input = false;
		Action current_action = LineCopy;
		while (std::getline(string_stream, line))
		{
			std::string clean_line = line;
			trim(clean_line);

			if (startsWith(clean_line, "//") || (current_action == AssembleStatement)) { }
			else if (startsWith(clean_line, "in "))
			{
				is_input = true;
				current_action = AssembleStatement;
			}
			else if (startsWith(clean_line, "out "))
			{
				is_input = false;
				current_action = AssembleStatement;
			}

			// Assemble Current Statement
			if (current_action == AssembleStatement)
			{
				addToStatement(clean_line);

				if (statementEnded())
				{
					current_action = LineCopy;
				}
			}

			// Handle Statement
			if ((current_action == LineCopy) && (statement.size() != 0))
			{
				if ( statement.find(")") == std::string::npos ) // make sure it is not an input our output of a function
				{
					trim(statement);
					connectionDeclaration(statement, inputs, outputs, is_input);
				}

				statement.clear();
			}
		}
	}

	void ProgramResourceLoader::connectionDeclaration(std::string src, Connections& inputs, Connections& outputs, bool const is_input)
	{
		std::string original = src;

		//std::cout << "connectionDeclaration: " << src << std::endl;

		std::string connection_marker = is_input ? "in" : "out";

		replace(src, ";", "");
		trim(src);
		src = src.substr(connection_marker.size());
		trim(src);

		std::string::size_type separator = src.find(" ");
		std::string marker = src.substr(0, separator);

		src = src.substr(marker.size(), std::string::npos);
		trim(src);

		StringElements variables = split(src, ",");

		for(StringElements::size_type i = 0; i != variables.size(); ++i)
		{
			std::string variable = variables[i];
			trim(variable);

			Connection connection;
			connection.id = -1;
			connection.name = variable;
			connection.type = marker;
			connection.semantic = VertexBuffer::semanticFromString( findSemanticByVariable(variable) );

			//std::cout << "[" << marker << "] " << "[" << variable << "]" << "[" << int(connection.semantic) << "]" << std::endl;

			in_out_lines[variable] = original;

			if (is_input)
			{
				inputs.push_back(connection);
			}
			else
			{
				outputs.push_back(connection);
			}
		}
	}

	std::string ProgramResourceLoader::declarationUniform(SharedPointer<Program> program, std::string uniform_src)
	{
		replace(uniform_src, ";", "");
		trim(uniform_src);
		uniform_src = uniform_src.substr(uniform_marker.size());
		trim(uniform_src);

		std::string::size_type separator = uniform_src.find(" ");
		std::string marker = uniform_src.substr(0, separator);

		uniform_src = uniform_src.substr(marker.size(), std::string::npos);
		trim(uniform_src);

		if (marker == mat4x4_marker)
		{
			marker = mat4_marker;
		}
		else if (marker == sampler2D_marker)
		{
			marker = int_marker;
		}

		StringElements variables = split(uniform_src, ",");

		for(StringElements::size_type i = 0; i != variables.size(); ++i)
		{
			std::string variable = variables[i];

			Uniform::SizeType size = 1;

			// Check if it is an array
			std::string::size_type indexer_begin = variable.find("[");
			if (indexer_begin != std::string::npos)
			{
				indexer_begin += 1;
				std::string::size_type indexer_end = variable.find("]");
				size = lexical_cast<Uniform::SizeType>( variable.substr(indexer_begin, indexer_end - indexer_begin) , 1);

				variable = variable.substr(0, indexer_begin - 1);
			}

			trim(variable);

			if (!program->hasUniform(variable))
			{
				Uniform::Type type = Uniform::typeFromString(marker);
				program->addUniform( new Uniform(variable, type, size) );
				//CoreEngine::instance()->log() << "[" << marker << " | " << size << " | " << variable << "]" << std::endl;
			}
		}

		return uniform_src;
	}

	void ProgramResourceLoader::pragmaDefault(std::string const& line)
	{
		std::string command = line.substr(default_marker.size());
		trim(command);

		std::string::size_type space = command.find(" ");

		std::string name = command.substr(0, space);
		std::string values = command.substr(space);
		replace(values, ",", "");
		trim(values);

		DefaultValue default_value;
		default_value.first = name;
		default_value.second = values;

		addDefaultValue(default_value);
		//CoreEngine::instance()->log() << "Default [" << default_value.first << " | " << default_value.second << "]" << std::endl;
	}

	std::string ProgramResourceLoader::pragmaInclude(std::string const& line, std::string const& base_location)
	{
		std::string file = line.substr(include_marker.size());
		trim(file);

		std::string file_contents;
		std::string filename = convertFileNameToNativeStyle(base_location + "/" + file);
		if (readRawText(filename, file_contents))
		{
			std::string current_base_location = getFilePath(file);
			trim(current_base_location);

			//make the contents includes relative to the base
			if (!current_base_location.empty())
			{
				replace(file_contents, include_marker + " ", include_marker + " " + current_base_location + "/");
			}

			trim(file_contents);
		}
		else
		{
			errors += "Unable to include " + filename + "\n";
		}

		return file_contents;
	}

	void ProgramResourceLoader::pragmaSemantic(std::string const& line)
	{
		std::string command = line.substr(semantic_marker.size());
		trim(command);

		std::string::size_type space = command.find(" ");

		Semantic value;
		value.first = command.substr(0, space);
		value.second = command.substr(space);

		trim(value.first);
		trim(value.second);

		addSemanticValue(value);
		//CoreEngine::instance()->log() << "Semantic [" << value.first << " | " << value.second << "]" << std::endl;
	}

	std::string ProgramResourceLoader::glslVersion(std::string const& line)
	{
		if (m_limitedToOpenGL21)
		{
			return "// <version commented due system limitation gl21> " + line;
		}

		return line;
	}

	std::string ProgramResourceLoader::getSection(std::string const& src, std::string const& name)
	{
		std::string output;

		std::string marker = section_marker + " " + name;

		if (!src.empty())
		{
			std::string::size_type begin = src.find(marker);

			if (begin != std::string::npos)
			{
				begin += marker.size();

				std::string::size_type end = src.find(section_marker, begin);

				if (end != std::string::npos)
				{
					end = end - begin;
				}

				output = src.substr(begin, end);
			}
		}

		trim(output);

		return output;
	}


	template<typename T>
	void addToMap(T const& value, std::vector<T>& elements)
	{
		typename std::vector<T>::iterator iterator = elements.begin();

		while (iterator != elements.end())
		{
			if (iterator->first == value.first)
			{
				break;
			}
			++iterator;
		}

		if (iterator != elements.end())
		{
			iterator->second = value.second;
		}
		else
		{
			elements.push_back(value);
		}
	}


	void ProgramResourceLoader::addDefaultValue(DefaultValue const& value)
	{
		addToMap(value, default_values);
	}

	void ProgramResourceLoader::addSemanticValue(Semantic const& value)
	{
		addToMap(value, semantics);
	}

	std::string ProgramResourceLoader::findSemanticByVariable(std::string const& value) const
	{
		std::string out = "None";

		Semantics::const_iterator iterator = semantics.begin();

		while (iterator != semantics.end())
		{
			if (iterator->first == value)
			{
				out = iterator->second;
				break;
			}
			++iterator;
		}

		return out;
	}

	void ProgramResourceLoader::applyDefaultValues(SharedPointer<Program> program)
	{
		DefaultValues::iterator iterator = default_values.begin();
		while (iterator != default_values.end())
		{
			if (program->hasUniform(iterator->first))
			{
				std::stringstream stream;
				stream << iterator->second;

				Uniform* uniform = &program->uniform(iterator->first);
				bool ok = true;

				if 		(uniform->type() == Uniform::FloatUniform) 			{ Real value; ok = read(stream, value); uniform->set(value); }
				else if (uniform->type() == Uniform::FloatVec2Uniform) 		{ Vector2D value; ok = read(stream, value); uniform->set(value); }
				else if (uniform->type() == Uniform::FloatVec3Uniform)		{ Vector3D value; ok = read(stream, value); uniform->set(value); }
				else if (uniform->type() == Uniform::FloatVec4Uniform)		{ Vector4D value; ok = read(stream, value); uniform->set(value); }
				else if (uniform->type() == Uniform::IntUniform) 			{ Int value; ok = read(stream, value); uniform->set(value); }
				else if (uniform->type() == Uniform::IntVec2Uniform) 		{ Vector2Di value; ok = read(stream, value); uniform->set(value); }
				else if (uniform->type() == Uniform::IntVec3Uniform)		{ Vector3Di value; ok = read(stream, value); uniform->set(value); }
				else if (uniform->type() == Uniform::IntVec4Uniform)		{ Vector4Di value; ok = read(stream, value); uniform->set(value); }

				if (!ok)
				{
					errors += "Bad default value for [" + iterator->first + "] : [" + iterator->second + "]";
				}
			}

			++iterator;
		}
	}



} // namespace rengine
