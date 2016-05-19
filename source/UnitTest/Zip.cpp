#include "UnitTest/UnitTest.h"

#include <rengine/file/Zip.h>

#include <algorithm>
#include <string>

using namespace std;
using namespace rengine;

//
// UnitTestUnzip
//

UNITT_TEST_BEGIN_CLASS(UnitTestUnzip)

virtual void run()
{
	std::string filename("unit_test_data/file_zipped.zip");

	Zip zip;
	UNITT_ASSERT( zip.load(filename) );


	Zip::FileData data = zip.read("unit_test_data/File/one_text_file.txt");
	UNITT_ASSERT(data.data);
	UNITT_ASSERT(data.size > 0);
	if (data.data)
	{
		std::string data_as_string(data.data.get(), data.data.get() + data.size);
		UNITT_FAIL_NOT_EQUAL("hello world\n", data_as_string);
	}

	data = zip.read("unit_test_data/Shader/Simple.eff");
	UNITT_ASSERT(data.data);
	UNITT_ASSERT(data.size > 0);

	std::string file_path = "unit_test_data/File";
	UNITT_ASSERT(zip.fileType(file_path) == FileDirectory);

	DirectoryContents contents = zip.getDirectoryContents(file_path);

	UNITT_FAIL_NOT_EQUAL(4 + 2, contents.size());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), ".") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "..") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "One") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "Tw o") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "one_text_file.txt") != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), "another file.txt") != contents.end());

	UNITT_ASSERT(zip.fileType("unit_test_data/File/One") == FileDirectory);
	UNITT_ASSERT(zip.fileType("unit_test_data/File/Tw o") == FileDirectory);
	UNITT_ASSERT(zip.fileType("unit_test_data/File/one_text_file.txt") == FileRegular);
	UNITT_ASSERT(zip.fileType("unit_test_data/File/another file.txt") == FileRegular);

	contents = find(file_path, EqualExtension("txt"), false, zip);
	UNITT_FAIL_NOT_EQUAL(2, contents.size());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), convertFileNameToNativeStyle("unit_test_data/File/one_text_file.txt")) != contents.end());
	UNITT_ASSERT(std::find(contents.begin(), contents.end(), convertFileNameToNativeStyle("unit_test_data/File/another file.txt")) != contents.end());


	//UNITT_FAIL_NOT_EQUAL(convertFileNameToUnixStyle(test_filename), test_filename_unix);
}
UNITT_TEST_END_CLASS(UnitTestUnzip)
