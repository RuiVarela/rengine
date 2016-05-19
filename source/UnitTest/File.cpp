#include "UnitTest/UnitTest.h"

#include <rengine/file/File.h>

#include <algorithm>

using namespace std;
using namespace rengine;

std::string const test_filename("unit_test_data/xml_test.xml");
std::string const test_filename_unix(test_filename);
std::string const test_filename_windows("unit_test_data\\xml_test.xml");

//
// UnitTestFileFilename
//

UNITT_TEST_BEGIN_CLASS(UnitTestFileFilename)

virtual void run()
{
	UNITT_FAIL_NOT_EQUAL(convertFileNameToUnixStyle(test_filename), test_filename_unix);
	UNITT_FAIL_NOT_EQUAL(convertFileNameToWindowsStyle(test_filename), test_filename_windows);

	UNITT_FAIL_NOT_EQUAL(convertFileNameToUnixStyle(test_filename_windows), test_filename_unix);
	UNITT_FAIL_NOT_EQUAL(convertFileNameToWindowsStyle(test_filename_unix), test_filename_windows);

#if RENGINE_PLATFORM == RENGINE_PLATFORM_WIN32
	UNITT_FAIL_IF( isFileNameNativeStyle(test_filename), "Filename should not be in native style" );
	UNITT_FAIL_NOT_EQUAL(convertFileNameToNativeStyle(test_filename), test_filename_windows);
#else //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32
	UNITT_FAIL_IF( !isFileNameNativeStyle(test_filename), "Filename should be in native style" );
	UNITT_FAIL_NOT_EQUAL(convertFileNameToNativeStyle(test_filename), test_filename_unix);
#endif //RENGINE_PLATFORM != RENGINE_PLATFORM_WIN32

}

UNITT_TEST_END_CLASS(UnitTestFileFilename)


//
// UnitTestFilePath
//

UNITT_TEST_BEGIN_CLASS(UnitTestFilePath)

virtual void run()
{
	UNITT_FAIL_NOT_EQUAL("unit_test_data", getFilePath(test_filename));
	UNITT_FAIL_NOT_EQUAL("xml", getFileExtension(test_filename));
	UNITT_FAIL_NOT_EQUAL("XmL", getFileExtension("unit_test_data/xmlss.XmL"));
	UNITT_FAIL_NOT_EQUAL("xml", getLowerCaseFileExtension("unit_test_data/xmlss.XmL"));
	UNITT_FAIL_NOT_EQUAL("xml_test.xml", getSimpleFileName(test_filename));
	UNITT_FAIL_NOT_EQUAL("unit_test_data/xml_test", getNameLessExtension(test_filename));
	UNITT_FAIL_NOT_EQUAL("xml_test", getStrippedName(test_filename));
}

UNITT_TEST_END_CLASS(UnitTestFilePath)


//
// UnitTestFileDirectory
//

UNITT_TEST_BEGIN_CLASS(UnitTestFileDirectory)

virtual void run()
{
	std::string file_path = convertFileNameToNativeStyle("unit_test_data/File");
	UNITT_ASSERT(fileType(file_path) == FileDirectory);

	DirectoryContents contents = getDirectoryContents(file_path);

	UNITT_FAIL_NOT_EQUAL(4 + 2, contents.size());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), ".") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "..") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "One") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "Tw o") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "one_text_file.txt") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "another file.txt") != contents.end());

	UNITT_ASSERT(fileType("unit_test_data/File/One") == FileDirectory);
	UNITT_ASSERT(fileType("unit_test_data/File/Tw o") == FileDirectory);
	UNITT_ASSERT(fileType("unit_test_data/File/one_text_file.txt") == FileRegular);
	UNITT_ASSERT(fileType("unit_test_data/File/another file.txt") == FileRegular);

	contents = find(file_path, EqualExtension("txt"));
	UNITT_FAIL_NOT_EQUAL(2, contents.size());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), convertFileNameToNativeStyle("unit_test_data/File/one_text_file.txt")) != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), convertFileNameToNativeStyle("unit_test_data/File/another file.txt")) != contents.end());
}

UNITT_TEST_END_CLASS(UnitTestFileDirectory)
