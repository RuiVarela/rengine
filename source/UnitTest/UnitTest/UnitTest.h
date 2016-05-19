// __!!ryven_copyright!!__ //

#ifndef UNITTEST_H_
#define UNITTEST_H_

#include <vector>
#include <string>
#include <sstream>

namespace UnitTest
{
	//
	// TestResult
	//

	struct Location
	{
		Location() :
			line_number(0)
		{
		}

		Location(int const line, std::string const& file) :
			line_number(line), filename(file)
		{
		}

		~Location()
		{
		}

		int line_number;
		std::string filename;
	};

	struct TestResult
	{
		TestResult() :
			passed(true)
		{
		}

		TestResult(bool const test_passed) :
			passed(test_passed)
		{
		}

		TestResult(std::string const& test_message, Location const& test_location) :
			passed(false), message(test_message), location(test_location)
		{
		}

		TestResult(Location const& test_location) :
			passed(false), location(test_location)
		{
		}

		virtual ~TestResult()
		{
		}

		virtual std::string value() const;

		bool passed;
		std::string message;
		Location location;
	};

	//
	// TestCase
	//

	class TestCase
	{
	public:
		TestCase();
		virtual ~TestCase();

		virtual std::string name() const { return "TestCase"; }

		virtual void pre();
		virtual void run();
		virtual void post();

		TestResult const& result() { return test_result; }
		void tryAndChangeResult(TestResult const& test_result);

		// results
		void passIf(bool condition, Location const& location);

		void fail(TestResult result);
		void failIf(bool condition, TestResult result);

		template <typename T>
		std::string asString(T const& value)
		{
			std::stringstream string_stream;
			string_stream << value;
			return string_stream.str();
		}

		void failNotEqual(std::string const& expected, std::string const& actual, Location const& location);
		void failNotEqual(int const& expected, int const& actual, Location const& location)			{ return failNotEqual<int>(expected, actual, location); }
		void failNotEqual(float const& expected, float const& actual, Location const& location)		{ return failNotEqual<float>(expected, actual, location); }

		template <typename T>
		void failNotEqual(T expected, T actual, Location const& location)
		{
			if (expected != actual)
			{
				createFailNotEqual(asString(expected), asString(actual), location);
			}
		}
		

	protected:
		void createFailNotEqual(std::string const& expected, std::string const& actual, Location const& location);
		TestResult test_result;
	};

	//
	// UnitTester
	//

	class UnitTester
	{
	public:
		typedef std::vector<TestCase*> TestCases;

		UnitTester();
		virtual ~UnitTester();

		void addTestCase(TestCase* test_case);

		virtual void reportTest(TestCase* test_case);
		virtual void informStats();

		void report();
		void test();
		void run();

		static UnitTester* instance();
	private:
		TestCases test_cases;
	};

	//
	// TestCaseAutoRegister
	//

	template <typename T>
	struct TestCaseAutoRegister
	{
		TestCaseAutoRegister()
		{
			UnitTester::instance()->addTestCase(new T());
		}
		~TestCaseAutoRegister()
		{

		}
	};
}

//
// Helper Macros
//

#define UNITT_NAME_TEST_CASE(test_name) \
	virtual std::string name() const { return #test_name; }

#define UNITT_TEST_ADD(test_class) \
	static UnitTest::TestCaseAutoRegister<test_class> test##test_class;

#define UNITT_TEST_BEGIN_CLASS(test_class) \
	struct test_class : public UnitTest::TestCase { \
	UNITT_NAME_TEST_CASE(test_class)

#define UNITT_TEST_END_CLASS(test_class) \
	}; \
	UNITT_TEST_ADD(test_class)

#define UNITT_TEST_BEGIN(test_class) \
	UNITT_TEST_BEGIN_CLASS(test_class) \
	virtual void run() {

#define UNITT_TEST_END(test_class) \
	} \
	UNITT_TEST_END_CLASS(test_class)


//
// Assertion
//

#define UNITT_LOCATION() UnitTest::Location( __LINE__, __FILE__ )
#define UNITT_ASSERT(condition) passIf(condition, UNITT_LOCATION())
#define UNITT_FAIL(message) fail(UnitTest::TestResult(message, UNITT_LOCATION()))
#define UNITT_FAIL_IF(condition, message) failIf(condition, UnitTest::TestResult(message, UNITT_LOCATION()))
#define UNITT_FAIL_NOT_EQUAL(expected, actual) failNotEqual(expected, actual, UNITT_LOCATION())


#endif /* UNITTEST_H_ */
