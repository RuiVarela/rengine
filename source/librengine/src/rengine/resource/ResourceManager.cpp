// __!!rengine_copyright!!__ //

#include <rengine/resource/ResourceManager.h>
#include <rengine/outputstream/OutputStream.h>
#include <rengine/CoreEngine.h>
#include <rengine/util/Demangle.h>

namespace rengine
{
	ResourceManager::ResourceManager()
	{
		caching_option = new SystemVariable("resource_caching_option", "default");
		caching_option->setHandler(this);
		caching_option->setDescription("resource caching state [cache, nocache]");
	}

	ResourceManager::~ResourceManager()
	{
		clearCachedResources();
		clearLoaders();
	}

	void ResourceManager::clearCachedResources()
	{
		for (ResourceLoaders::size_type i = 0; i != loaders.size(); ++i)
		{
			loaders[i]->clearCachedResources();
		}
	}

	void ResourceManager::setCacheOption(BaseResourceLoader::CacheOption const& option)
	{
		for (ResourceLoaders::size_type i = 0; i != loaders.size(); ++i)
		{
			loaders[i]->setCacheOption(option);
		}
	}

	void ResourceManager::addLoader(SharedPointer<BaseResourceLoader> const& loader)
	{
		loaders.push_back(loader);
	}

	void ResourceManager::configure()
	{
		CoreEngine::instance()->system().registerCommand(
				new SystemCommand("resourceReportInfo", ResourceReportInfo, this, "show a report of the current resource usage")
				);

		CoreEngine::instance()->system().registerCommand(
				new SystemCommand("resourceClearCache", ResourceClearCache, this, "remove all resource elements from cache")
				);

		CoreEngine::instance()->system().registerVariable(caching_option);

		registerDefaultLoaders();
	}

	void ResourceManager::reportLoadersInfo() const
	{
		CoreEngine::instance()->log() << "Resource Info report:" << std::endl;
		CoreEngine::instance()->log() << "[request count] location" << std::endl;
		for (ResourceLoaders::size_type i = 0; i != loaders.size(); ++i)
		{
			CoreEngine::instance()->log() << demangleType(loaders[i]->resourceTypeinfo().name()) <<  std::endl;

			BaseResourceLoader::ResourceLoaderInfo info = loaders[i]->loaderInfo();
			for (BaseResourceLoader::ResourceLoaderInfo::const_iterator i = info.begin(); i != info.end(); ++i)
			{
				CoreEngine::instance()->log() << "[" << i->request_count << "] " << i->name << std::endl;

			}
		}
	}

	void ResourceManager::operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		switch (command)
		{
			case ResourceReportInfo:
			{
				reportLoadersInfo();
				break;
			}
			case ResourceClearCache:
			{
				clearCachedResources();
				reportLoadersInfo();
				break;
			}

			default:
				break;
		}
	}

	Bool ResourceManager::operator()(SystemVariable& variable, SystemVariable::Arguments const& arguments)
	{
		//TODO: implement this

		CoreEngine::instance()->log() << " not implemented " << std::endl;
		return false;
	}
} // end of namespace
