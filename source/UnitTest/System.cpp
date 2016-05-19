#include "UnitTest/UnitTest.h"

#include <rengine/system/System.h>
#include <rengine/outputstream/OutputStream.h>


#include <string>
#include <iostream>
using namespace rengine;
using namespace std;
//
// UnitTestSystemVariable
//

UNITT_TEST_BEGIN_CLASS(UnitTestSystemVariable)

virtual void run()
{

	OutputStream my_stream;

	my_stream << "1234567890abcdefghijklmnopkrstuv" << std::endl;



	Variable int_variable0;
	Variable float_variable0(Variable::FloatType);
	Variable bool_variable0(Variable::BoolType);
	Variable string_variable0(Variable::StringType);

	int var_int = 0;
	float var_float = 0.0f;
	bool var_bool = false;
	string var_string = "";

	UNITT_FAIL_NOT_EQUAL(int_variable0.type(), Variable::IntType);
	UNITT_FAIL_NOT_EQUAL(float_variable0.type(), Variable::FloatType);
	UNITT_FAIL_NOT_EQUAL(bool_variable0.type(), Variable::BoolType);
	UNITT_FAIL_NOT_EQUAL(string_variable0.type(), Variable::StringType);

	UNITT_FAIL_NOT_EQUAL(int_variable0.asInt(), var_int);
	UNITT_FAIL_NOT_EQUAL(int(int_variable0), var_int);
	UNITT_FAIL_NOT_EQUAL(float_variable0.asFloat(), var_float);
	UNITT_FAIL_NOT_EQUAL(float(float_variable0), var_float);
	UNITT_FAIL_NOT_EQUAL(bool_variable0.asBool(), var_bool);
	UNITT_FAIL_NOT_EQUAL(bool(bool_variable0), var_bool);
	UNITT_FAIL_NOT_EQUAL(string_variable0.asString(), var_string);
	UNITT_FAIL_NOT_EQUAL(string(string_variable0), var_string);

	UNITT_FAIL_NOT_EQUAL(int_variable0.toString(), "0");
	UNITT_FAIL_NOT_EQUAL(float_variable0.toString(), "0");
	UNITT_FAIL_NOT_EQUAL(bool_variable0.toString(), "false");
	UNITT_FAIL_NOT_EQUAL(string_variable0.toString(), "");

	var_int = 3;
	var_float = 4.78f;
	var_bool = true;
	var_string = "hello world";

	int_variable0 = var_int;
	float_variable0 = var_float;
	bool_variable0 = var_bool;
	string_variable0 = var_string;

	UNITT_FAIL_NOT_EQUAL(int_variable0.asInt(), var_int);
	UNITT_FAIL_NOT_EQUAL(int(int_variable0), var_int);
	UNITT_FAIL_NOT_EQUAL(float_variable0.asFloat(), var_float);
	UNITT_FAIL_NOT_EQUAL(float(float_variable0), var_float);
	UNITT_FAIL_NOT_EQUAL(bool_variable0.asBool(), var_bool);
	UNITT_FAIL_NOT_EQUAL(bool(bool_variable0), var_bool);
	UNITT_FAIL_NOT_EQUAL(string_variable0.asString(), var_string);
	UNITT_FAIL_NOT_EQUAL(string(string_variable0), var_string);

	UNITT_FAIL_NOT_EQUAL(int_variable0.toString(), "3");
	UNITT_FAIL_NOT_EQUAL(float_variable0.toString(), "4.78");
	UNITT_FAIL_NOT_EQUAL(bool_variable0.toString(), "true");
	UNITT_FAIL_NOT_EQUAL(string_variable0.toString(), "hello world");

}

UNITT_TEST_END_CLASS(UnitTestSystemVariable)
