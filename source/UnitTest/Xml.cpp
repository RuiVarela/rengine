#include "UnitTest/UnitTest.h"

#include <rengine/file/XmlSerialization.h>

#include <string>

using namespace std;


#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32
std::string const test_filename("unit_test_data\\xml_test.xml");
#else //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32
std::string const test_filename("unit_test_data/xml_test.xml");
#endif //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32

class MyType
{
public:
	MyType()
	{
		one = 1;
		two = 2.2f;
		three = 3.3;
		four = "4";
	}

	bool isValid()
	{
		bool state = true;

		state &= (one == 1);
		state &= (two == 2.2f);
		state &= (three == 3.3);
		state &= (four == "4");

		return state;
	}

	void clear()
	{
		one = 0;
		two = 0;
		three = 0;
		four = "";
	}

	int one;
	float two;
	double three;
	std::string four;
};

//
// Serialization method outside the class.
//
namespace rengine
{
	namespace serialization
	{
		template <>
		void serialize<MyType>(XmlArchive& archive, MyType& test)
		{
			XML_SERIALIZE(archive, test.one);
			XML_SERIALIZE(archive, test.two);
			XML_SERIALIZE(archive, test.three);
			XML_SERIALIZE(archive, test.four);
		}
	}
}


//
// Serialization method inside the class.
//

class MySecondType
{
public :
	MySecondType(bool do_clear = true)
	{
		one = 101;
		two = 102.2f;
		three = 10973.63567;
		four = "104";

		if (do_clear)
		{
			clear();
		}
	}

	bool isValid()
	{
		bool state = true;

		state &= (one == 101);
		state &= (two == 102.2f);
		state &= (three == 10973.63567);
		state &= (four == "104");

		state &= my_test.isValid();

		return state;
	}

	void clear()
	{
		one = 0;
		two = 0;
		three = 0;
		four = "";

		my_test.clear();
	}

	int one;
	float two;
	double three;
	std::string four;

	MyType my_test;


	//
	// Serialization method inside the class.
	//
	void serialize(rengine::XmlArchive& archive)
	{
		XML_SERIALIZE(archive, one);
		XML_SERIALIZE(archive, two);
		XML_SERIALIZE(archive, three);
		XML_SERIALIZE(archive, four);
		XML_SERIALIZE(archive, my_test);
	}
};

//
// UnitTestXmlWriting
//

UNITT_TEST_BEGIN_CLASS(UnitTestXmlWriting)

virtual void run()
{
	rengine::XmlArchive archive;
	UNITT_ASSERT(archive.open(test_filename, rengine::XmlArchive::Write));

	int my_integer = 2;
	double my_double = 23.67;
	float my_float = 45.0089f;
	std::string my_string = "my string!";
	MyType my_test;
	MySecondType my_second_test(false);

	rengine::serialize(archive, "my_named_integer", my_integer); //manually naming
	XML_SERIALIZE(archive, my_integer); // auto naming
	XML_SERIALIZE(archive, my_double);
	XML_SERIALIZE(archive, my_float);
	XML_SERIALIZE(archive, my_string);
	XML_SERIALIZE(archive, my_test);
	XML_SERIALIZE(archive, my_second_test);

	std::vector<int> int_vector;
	std::list<int> int_list;
	std::vector<MySecondType> my_second_type_vector;
	unsigned int const max = 10;

	for (unsigned int i = 0; i != max; ++i)
	{
		int_vector.push_back(max - i);
		int_list.push_back(max - i);
		my_second_type_vector.push_back(MySecondType(false));
	}

	XML_SERIALIZE(archive, int_vector);
	XML_SERIALIZE(archive, int_list);
	XML_SERIALIZE(archive, my_second_type_vector);


	std::map<std::string, int> int_map;
	int_map["zero"] = 0;
	int_map["one"] = 1;
	int_map["two"] = 2;
	int_map["three"] = 3;

	std::map<std::string, MySecondType> my_second_map;
	my_second_map["four"] = MySecondType(false);
	my_second_map["five"] = MySecondType(false);
	my_second_map["six"] = MySecondType(false);
	my_second_map["seven"] = MySecondType(false);

	XML_SERIALIZE(archive, int_map);
	XML_SERIALIZE(archive, my_second_map);

	UNITT_ASSERT(archive.save());
}

UNITT_TEST_END_CLASS(UnitTestXmlWriting)


//
// UnitTestXmlReading
//

UNITT_TEST_BEGIN_CLASS(UnitTestXmlReading)

virtual void run()
{
	int my_integer = 0;
	double my_double = 0;
	float my_float = 0;
	std::string my_string;

	rengine::XmlArchive archive;
	UNITT_ASSERT(archive.open(test_filename));

	rengine::serialize(archive, "my_named_integer", my_integer); //manually naming
	UNITT_ASSERT(my_integer == 2);
	my_integer = 0;
	MyType my_test;
	MySecondType my_second_test;

	my_test.clear();
	my_second_test.clear();

	XML_SERIALIZE(archive, my_integer); // auto naming
	UNITT_ASSERT(my_integer == 2);
	XML_SERIALIZE(archive, my_double);
	UNITT_ASSERT(my_double == 23.67);
	XML_SERIALIZE(archive, my_float);
	UNITT_ASSERT(my_float == 45.0089f);
	XML_SERIALIZE(archive, my_string);
	UNITT_ASSERT(my_string == "my string!");

	XML_SERIALIZE(archive, my_test);
	XML_SERIALIZE(archive, my_second_test);

	UNITT_ASSERT(my_test.isValid());
	UNITT_ASSERT(my_second_test.isValid());

	std::vector<int> int_vector;
	std::list<int> int_list;
	std::vector<MySecondType> my_second_type_vector;
	unsigned int const max = 10;

	XML_SERIALIZE(archive, int_vector);
	XML_SERIALIZE(archive, int_list);
	XML_SERIALIZE(archive, my_second_type_vector);

	for (unsigned int i = 0; i != max; ++i)
	{
		UNITT_ASSERT(int_vector[i] == int(max - i));

		UNITT_ASSERT(my_second_type_vector[i].isValid());
	}


	unsigned int counter = 0;
	for (std::list<int>::iterator iterator = int_list.begin();
		iterator != int_list.end();
		++iterator)
	{
		UNITT_ASSERT(*iterator == int(max - counter));
		++counter;
	}


	std::map<std::string, int> int_map;
	XML_SERIALIZE(archive, int_map);
	UNITT_ASSERT(int_map.find("zero") != int_map.end());
	if (int_map.find("zero") != int_map.end())
	{
		UNITT_FAIL_NOT_EQUAL(0, int_map["zero"]);
	}

	UNITT_ASSERT(int_map.find("one") != int_map.end());
	if (int_map.find("one") != int_map.end())
	{
		UNITT_FAIL_NOT_EQUAL(1, int_map["one"]);
	}

	UNITT_ASSERT(int_map.find("two") != int_map.end());
	if (int_map.find("two") != int_map.end())
	{
		UNITT_FAIL_NOT_EQUAL(2, int_map["two"]);
	}

	UNITT_ASSERT(int_map.find("three") != int_map.end());
	if (int_map.find("three") != int_map.end())
	{
		UNITT_FAIL_NOT_EQUAL(3, int_map["three"]);
	}


	std::map<std::string, MySecondType> my_second_map;
	XML_SERIALIZE(archive, my_second_map);
	UNITT_ASSERT(my_second_map.find("four") != my_second_map.end());
	if (my_second_map.find("four") != my_second_map.end())
	{
		UNITT_ASSERT(my_second_map["four"].isValid());
	}

	UNITT_ASSERT(my_second_map.find("five") != my_second_map.end());
	if (my_second_map.find("five") != my_second_map.end())
	{
		UNITT_ASSERT(my_second_map["five"].isValid());
	}

	UNITT_ASSERT(my_second_map.find("six") != my_second_map.end());
	if (my_second_map.find("six") != my_second_map.end())
	{
		UNITT_ASSERT(my_second_map["six"].isValid());
	}

	UNITT_ASSERT(my_second_map.find("seven") != my_second_map.end());
	if (my_second_map.find("seven") != my_second_map.end())
	{
		UNITT_ASSERT(my_second_map["seven"].isValid());
	}
}

UNITT_TEST_END_CLASS(UnitTestXmlReading)
