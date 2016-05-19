// __!!rengine_copyright!!__ //

#ifndef __RENGINE_RESOURCE_MANAGER_H__
#define __RENGINE_RESOURCE_MANAGER_H__

#include <rengine/resource/ResourceLoader.h>
#include <rengine/system/System.h>
#include <vector>

namespace rengine
{
	class ResourceManager: public SystemCommand::Handler, public SystemVariable::Handler
	{
	public:
		typedef std::vector< SharedPointer<BaseResourceLoader> > ResourceLoaders;

		ResourceManager();
		~ResourceManager();

		void clearCachedResources();
		//set cache option for all loaders
		void setCacheOption(BaseResourceLoader::CacheOption const& option);

		void addLoader(SharedPointer<BaseResourceLoader> const& loader);
		void clearLoaders();

		ResourceLoaders const& getLoaders() const;

		void reportLoadersInfo() const;

		//register system commands and register default loaders
		void configure();

		void registerDefaultLoaders();

		template<typename T>
		Bool suportsFormat(std::string const& extension) const;

		template<typename T>
		SharedPointer<T> load(std::string const& location, OpaqueProperties const& options = OpaqueProperties());

		//
		// Gets the resource location from the object
		template<typename T>
		std::string resourceLocation(T const * const resource) const;


		void operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments);
		Bool operator()(SystemVariable& variable, SystemVariable::Arguments const& arguments);
	private:
		ResourceLoaders loaders;

		enum Commands
		{
			ResourceReportInfo,
			ResourceClearCache
		};

		SharedPointer<SystemVariable> caching_option;
	};

	//
	// Implementation
	//

	RENGINE_INLINE ResourceManager::ResourceLoaders const& ResourceManager::getLoaders() const
	{
		return loaders;
	}

	RENGINE_INLINE void ResourceManager::clearLoaders()
	{
		loaders.clear();
	}

	template<typename T>
	std::string ResourceManager::resourceLocation(T const * const resource) const
	{
		std::string location;
		for (ResourceLoaders::size_type i = 0; ((i != loaders.size()) && location.empty()); ++i)
		{
			if (loaders[i]->resourceTypeinfo() == typeid(T))
			{
				ResourceLoader<T> *loader = dynamic_cast< ResourceLoader<T>* > ( loaders[i].get() );
				if (loader->isResourceCached(resource))
				{
					typename ResourceLoader<T>::CacheItem item = loader->getCacheItem(resource);
					location = item.location;
				}
			}
		}

		return location;
	}

	template<typename T>
	Bool ResourceManager::suportsFormat(std::string const& extension) const
	{
		Bool supported = false;
		for (ResourceLoaders::size_type i = 0; ((i != loaders.size()) && !supported); ++i)
		{
			if (loaders[i]->resourceTypeinfo() == typeid(T))
			{
				if (loaders[i]->suportsFormat(extension))
				{
					supported = true;
				}
			}
		}

		return supported;
	}

	template<typename T>
	SharedPointer<T> ResourceManager::load(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<T> resource;

		for (ResourceLoaders::size_type i = 0; ((i != loaders.size()) && !resource.get()); ++i)
		{
			if (loaders[i]->resourceTypeinfo() == typeid(T))
			{
				ResourceLoader<T> *loader = dynamic_cast< ResourceLoader<T>* > ( loaders[i].get() );
				resource = loader->load(location, options);
			}
		}
		return resource;
	}

}// end of namespace

#endif // __RENGINE_RESOURCE_MANAGER_H__
