#include "UnitTest/UnitTest.h"

//#include <iostream>

#include <rengine/lang/exception/Exception.h>
#include <rengine/lang/exception/BaseExceptions.h>

using namespace std;
using namespace rengine;

void pseudoNew()
{
	throw AllocationException(123, "Unable to allocate memory");
}
void callPseudoNew()
{
	try
	{
		pseudoNew();
	}
	catch (AllocationException const& exception)
	{
//		std::cout << "AllocationException caught on callPseudoNew()" << std::endl;
//		std::cout << exception << std::endl;

		throw MemoryFullException(321, "Memory is full", exception);
	}
}

//
// UnitTestExceptions
//

UNITT_TEST_BEGIN_CLASS(UnitTestExceptions)

virtual void run()
{
	Exception exception_0("first...");
	Exception exception_1(2, "second...");
	Exception exception_2(3);

//	std::cout << exception_0 << std::endl;
//	std::cout << exception_1 << std::endl;
//	std::cout << exception_2 << std::endl;

	Exception grandparent(4, "grandparent");
	Exception father(5, "father", grandparent);
	Exception son(6, "son", father);

//	std::cout << son << std::endl;

	Exception copy(son);
//	std::cout << copy << std::endl;

	MemoryFullException mem_full(12, "This is full", son);
	MemoryFullException mem_full_copy(mem_full);

//	std::cout << mem_full << std::endl;
//	std::cout << mem_full_copy << std::endl;

	try
	{
		callPseudoNew();
	}
	catch (AllocationException const& exception)
	{
//		std::cout << "Smokin::AllocationException caught on main" << std::endl;
//		std::cout << exception << std::endl;
	}
	catch (MemoryFullException const& exception)
	{
//		std::cout << "Smokin::MemoryFullException caught on main" << std::endl;
//		std::cout << exception << std::endl;
	}
	catch (...)
	{
		UNITT_FAIL("Unhandled Exception caught on main");
	}
}

UNITT_TEST_END_CLASS(UnitTestExceptions)
