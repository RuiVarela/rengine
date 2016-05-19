#include "UnitTest/UnitTest.h"

#include <rengine/lang/Lang.h>
#include <rengine/lang/debug/Debug.h>

#include <iostream>

using namespace rengine;

//
// UnitTestTypes
//

UNITT_TEST_BEGIN_CLASS(UnitTestTypes)

	virtual void run()
	{
		UNITT_FAIL_NOT_EQUAL(32 / 8, sizeof(Real32));
		UNITT_FAIL_NOT_EQUAL(64 / 8, sizeof(Real64));
		UNITT_FAIL_NOT_EQUAL( 8 / 8, sizeof(Int8));
		UNITT_FAIL_NOT_EQUAL(16 / 8, sizeof(Int16));
		UNITT_FAIL_NOT_EQUAL(32 / 8, sizeof(Int32));
		UNITT_FAIL_NOT_EQUAL(64 / 8, sizeof(Int64));
		UNITT_FAIL_NOT_EQUAL( 8 / 8, sizeof(Uint8));
		UNITT_FAIL_NOT_EQUAL(16 / 8, sizeof(Uint16));
		UNITT_FAIL_NOT_EQUAL(32 / 8, sizeof(Uint32));
		UNITT_FAIL_NOT_EQUAL(64 / 8, sizeof(Uint64));

		UNITT_FAIL_NOT_EQUAL(sizeof(Real32), sizeof(Real));
		UNITT_FAIL_NOT_EQUAL(sizeof(Int32),  sizeof(Int));
		UNITT_FAIL_NOT_EQUAL(sizeof(Uint32), sizeof(Uint));
		UNITT_FAIL_NOT_EQUAL(sizeof(Uint8),  sizeof(Uchar));
		UNITT_FAIL_NOT_EQUAL(sizeof(Int8),   sizeof(Char));
		UNITT_FAIL_NOT_EQUAL(sizeof(Uint16), sizeof(Ushort));
		UNITT_FAIL_NOT_EQUAL(sizeof(Int16),  sizeof(Short));
		UNITT_FAIL_NOT_EQUAL(sizeof(bool),   sizeof(Bool));
		UNITT_FAIL_NOT_EQUAL(sizeof(Int8),   sizeof(Char));
		UNITT_FAIL_NOT_EQUAL(sizeof(Uint8),   sizeof(Byte));

	}

UNITT_TEST_END_CLASS(UnitTestTypes)

class Base
{
public:
	Base()
	{

	}

	virtual ~Base()
	{
		//std::cout << "Base !"<< std::endl;
		//RENGINE_ASSERT(pureVirtualFunctionCall() == 1);
	}

	virtual int virtualFunction()
	{
		return 1;
	}

	virtual int virtualFunctionClose() = 0;

	int pureVirtualFunctionCall()
	{
		return virtualFunctionClose();
	}
};

class Extended : public Base
{
public:
	Extended()
	{

	}
	virtual ~Extended()
	{
		RENGINE_ASSERT(virtualFunctionClose() == 12);
		//std::cout << "Extended " << virtualFunctionClose() << std::endl;
	}

	virtual int virtualFunction()
	{
		return 3;
	}


	virtual int virtualFunctionClose()
	{
		return 4 * virtualFunction();
	}
};

class MoreExtended : public Extended
{
public:
	MoreExtended()
	{

	}
	virtual ~MoreExtended()
	{
		RENGINE_ASSERT(virtualFunctionClose() == 30);
		//std::cout << "MoreExtended " << virtualFunctionClose() << std::endl;
	}

	virtual int virtualFunction()
	{
		return 5;
	}


	virtual int virtualFunctionClose()
	{
		return 6 * virtualFunction();
	}
};


//
// UnitTestClasses
//

UNITT_TEST_BEGIN_CLASS(UnitTestClasses)

	virtual void run()
	{
		{
			Base* base = 0;
			SharedPointer<Extended> extended = new Extended();
			base = extended.get();
			UNITT_FAIL_NOT_EQUAL(12,  extended->virtualFunctionClose());
			UNITT_FAIL_NOT_EQUAL(12,  base->virtualFunctionClose());

			SharedPointer<MoreExtended> more_extended = new MoreExtended();
			base = more_extended.get();
			UNITT_FAIL_NOT_EQUAL(30,  more_extended->virtualFunctionClose());
			UNITT_FAIL_NOT_EQUAL(30,  base->virtualFunctionClose());
		}

	}

UNITT_TEST_END_CLASS(UnitTestClasses)
