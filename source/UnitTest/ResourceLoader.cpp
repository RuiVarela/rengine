#include "UnitTest/UnitTest.h"

#include <rengine/resource/ResourceLoader.h>
#include <rengine/string/String.h>
#include <rengine/file/File.h>

using namespace rengine;

class IntResouceLoader : public ResourceLoader<int>
{
public:
	virtual bool suportsFormat(std::string const& extension) const
	{
		return (extension == "int");
	}

	// not needed to overwrite if loading from disk
	virtual bool canLoadResourceFromLocation(std::string const& resource_location) const
	{
		return (suportsFormat( getFileExtension(resource_location) ) /*&& fileExists(resource_location)*/);
	}

	virtual SharedPointer<int> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties())
	{
		std::string simple_filename = getStrippedName(location);

		if (simple_filename == "one")
		{
			return new int(1);
		}
		else if (simple_filename == "two")
		{
			return new int(2);
		}
		else if (simple_filename == "three")
		{
			return new int(3);
		}
		else if (simple_filename == "four")
		{
			return new int(4);
		}

		return SharedPointer<int>();
	}
};

//
// UnitTestResourceLoader
//

UNITT_TEST_BEGIN_CLASS(UnitTestResourceLoader)

	virtual void run()
	{
		IntResouceLoader int_loader;
		UNITT_ASSERT(int_loader.resourceTypeinfo() == typeid(int));

		SharedPointer<int> int_0 = int_loader.load("my_invalid_load");
		UNITT_ASSERT(!int_0);
		UNITT_ASSERT(int_0.get() == 0);

		UNITT_ASSERT(int_loader.suportsFormat("int"));

		UNITT_ASSERT(int_loader.canLoadResourceFromLocation("path/one.int"));
		UNITT_ASSERT(int_loader.canLoadResourceFromLocation("path/two.int"));
		UNITT_ASSERT(int_loader.canLoadResourceFromLocation("path/three.int"));
		UNITT_ASSERT(int_loader.canLoadResourceFromLocation("path/four.int"));

		UNITT_FAIL_NOT_EQUAL(0, int_loader.numberOfCachedResources());

		SharedPointer<int> int_1 = int_loader.load("path/one.int");
		UNITT_ASSERT(int_1);
		UNITT_FAIL_NOT_EQUAL(1, *int_1);
		UNITT_FAIL_NOT_EQUAL(1, int_loader.numberOfCachedResources());
		UNITT_ASSERT(int_loader.isResourceCached("path/one.int"));
		UNITT_FAIL_NOT_EQUAL(1, int_loader.getCacheItem("path/one.int").request_count);


		SharedPointer<int> int_2 = int_loader.load("path/two.int");
		UNITT_ASSERT(int_2);
		UNITT_FAIL_NOT_EQUAL(2, *int_2);
		UNITT_FAIL_NOT_EQUAL(2, int_loader.numberOfCachedResources());
		UNITT_ASSERT(int_loader.isResourceCached("path/two.int"));
		UNITT_FAIL_NOT_EQUAL(1, int_loader.getCacheItem("path/two.int").request_count);


		SharedPointer<int> int_3 = int_loader.load("path/three.int");
		UNITT_ASSERT(int_3);
		UNITT_FAIL_NOT_EQUAL(3, *int_3);
		UNITT_FAIL_NOT_EQUAL(3, int_loader.numberOfCachedResources());
		UNITT_ASSERT(int_loader.isResourceCached("path/three.int"));
		UNITT_FAIL_NOT_EQUAL(1, int_loader.getCacheItem("path/three.int").request_count);


		SharedPointer<int> int_4 = int_loader.load("path/four.int");
		UNITT_ASSERT(int_4);
		UNITT_FAIL_NOT_EQUAL(4, *int_4);
		UNITT_FAIL_NOT_EQUAL(4, int_loader.numberOfCachedResources());
		UNITT_ASSERT(int_loader.isResourceCached("path/four.int"));
		UNITT_FAIL_NOT_EQUAL(1, int_loader.getCacheItem("path/four.int").request_count);


		SharedPointer<int> int_1_1 = int_loader.load("path/one.int");
		UNITT_ASSERT(int_1 == int_1_1);
		UNITT_FAIL_NOT_EQUAL(4, int_loader.numberOfCachedResources());
		UNITT_FAIL_NOT_EQUAL(2, int_loader.getCacheItem("path/one.int").request_count);

		int_loader.load("path/one.int");
		UNITT_FAIL_NOT_EQUAL(4, int_loader.numberOfCachedResources());
		UNITT_FAIL_NOT_EQUAL(3, int_loader.getCacheItem("path/one.int").request_count);

		int_loader.load("path/one.int");
		UNITT_FAIL_NOT_EQUAL(4, int_loader.numberOfCachedResources());
		UNITT_FAIL_NOT_EQUAL(4, int_loader.getCacheItem("path/one.int").request_count);

		int_loader.clearCachedResources();
		UNITT_FAIL_NOT_EQUAL(0, int_loader.numberOfCachedResources());

		int_loader.load("path/one.int");
		UNITT_FAIL_NOT_EQUAL(1, int_loader.numberOfCachedResources());
		UNITT_FAIL_NOT_EQUAL(1, int_loader.getCacheItem("path/one.int").request_count);



		int_loader.clearCachedResources();
		int_loader.setCacheOption(BaseResourceLoader::DoNoCacheResources);

		int_loader.load("path/one.int");
		UNITT_FAIL_NOT_EQUAL(0, int_loader.numberOfCachedResources());
		UNITT_ASSERT(!int_loader.isResourceCached("path/one.int"));

		int_loader.load("path/one.int");
		UNITT_FAIL_NOT_EQUAL(0, int_loader.numberOfCachedResources());
		UNITT_ASSERT(!int_loader.isResourceCached("path/one.int"));

	//	UNITT_ASSERT(int_0.valid());

	}

UNITT_TEST_END_CLASS(UnitTestResourceLoader)
