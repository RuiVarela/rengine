#include "UnitTest/UnitTest.h"

#include <rengine/string/String.h>

#include <string>
#include <iostream>

using namespace std;
using namespace rengine;


//
// UnitTestStringPrinter
//

UNITT_TEST_BEGIN_CLASS(UnitTestStringPrinter)


	virtual void run()
	{
		int const max_buffer_size = 2048;
		char buffer[max_buffer_size];
		buffer[0] = '\n';


		UNITT_FAIL_NOT_EQUAL("hello world!", stringPrinter("hello world!"));
		UNITT_FAIL_NOT_EQUAL("[1] [2] [3]", stringPrinter("[%1%] [%2%] [%3%]", 1, 2, 3));
		UNITT_FAIL_NOT_EQUAL("[one] [two] three.", stringPrinter("[%1%] [%2%] %3%.", "one", "two", "three"));
		UNITT_FAIL_NOT_EQUAL("[1] [2.546] [three]", stringPrinter("[%1%] [%2%] [%3%]", 1, 2.546f, "three"));
		UNITT_FAIL_NOT_EQUAL("[1] [2.546] [three]", stringPrinter("[%1%] [%2%] [%3%]", 1, 2.546, "three"));
		UNITT_FAIL_NOT_EQUAL("% % %", stringPrinter("%% %% %%"));
		UNITT_FAIL_NOT_EQUAL("[1234567891011121314]", stringPrinter("[%d]", Int64(1234567891011121314)));

		typedef std::pair<std::string, float> TestPair; 
		typedef std::vector<TestPair> Tests;

		Real32 value = 732.5467;
		Tests tests;
		tests.push_back( TestPair("%.3f", value) );

		for (Tests::const_iterator i = tests.begin(); i != tests.end(); ++i)
		{
			sprintf(buffer, i->first.c_str(), i->second);
			UNITT_FAIL_NOT_EQUAL(std::string(buffer), stringPrinter(i->first.c_str(), i->second));
		}


		char const* dummy_pointer = "A pointer";
		void* pointer = (void*)dummy_pointer;

		sprintf(buffer, "%p", dummy_pointer);
		UNITT_FAIL_NOT_EQUAL(std::string(buffer), stringPrinter("%1%", pointer));
		UNITT_FAIL_NOT_EQUAL(std::string(buffer), stringPrinter("%p", pointer));


		UNITT_FAIL_NOT_EQUAL("[e][f][10]", stringPrinter("[%x][%x][%x]", 14U, 15U, 16U));
		UNITT_FAIL_NOT_EQUAL("[E][F][10]", stringPrinter("[%X][%X][%X]", 14U, 15U, 16U));
		UNITT_FAIL_NOT_EQUAL("[0XE][0XF][0X10]", stringPrinter("[%#X][%#X][%#X]", 14U, 15U, 16U));
	}

UNITT_TEST_END_CLASS(UnitTestStringPrinter)

//
// UnitTestString
//

UNITT_TEST_BEGIN_CLASS(UnitTestString)

virtual void run()
{
	std::string one_as_string = "1.0";
	float one_as_float = lexical_cast<float>(one_as_string);

	UNITT_FAIL_NOT_EQUAL(1.0f, one_as_float);
	UNITT_FAIL_NOT_EQUAL("2", lexical_cast<std::string>(2.0f));
	UNITT_FAIL_NOT_EQUAL("2.55", lexical_cast<std::string>(2.55f));

	UNITT_FAIL_NOT_EQUAL(3.55, lexical_cast<double>("3.55", 0.0));
	UNITT_FAIL_NOT_EQUAL(0.0, lexical_cast<double>("a3.55", 0.0));

	std::string message = "hello c++, and other worlds++one++two++three++";
	StringElements elements = split(message, " ");
	UNITT_FAIL_NOT_EQUAL("hello", elements[0]);
	UNITT_FAIL_NOT_EQUAL("c++,", elements[1]);
	UNITT_FAIL_NOT_EQUAL("and", elements[2]);
	UNITT_FAIL_NOT_EQUAL("other", elements[3]);
	UNITT_FAIL_NOT_EQUAL("worlds++one++two++three++", elements[4]);

	elements = split(message, ",");
	UNITT_FAIL_NOT_EQUAL("hello c++", elements[0]);
	UNITT_FAIL_NOT_EQUAL(" and other worlds++one++two++three++", elements[1]);

	elements = split(message, "++");
	UNITT_FAIL_NOT_EQUAL("hello c", elements[0]);
	UNITT_FAIL_NOT_EQUAL(", and other worlds", elements[1]);
	UNITT_FAIL_NOT_EQUAL("one", elements[2]);
	UNITT_FAIL_NOT_EQUAL("two", elements[3]);
	UNITT_FAIL_NOT_EQUAL("three", elements[4]);

	message = "   hello   ";

	std::string processed = message;
	trimRight(processed);
	UNITT_FAIL_NOT_EQUAL("   hello", processed);

	processed = message;
	trimLeft(processed);
	UNITT_FAIL_NOT_EQUAL("hello   ", processed);

	processed = message;
	trim(processed);
	UNITT_FAIL_NOT_EQUAL("hello", processed);

	message = "one two three";
	replace(message, "two", "four" );
	UNITT_FAIL_NOT_EQUAL("one four three", message);

	replace(message, "four", "" );
	UNITT_FAIL_NOT_EQUAL("one  three", message);

	uppercase(message);
	UNITT_FAIL_NOT_EQUAL("ONE  THREE", message);

	lowercase(message);
	UNITT_FAIL_NOT_EQUAL("one  three", message);
}

UNITT_TEST_END_CLASS(UnitTestString)


//
// UnitTestStringIsInteger
//

UNITT_TEST_BEGIN_CLASS(UnitTestStringIsInteger)

virtual void run()
{
	UNITT_ASSERT(isInteger("10"));
	UNITT_ASSERT(isInteger("112355"));
	UNITT_ASSERT(isInteger("776213810"));
	UNITT_ASSERT(isInteger("+15"));
	UNITT_ASSERT(isInteger("-10"));

	UNITT_ASSERT(!isInteger("+15+"));
	UNITT_ASSERT(!isInteger("-10-"));
	UNITT_ASSERT(!isInteger("+"));
	UNITT_ASSERT(!isInteger("-"));
	UNITT_ASSERT(!isInteger("1-10"));
	UNITT_ASSERT(!isInteger("1+10"));
	UNITT_ASSERT(!isInteger("a110"));
	UNITT_ASSERT(!isInteger("1a10"));
	UNITT_ASSERT(!isInteger("110a"));
	UNITT_ASSERT(!isInteger("1.10"));
}

UNITT_TEST_END_CLASS(UnitTestStringIsInteger)

//
// UnitTestStringIsReal
//

UNITT_TEST_BEGIN_CLASS(UnitTestStringIsReal)

virtual void run()
{

	UNITT_ASSERT(isReal("1.0"));
	UNITT_ASSERT(isReal("21."));
	UNITT_ASSERT(isReal(".13"));
	UNITT_ASSERT(isReal("123.49"));
	UNITT_ASSERT(isReal("456.78"));
	UNITT_ASSERT(isReal("+12456.78"));
	UNITT_ASSERT(isReal("-12456.78"));
	UNITT_ASSERT(isReal("123110.123410"));

	UNITT_ASSERT(!isReal("--"));
	UNITT_ASSERT(!isReal("+"));
	UNITT_ASSERT(!isReal("."));
	UNITT_ASSERT(!isReal("1"));
	UNITT_ASSERT(!isReal("10"));
	UNITT_ASSERT(!isReal("a110"));
	UNITT_ASSERT(!isReal("1a10"));
	UNITT_ASSERT(!isReal("110a.0"));
	UNITT_ASSERT(!isReal("1+10.0"));
	UNITT_ASSERT(!isReal("1231-10.0"));
	UNITT_ASSERT(!isReal("*123110.123410"));
	UNITT_ASSERT(!isReal("1.23110.123410"));
}

UNITT_TEST_END_CLASS(UnitTestStringIsReal)

//
// UnitTestStringIsBool
//

UNITT_TEST_BEGIN_CLASS(UnitTestStringIsBool)

virtual void run()
{
	UNITT_ASSERT(isBoolean("1"));
	UNITT_ASSERT(isBoolean("0"));

	UNITT_ASSERT(isBoolean("false"));
	UNITT_ASSERT(isBoolean("true"));

	UNITT_ASSERT(!isBoolean("False"));
	UNITT_ASSERT(!isBoolean("True"));
	UNITT_ASSERT(!isBoolean("10"));
	UNITT_ASSERT(!isBoolean("112355"));
	UNITT_ASSERT(!isBoolean("776213810"));
	UNITT_ASSERT(!isBoolean("+15"));
	UNITT_ASSERT(!isBoolean("-10"));
	UNITT_ASSERT(!isBoolean("+15+"));
	UNITT_ASSERT(!isBoolean("-10-"));
	UNITT_ASSERT(!isBoolean("+"));
	UNITT_ASSERT(!isBoolean("-"));
	UNITT_ASSERT(!isBoolean("1-10"));
	UNITT_ASSERT(!isBoolean("1+10"));
	UNITT_ASSERT(!isBoolean("a110"));
	UNITT_ASSERT(!isBoolean("1a10"));
	UNITT_ASSERT(!isBoolean("110a"));
	UNITT_ASSERT(!isBoolean("1.10"));
}

UNITT_TEST_END_CLASS(UnitTestStringIsBool)


//
// UnitTestStringIsNumeric
//

UNITT_TEST_BEGIN_CLASS(UnitTestStringIsNumeric)

virtual void run()
{
	UNITT_ASSERT(isNumber("10"));
	UNITT_ASSERT(isNumber("112355"));
	UNITT_ASSERT(isNumber("776213810"));
	UNITT_ASSERT(isNumber("+15"));
	UNITT_ASSERT(isNumber("-10"));
	UNITT_ASSERT(isNumber("1.0"));
	UNITT_ASSERT(isNumber("21."));
	UNITT_ASSERT(isNumber(".13"));
	UNITT_ASSERT(isNumber("123.49"));
	UNITT_ASSERT(isNumber("456.78"));
	UNITT_ASSERT(isNumber("+12456.78"));
	UNITT_ASSERT(isNumber("-12456.78"));
	UNITT_ASSERT(isNumber("123110.123410"));


	UNITT_ASSERT(!isNumber("+15+"));
	UNITT_ASSERT(!isNumber("-10-"));
	UNITT_ASSERT(!isNumber("+"));
	UNITT_ASSERT(!isNumber("-"));
	UNITT_ASSERT(!isNumber("1-10"));
	UNITT_ASSERT(!isNumber("1+10"));
	UNITT_ASSERT(!isNumber("a110"));
	UNITT_ASSERT(!isNumber("1a10"));
	UNITT_ASSERT(!isNumber("110a"));
	UNITT_ASSERT(!isNumber("--"));
	UNITT_ASSERT(!isNumber("+"));
	UNITT_ASSERT(!isNumber("."));
	UNITT_ASSERT(!isNumber("a110"));
	UNITT_ASSERT(!isNumber("1a10"));
	UNITT_ASSERT(!isNumber("110a.0"));
	UNITT_ASSERT(!isNumber("1+10.0"));
	UNITT_ASSERT(!isNumber("1231-10.0"));
	UNITT_ASSERT(!isNumber("*123110.123410"));
	UNITT_ASSERT(!isNumber("1.23110.123410"));
	UNITT_ASSERT(!isNumber("false"));
	UNITT_ASSERT(!isNumber("true"));
	UNITT_ASSERT(!isNumber("False"));
	UNITT_ASSERT(!isNumber("True"));
}

UNITT_TEST_END_CLASS(UnitTestStringIsNumeric)


//
// UnitTestStringStartsWith
//

UNITT_TEST_BEGIN_CLASS(UnitTestStringStartsWith)

	virtual void run()
	{
		UNITT_ASSERT(startsWith("prefix$body", "prefix"));
		UNITT_ASSERT(!startsWith("aprefix$body", "prefix"));
		UNITT_ASSERT(startsWith("$prefix$body", "$prefix"));
		UNITT_ASSERT(!startsWith(" prefix$body", "$prefix"));
		UNITT_ASSERT(!startsWith(" prefix$body", "prefix"));
		UNITT_ASSERT(!startsWith("", "prefix"));

	}

UNITT_TEST_END_CLASS(UnitTestStringStartsWith)


//
// UnitTestStringEndsWith
//

UNITT_TEST_BEGIN_CLASS(UnitTestStringEndsWith)

	virtual void run()
	{
		UNITT_ASSERT(endsWith("prefix$body$sufix", "sufix"));
		UNITT_ASSERT(!endsWith("aprefix$sufixc", "sufix"));
		UNITT_ASSERT(endsWith("$prefix$bodydsufix$", "sufix$"));
		UNITT_ASSERT(!endsWith(" prefix$body ", "prefix"));
		UNITT_ASSERT(endsWith(" prefix$body prefix ", "prefix "));
		UNITT_ASSERT(!endsWith("", "prefix "));
	}

UNITT_TEST_END_CLASS(UnitTestStringEndsWith)
