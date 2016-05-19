#include "UnitTest/UnitTest.h"

#include <rengine/lang/SourceCodeLocation.h>
#include <rengine/outputstream/Log.h>
#include <rengine/file/File.h>


#include <iostream>

using namespace rengine;
//
// UnitTestLogSourceCodeLocation
//

UNITT_TEST_BEGIN_CLASS(UnitTestLogSourceCodeLocation)

	virtual void run()
	{
		SourceCodeLocation location = RENGINE_SOURCE_CODE_LOCATION();
		UNITT_FAIL_NOT_EQUAL(__LINE__ - 1, location.line_number);

		//std::cout << location.filename << " | " << location.function << " | " << std::endl;

		std::string source = convertCase(Lowercase, convertFileNameToUnixStyle(location.filename));
		std::string test = convertCase(Lowercase, "rengine/source/UnitTest/Log.cpp");
		UNITT_ASSERT(endsWith(source, test));

		//UNITT_FAIL_NOT_EQUAL(std::string(location.function), "run");
		//UNITT_FAIL_NOT_EQUAL(std::string(location.function_pretty), "virtual void UnitTestLogSourceCodeLocation::run()");
	}

UNITT_TEST_END_CLASS(UnitTestLogSourceCodeLocation)


//
// UnitTestLog
//

UNITT_TEST_BEGIN_CLASS(UnitTestLog)

	virtual void run()
	{
		for (int i = 0; i != 100; ++i)
			RLOG_INFO("UnitTest", "dumping line " << i);
	}

UNITT_TEST_END_CLASS(UnitTestLog)
