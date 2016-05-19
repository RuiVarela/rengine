#include "UnitTest/UnitTest.h"

#include <rengine/util/Any.h>
#include <rengine/util/OpaqueProperty.h>

#include <list>
#include <string>
#include <algorithm>

using namespace rengine;

typedef std::list<Any> Many;

void append_int(Many& values, int value)
{
	Any to_append = value;
    values.push_back(to_append);
}

void append_string(Many& values, std::string const& value)
{
    values.push_back(value);
}

void append_char_ptr(Many& values, char const* value)
{
    values.push_back(value);
}

void append_any(Many& values, Any const& value)
{
    values.push_back(value);
}

void append_nothing(Many& values)
{
    values.push_back(Any());
}


bool is_empty(Any const& operand)
{
    return operand.empty();
}

bool is_int(Any const& operand)
{
    return operand.sameType<int>();
}

bool is_double(Any const& operand)
{
    return operand.sameType<double>();
}

bool is_string(Any const& operand)
{
    return operand.sameType<std::string>();
}

bool is_char_ptr(Any const& operand)
{
    try
    {
        any_cast<char const*>(operand);
        return true;
    }
    catch(std::bad_cast const&)
    {
        return false;
    }
}


//
// UnitTestAnyType
//

UNITT_TEST_BEGIN_CLASS(UnitTestAnyType)

	virtual void run()
	{
		Any any_int_0(1);
		UNITT_ASSERT( any_int_0.sameType<int>() );
		UNITT_ASSERT( !any_int_0.sameType<float>() );
		UNITT_ASSERT( !any_int_0.sameType<double>() );
		UNITT_ASSERT( !any_int_0.sameType<std::string>() );
		UNITT_ASSERT( any_cast<int>(any_int_0) == 1);

		Any any_float_0(1.0f);
		UNITT_ASSERT( any_float_0.sameType<float>() );
		UNITT_ASSERT( any_cast<float>(any_float_0) == 1.0f);

		Any any_double_0(1.0);
		UNITT_ASSERT( any_double_0.sameType<double>() );
		UNITT_ASSERT( any_cast<double>(any_double_0) == 1.0);

		Any any_string_0(std::string("hello_world"));
		UNITT_ASSERT( any_string_0.sameType<std::string>() );
		UNITT_ASSERT( any_cast<std::string>(any_string_0) == std::string("hello_world"));

		Any any_string_1;
		any_string_1 = std::string("new hello_world");

		UNITT_ASSERT( any_string_1.sameType<std::string>() );
		UNITT_ASSERT( any_cast<std::string>(any_string_1) == std::string("new hello_world"));

		Any any_c_string_0("olá_mundo");
		UNITT_ASSERT( any_c_string_0.sameType<std::string>() );
		UNITT_ASSERT( any_cast<std::string>(any_c_string_0) == std::string("olá_mundo"));


		try
		{
			int x = any_cast<int>(any_string_1);
			x = 0;
			UNITT_FAIL("bad cast should have thrown a exception.");
		}
		catch(std::bad_cast)
		{
		}

		Any any_double_1(1523.0);
		Any any_double_2 = 12333.0;
		Any any_double_3;
		any_double_3 = any_double_1;

		UNITT_ASSERT(any_cast<double>(any_double_3) == any_cast<double>(any_double_1));
	}

UNITT_TEST_END_CLASS(UnitTestAnyType)

//
// UnitTestAnyOnContainers
//

UNITT_TEST_BEGIN_CLASS(UnitTestAnyOnContainers)

	virtual void run()
	{
		Many many;

		append_nothing(many);

		append_int(many, 1);
		append_string(many, "One");

		append_int(many, 2);
		append_string(many, "Two");

		append_int(many, 3);
		append_string(many, "Three");

		append_int(many, 4);
		append_string(many, "Four");

		append_char_ptr(many, "char pointer");
		append_any(many, Any(34.65));

		append_nothing(many);

		unsigned int count = 0;

		count = std::count_if(many.begin(), many.end(), is_empty);
		UNITT_FAIL_NOT_EQUAL(2, count);

		count = std::count_if(many.begin(), many.end(), is_string);
		UNITT_FAIL_NOT_EQUAL(4, count);

		count = std::count_if(many.begin(), many.end(), is_char_ptr);
		UNITT_FAIL_NOT_EQUAL(1, count);

		count = std::count_if(many.begin(), many.end(), is_int);
		UNITT_FAIL_NOT_EQUAL(4, count);

		count = std::count_if(many.begin(), many.end(), is_double);
		UNITT_FAIL_NOT_EQUAL(1, count);

	}

UNITT_TEST_END_CLASS(UnitTestAnyOnContainers)


//
// UnitTestOpaqueProperty
//

UNITT_TEST_BEGIN_CLASS(UnitTestOpaqueProperty)

	virtual void run()
	{
		OpaqueProperty property_0;
		OpaqueProperty property_1("name_0");
		OpaqueProperty property_2(3);
		OpaqueProperty property_3("name_1", 1234.5f);
		OpaqueProperty property_4("name_2", "value");
		OpaqueProperty property_5 = property_4;

		UNITT_ASSERT(property_0.value.empty());
		UNITT_ASSERT(property_1.value.empty());
		UNITT_ASSERT(!property_2.value.empty());
		UNITT_ASSERT(!property_3.value.empty());
		UNITT_ASSERT(!property_4.value.empty());
		UNITT_ASSERT(!property_5.value.empty());

		UNITT_ASSERT(property_3.value.sameType<float>());

		UNITT_ASSERT(!property_4.value.sameType<char const*>());
		UNITT_ASSERT(property_4.value.sameType<std::string>());

		OpaqueProperties properties;

		properties.push_back(property_0);
		properties.push_back(property_1);
		properties.push_back(property_2);
		properties.push_back(property_3);
		properties.push_back(property_4);
		properties.push_back(property_5);

		UNITT_ASSERT(!properties.hasProperty("nothing"));
		UNITT_ASSERT(properties.hasProperty(""));
		UNITT_ASSERT(properties.hasProperty("name_0"));
		UNITT_ASSERT(properties.hasProperty("name_1"));
		UNITT_ASSERT(properties.hasProperty("name_2"));

		UNITT_ASSERT(properties["name_0"].value.empty());
		UNITT_ASSERT(!properties["name_1"].value.empty());
		UNITT_ASSERT(!properties["name_2"].value.empty());

		UNITT_ASSERT(properties[0].value.empty());
		UNITT_ASSERT(properties[1].value.empty());

		UNITT_ASSERT( properties[2].value.sameType<int>() );
		UNITT_ASSERT( properties[3].value.sameType<float>() );
		UNITT_ASSERT( properties[4].value.sameType<std::string>() );


		UNITT_FAIL_NOT_EQUAL(any_cast<int>(properties[2].value), 3);
		UNITT_FAIL_NOT_EQUAL(any_cast<float>(properties[3].value), 1234.5f);
		UNITT_FAIL_NOT_EQUAL(any_cast<std::string>(properties[4].value), "value");
		UNITT_FAIL_NOT_EQUAL(any_cast<std::string>(properties[5].value), "value");

		properties["name_0"].name = "new_name";
		UNITT_ASSERT(properties.hasProperty("new_name"));
	}

UNITT_TEST_END_CLASS(UnitTestOpaqueProperty)
