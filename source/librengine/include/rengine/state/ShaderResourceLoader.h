// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SHADER_RESOURCE_LOADER_H__
#define __RENGINE_SHADER_RESOURCE_LOADER_H__

#include <rengine/resource/ResourceLoader.h>
#include <rengine/state/Program.h>
#include <rengine/util/StringTable.h>

namespace rengine
{
	class ShaderResourceLoader : public ResourceLoader<Shader>
	{
	public:
		virtual Bool suportsFormat(std::string const& extension) const;

		/**
		 * Accepts the following options
		 * -> Shader::Type shader_type
		 */
		virtual SharedPointer<Shader> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	};

	class ProgramResourceLoader : public ResourceLoader<Program>
	{
	public:
		virtual Bool suportsFormat(std::string const& extension) const;

		virtual SharedPointer<Program> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties());
	private:
		typedef std::pair<std::string, std::string> DefaultValue;
		typedef std::pair<std::string, std::string> Semantic;
		typedef std::vector<DefaultValue> DefaultValues;
		typedef std::vector<Semantic> Semantics;

		typedef Program::Connection Connection;
		typedef Program::Connections Connections;

		enum Action {
			LineCopy			= 0,
			AssembleStatement   = 1,
			UniformStatement	= 2
		};

		std::string getSection(std::string const& src, std::string const& name);
		std::string preprocess(SharedPointer<Program> program, std::string src, std::string const& base_location);
		std::string expandSymbols(std::string const& src);

		std::string pragmaInclude(std::string const& line, std::string const& base_location);
		void pragmaDefault(std::string const& line);
		void pragmaSemantic(std::string const& line);
		std::string glslVersion(std::string const& line);

		std::string declarationUniform(SharedPointer<Program> program, std::string uniform_src);

		bool isTypeDeclaration(std::string clean_line);
		std::string varyingElements(std::string const& src, std::string prefix);
		void collectInputOutput(std::string const& src, Connections& inputs, Connections& outputs);
		void connectionDeclaration(std::string src, Connections& inputs, Connections& outputs, bool const is_input);

		void addToStatement(std::string const& src);
		Bool statementEnded();

		void applyDefaultValues(SharedPointer<Program> program);

		void addDefaultValue(DefaultValue const& value);

		void addSemanticValue(Semantic const& value);
		std::string findSemanticByVariable(std::string const& value) const;


		std::string errors;
		std::string statement;

		DefaultValues default_values;
		Semantics semantics;
		StringTable symbols;

		bool m_limitedToOpenGL21;

		typedef std::map<std::string, std::string> InOutLines;
		InOutLines in_out_lines;
		std::string replaceInOutLines(std::string const src, Bool isIn);
	};
}

#endif //__RENGINE_SHADER_RESOURCE_LOADER_H__

