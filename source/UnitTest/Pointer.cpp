#include "UnitTest/UnitTest.h"

#include <rengine/lang/Lang.h>

#include <string>
#include <iostream>

using namespace std;
using namespace rengine;

class MyTest
{
public:
	MyTest(std::string const& name)
		:my_name(name)
	{
		//std::cout << "Contructor : " << my_name << std::endl;
	}

	virtual void speak()
	{
		//std::cout << "My Name is : " << my_name << std::endl;
	}

	virtual ~MyTest()
	{
		//std::cout << "Destructor : " << my_name << std::endl;
	}

	std::string my_name;
};

class MyTestChild : public MyTest
{
public:
	MyTestChild(std::string const& name)
		:MyTest(name)
	{}

	virtual ~MyTestChild() {}
private:
};

bool testByCopy(SharedPointer<MyTest> by_copy, unsigned int count)
{
	return (by_copy.referenceCount() == count);
}

bool testByRef(SharedPointer<MyTest>& ref, unsigned int count)
{
	ref->speak();

	return (ref.referenceCount() == count);
}

bool testByConstRef(SharedPointer<MyTest> const& ref, unsigned int count)
{
	ref->speak();
	return (ref.referenceCount() == count);
}

SharedPointer<MyTest> testReturn()
{
	SharedPointer<MyTest> my_test = new MyTest("MyTest");
	return my_test;
}

//
// UnitTestSharePointer
//

UNITT_TEST_BEGIN_CLASS(UnitTestSharePointer)

virtual void run()
{
	MyTest one("one");

	MyTest *two = new MyTest("two");
	SharedPointer<MyTest> share_pointer_two(two);
	UNITT_FAIL_NOT_EQUAL(1, share_pointer_two.referenceCount());

	SharedPointer<MyTest> three( new MyTest("three") );
	SharedPointer<MyTest> four( new MyTest("four") );
	UNITT_FAIL_NOT_EQUAL(1, four.referenceCount());

	{
		SharedPointer<MyTest> four_copy(four);
		UNITT_FAIL_NOT_EQUAL(four.referenceCount() , four_copy.referenceCount());
		UNITT_FAIL_NOT_EQUAL(2, four.referenceCount());

		UNITT_ASSERT(four.get() != three);
		UNITT_ASSERT(three != four.get());
		UNITT_ASSERT(three != four.operator ->());
		UNITT_ASSERT(three != four);

		UNITT_ASSERT(four_copy.get() == four);
		UNITT_ASSERT(four == four_copy.get());
		UNITT_ASSERT(four == four_copy.operator ->());
		UNITT_ASSERT(four == four_copy);
	}

	UNITT_FAIL_NOT_EQUAL(1, four.referenceCount());

	SharedPointer<MyTest> five( new MyTest("five") );
	UNITT_FAIL_NOT_EQUAL(1, five.referenceCount());

	SharedPointer<MyTest> six( new MyTest("six") );
	SharedPointer<MyTest> seven;
	six = five;
	seven = six;

	UNITT_FAIL_NOT_EQUAL(3, five.referenceCount());

	six = new MyTest("Plain Six.");
	UNITT_FAIL_NOT_EQUAL(2, five.referenceCount());

	six = 0;
	UNITT_FAIL_NOT_EQUAL(1, six.referenceCount());
}

UNITT_TEST_END_CLASS(UnitTestSharePointer)


//
// UnitTestSharePointerFunction
//

UNITT_TEST_BEGIN_CLASS(UnitTestSharePointerFunction)

virtual void run()
{
	// passing pointer to functions...
	{
		SharedPointer<MyTest> param_0 = new MyTest("parameter 0");
		UNITT_FAIL_NOT_EQUAL(1, param_0.referenceCount());

		UNITT_ASSERT(testByCopy(param_0, 2));
		UNITT_ASSERT(testByRef(param_0, 1));
		UNITT_ASSERT(testByConstRef(param_0, 1));

		SharedPointer<MyTest> returned = testReturn();
		returned->speak();
		UNITT_FAIL_NOT_EQUAL(1, returned.referenceCount());
	}

	{
		SharedPointer<MyTestChild> child( new MyTestChild("child") );
		SharedPointer<MyTest> father( new MyTest("father") );

		SharedPointer<MyTest> conversion_test_0 = child;
		UNITT_FAIL_NOT_EQUAL(2, child.referenceCount());
		SharedPointer<MyTest> conversion_test_1;
		conversion_test_1 = child;
		UNITT_FAIL_NOT_EQUAL(3, child.referenceCount());

		SharedPointer<MyTestChild> conversion_test_2;

		conversion_test_2 = dynamic_pointer_cast<MyTestChild>(conversion_test_1);
		UNITT_FAIL_NOT_EQUAL(4, child.referenceCount());
		conversion_test_1 = 0;
		UNITT_FAIL_NOT_EQUAL(3, child.referenceCount());

	}
}

UNITT_TEST_END_CLASS(UnitTestSharePointerFunction)
