#include "UnitTest/UnitTest.h"

#include <rengine/state/DrawStates.h>
#include <rengine/state/BaseStates.h>

#include <rengine/state/Streams.h>
#include <rengine/state/Program.h>
#include <rengine/state/ShaderResourceLoader.h>
#include <rengine/string/String.h>


#include <iostream>

using namespace rengine;
using namespace std;


static const std::string vertex_shader_path = "unit_test_data/Shader/pin.vsh";
static const std::string fragment_shader_path = "unit_test_data/Shader/pin.fsh";
static const std::string program_path = "unit_test_data/Shader/BasicEffect.eff";

//
// UnitTestDrawStates
//

UNITT_TEST_BEGIN_CLASS(UnitTestDrawStates)

	virtual void run()
	{
		DrawStates states;

		UNITT_ASSERT(!states.hasCapability(DrawStates::DepthTest));
		UNITT_ASSERT(!states.hasCapability(DrawStates::Blend));
		UNITT_ASSERT(!states.hasCapability(DrawStates::CullFace));


		states.setCapability(DrawStates::DepthTest, DrawStates::Off);
		states.setCapability(DrawStates::Blend, DrawStates::On);
		states.setCapability(DrawStates::CullFace, DrawStates::On);


		UNITT_ASSERT(states.hasCapability(DrawStates::DepthTest));
		UNITT_ASSERT(states.hasCapability(DrawStates::Blend));
		UNITT_ASSERT(states.hasCapability(DrawStates::CullFace));


		UNITT_FAIL_NOT_EQUAL(DrawStates::Off, states.getCapability(DrawStates::DepthTest));
		UNITT_FAIL_NOT_EQUAL(DrawStates::On, states.getCapability(DrawStates::Blend));
		UNITT_FAIL_NOT_EQUAL(DrawStates::On, states.getCapability(DrawStates::CullFace));


		states.setCapability(DrawStates::DepthTest, DrawStates::On);
		states.setCapability(DrawStates::Blend, DrawStates::On);
		states.setCapability(DrawStates::CullFace, DrawStates::On);


		UNITT_FAIL_NOT_EQUAL(DrawStates::On, states.getCapability(DrawStates::DepthTest));
		UNITT_FAIL_NOT_EQUAL(DrawStates::On, states.getCapability(DrawStates::Blend));
		UNITT_FAIL_NOT_EQUAL(DrawStates::On, states.getCapability(DrawStates::CullFace));


		states.setCapability(DrawStates::DepthTest, DrawStates::Off);
		states.setCapability(DrawStates::Blend, DrawStates::Off);
		states.setCapability(DrawStates::CullFace, DrawStates::Off);


		UNITT_FAIL_NOT_EQUAL(DrawStates::Off, states.getCapability(DrawStates::DepthTest));
		UNITT_FAIL_NOT_EQUAL(DrawStates::Off, states.getCapability(DrawStates::Blend));
		UNITT_FAIL_NOT_EQUAL(DrawStates::Off, states.getCapability(DrawStates::CullFace));

		states.clearCapability(DrawStates::DepthTest);
		states.clearCapability(DrawStates::Blend);

		UNITT_ASSERT(!states.hasCapability(DrawStates::DepthTest));
		UNITT_ASSERT(!states.hasCapability(DrawStates::Blend));
		UNITT_ASSERT(states.hasCapability(DrawStates::CullFace));

		states.setDefaults();
		UNITT_ASSERT(!states.hasCapability(DrawStates::DepthTest));
		UNITT_ASSERT(!states.hasCapability(DrawStates::Blend));
		UNITT_ASSERT(!states.hasCapability(DrawStates::CullFace));



		//
		// Copy Test
		//

		states.setCapability(DrawStates::DepthTest, DrawStates::Off);
		states.setCapability(DrawStates::Blend, DrawStates::On);
		states.setCapability(DrawStates::CullFace, DrawStates::On);


		UNITT_ASSERT(states.hasCapability(DrawStates::DepthTest));
		UNITT_ASSERT(states.hasCapability(DrawStates::Blend));
		UNITT_ASSERT(states.hasCapability(DrawStates::CullFace));

		UNITT_ASSERT(states.hasCapability(DrawStates::DepthTest));
		UNITT_ASSERT(states.hasCapability(DrawStates::Blend));
		UNITT_ASSERT(states.hasCapability(DrawStates::CullFace));

		DrawStates copy;
		copy = states;

		UNITT_FAIL_NOT_EQUAL(states.hasCapability(DrawStates::DepthTest), states.hasCapability(DrawStates::DepthTest));
		UNITT_FAIL_NOT_EQUAL(states.hasCapability(DrawStates::Blend), states.hasCapability(DrawStates::Blend));
		UNITT_FAIL_NOT_EQUAL(states.hasCapability(DrawStates::CullFace), states.hasCapability(DrawStates::CullFace));

		for(DrawStates::CapabilityValueMap::const_iterator capability_iterator = states.getCapabilities().begin();
			capability_iterator != states.getCapabilities().end();
			++capability_iterator)
		{
			UNITT_ASSERT(copy.hasCapability(capability_iterator->first));
			UNITT_FAIL_NOT_EQUAL(states.getCapability(capability_iterator->first), copy.getCapability(capability_iterator->first));
		}

	}

UNITT_TEST_END_CLASS(UnitTestDrawStates)

//
// UnitTestDrawStatesStreams
//

UNITT_TEST_BEGIN_CLASS(UnitTestDrawStatesStreams)

	virtual void run()
	{
		UNITT_FAIL_NOT_EQUAL("Undefined", lexical_cast<std::string>(DrawStates::Undefined));
		UNITT_FAIL_NOT_EQUAL("Blend", lexical_cast<std::string>(DrawStates::Blend));
		UNITT_FAIL_NOT_EQUAL("CullFace", lexical_cast<std::string>(DrawStates::CullFace));
		UNITT_FAIL_NOT_EQUAL("DepthTest", lexical_cast<std::string>(DrawStates::DepthTest));
		UNITT_FAIL_NOT_EQUAL("invalid_name", lexical_cast<std::string>(DrawStates::Capability(12345)));


		UNITT_FAIL_NOT_EQUAL("Off", lexical_cast<std::string>(DrawStates::Off));
		UNITT_FAIL_NOT_EQUAL("On", lexical_cast<std::string>(DrawStates::On));
		UNITT_FAIL_NOT_EQUAL("invalid_name", lexical_cast<std::string>(DrawStates::Value(12345)));

		DrawStates states;
		states.setCapability(DrawStates::DepthTest, DrawStates::Off);
		states.setCapability(DrawStates::Blend, DrawStates::Off);
		states.setCapability(DrawStates::CullFace, DrawStates::Off);



		SharedPointer<BlendFunction> blend_function_0 = new BlendFunction(BlendFunction::Zero, BlendFunction::One);
		SharedPointer<BlendFunction> blend_function_1 = blend_function_0->clone();
		UNITT_FAIL_NOT_EQUAL(0, blend_function_0->compare(*blend_function_1));


//		std::cout << states << std::endl;
//
//		states.clear();
//		std::cout << "clear: " << std::endl << states << std::endl;
//
//		states.setState(blend_function_0);
//		std::cout <<  "blend_function_0: " << std::endl << states << std::endl;
//		states.setState(blend_function_0, DrawStates::Off);
//		std::cout <<  "blend_function_0: " << std::endl << states << std::endl;

	}

UNITT_TEST_END_CLASS(UnitTestDrawStatesStreams)


//
// UnitTestShaderLoader
//

UNITT_TEST_BEGIN_CLASS(UnitTestShaderLoader)

	virtual void run()
	{

		ShaderResourceLoader shader_resource_loader;
		shader_resource_loader.setCacheOption(BaseResourceLoader::DoNoCacheResources);


		ProgramResourceLoader program_resource_loader;
		program_resource_loader.setCacheOption(BaseResourceLoader::DoNoCacheResources);

//		{
//			SharedPointer<Shader> shader = shader_resource_loader.load(vertex_shader_path);
//			UNITT_ASSERT(shader->type() == Shader::Vertex);
//
//			std::cout << "<" << vertex_shader_path << ">" << std::endl << shader->source() << std::endl;
//		}
//
//		{
//			SharedPointer<Shader> shader = shader_resource_loader.load(fragment_shader_path);
//			UNITT_ASSERT(shader->type() == Shader::Fragment);
//
//			std::cout << "<" << fragment_shader_path << ">" << std::endl << shader->source() << std::endl;
//		}


//		{
//			SharedPointer<Program> program = program_resource_loader.load(program_path);
//			UNITT_ASSERT(program);
//			UNITT_ASSERT(program->getShader(Shader::Vertex));
//			UNITT_ASSERT(program->getShader(Shader::Fragment));
//
//			std::cout << "<" << program_path << " Shader::Vertex>" << std::endl << program->getShader(Shader::Vertex)->source() << std::endl;
//			std::cout << "<" << program_path << " Shader::Fragment>" << std::endl << program->getShader(Shader::Fragment)->source() << std::endl;
//		}


	}

UNITT_TEST_END_CLASS(UnitTestShaderLoader)
