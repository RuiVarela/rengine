// __!!ryven_copyright!!__ //

#include <rengine/lang/debug/Debug.h>
#include <iostream>

#include "UnitTest/UnitTest.h"

int main()
{
	rengine::enableApplicationDebugger();

	std::cout << "UnitTest Running." << std::endl;
	UnitTest::UnitTester::instance()->run();
	std::cout << "UnitTest Finished." << std::endl;

#if RENGINE_COMPILER == RENGINE_COMPILER_MSVC
	std::cin.get();
#endif //RENGINE_COMPILER == RENGINE_COMPILER_MSVC

	return 0;
}
