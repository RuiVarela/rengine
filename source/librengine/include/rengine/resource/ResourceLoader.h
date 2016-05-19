// __!!rengine_copyright!!__ //

#ifndef __RENGINE_RESOURCE_LOADER_H__
#define __RENGINE_RESOURCE_LOADER_H__

#include <rengine/lang/Lang.h>
#include <rengine/util/OpaqueProperty.h>
#include <map>

namespace rengine
{
	class BaseResourceLoader
	{
	public:
		typedef Uint SizeType;

		enum CacheOption
		{
			DoNoCacheResources,
			CacheResources
		};

		struct CacheItemInfo
		{
			std::string name;
			SizeType request_count;
		};

		typedef std::vector<CacheItemInfo> ResourceLoaderInfo;

		BaseResourceLoader();
		virtual ~BaseResourceLoader();

		virtual Bool suportsFormat(std::string const& extension) const;

		virtual Bool canLoadResourceFromLocation(std::string const& resource_location) const;

		CacheOption getCacheOption() const;

		virtual void setCacheOption(CacheOption const& option);

		virtual SizeType numberOfCachedResources() const = 0;

		virtual void clearCachedResources() = 0;

		virtual Bool isResourceCached(std::string const& resource_location) const = 0;

		virtual std::type_info const& resourceTypeinfo() const = 0;

		virtual ResourceLoaderInfo loaderInfo() const = 0;
	private:
		CacheOption cache_option;
	};

	//
	// ResourceLoader
	//

	template<typename T>
	class ResourceLoader : public BaseResourceLoader
	{
	public:
		typedef T ResourceType;

		struct CacheItem
		{
			CacheItem();

			SharedPointer<ResourceType> resource;
			SizeType request_count;
			std::string location;
		};

		typedef std::map<std::string, CacheItem> ResourceCache;

		ResourceLoader();
		virtual ~ResourceLoader();

		virtual SizeType numberOfCachedResources() const;

		virtual void clearCachedResources();

		virtual Bool isResourceCached(std::string const& resource_location) const;
		virtual Bool isResourceCached(ResourceType const * const resource) const;

		virtual std::type_info const& resourceTypeinfo() const;

		virtual ResourceLoaderInfo loaderInfo() const;

		virtual SharedPointer<ResourceType> loadImplementation(std::string const& location, OpaqueProperties const& options = OpaqueProperties()) = 0;

		SharedPointer<ResourceType> load(std::string const& location, OpaqueProperties const& options = OpaqueProperties());


		CacheItem const& getCacheItem(ResourceType const * const resource) const;
		CacheItem const& getCacheItem(std::string const& location) const;
	private:
		ResourceCache cache;
	};

	//
	// Implementation
	//

	RENGINE_INLINE BaseResourceLoader::BaseResourceLoader()
	{
		setCacheOption(CacheResources);
	}

	RENGINE_INLINE BaseResourceLoader::~BaseResourceLoader()
	{
	}

	RENGINE_INLINE BaseResourceLoader::CacheOption BaseResourceLoader::getCacheOption() const
	{
		return cache_option;
	}

	RENGINE_INLINE void BaseResourceLoader::setCacheOption(CacheOption const& option)
	{
		cache_option = option;
	}

	//
	// Resource Loader
	//

	template<typename T>
	ResourceLoader<T>::ResourceLoader()
	{
	}

	template<typename T>
	ResourceLoader<T>::~ResourceLoader()
	{
	}

	template<typename T>
	ResourceLoader<T>::CacheItem::CacheItem() :
		resource(0), request_count(0)
	{
	}

	template<typename T>
	std::type_info const& ResourceLoader<T>::resourceTypeinfo() const
	{
		return typeid(ResourceType);
	}

	template<typename T>
	BaseResourceLoader::ResourceLoaderInfo ResourceLoader<T>::loaderInfo() const
	{
		ResourceLoaderInfo info;

		for (typename ResourceCache::const_iterator i = cache.begin(); i != cache.end(); ++i)
		{
			CacheItemInfo item;
			item.name = i->first;
			item.request_count = i->second.request_count;

			info.push_back(item);
		}

		return info;
	}

	template<typename T>
	SharedPointer<T> ResourceLoader<T>::load(std::string const& location, OpaqueProperties const& options)
	{
		SharedPointer<T> resource;

		if ((getCacheOption() == CacheResources) && isResourceCached(location))
		{
			CacheItem& cache_item = cache[location];
			cache_item.request_count++;
			resource = cache_item.resource;

		}
		else if (canLoadResourceFromLocation(location))
		{
			resource = loadImplementation(location, options);

			Bool do_cache = (getCacheOption() == CacheResources);
			if (options.hasProperty("cache_option"))
			{
				CacheOption override_option = any_cast<CacheOption>( options["cache_option"].value );
				do_cache = (override_option == CacheResources);
			}

			if (resource.get() && do_cache)
			{
				CacheItem cache_item;
				cache_item.request_count = 1;
				cache_item.resource = resource;
				cache_item.location = location;

				cache[location] = cache_item;
			}
		}

		return resource;
	}

	template<typename T>
	typename ResourceLoader<T>::SizeType ResourceLoader<T>::numberOfCachedResources() const
	{
		return cache.size();
	}

	template<typename T>
	void ResourceLoader<T>::clearCachedResources()
	{
		cache.clear();
	}

	template<typename T>
	Bool ResourceLoader<T>::isResourceCached(std::string const& resource_location) const
	{
		return (cache.find(resource_location) != cache.end());
	}

	template<typename T>
	Bool ResourceLoader<T>::isResourceCached(ResourceType const * const resource) const
	{
		Bool found = false;

		for (typename ResourceCache::const_iterator i = cache.begin();
			 (i != cache.end()) && !found;
			 ++i)
		{
			if (i->second.resource.get() == resource)
			{
				found = true;
			}
		}

		return found;
	}

	template<typename T>
	typename ResourceLoader<T>::CacheItem const& ResourceLoader<T>::getCacheItem(ResourceType const * const resource) const
	{
		Bool found = false;
		CacheItem const * item = 0;

		for (typename ResourceCache::const_iterator i = cache.begin();
			 (i != cache.end()) && !found;
			 ++i)
		{
			if (i->second.resource.get() == resource)
			{
				found = true;
				item = &(i->second);
			}
		}

		return *item;
	}

	template<typename T>
	typename ResourceLoader<T>::CacheItem const& ResourceLoader<T>::getCacheItem(std::string const& location) const
	{
		RENGINE_ASSERT( isResourceCached(location) );
		return cache.find(location)->second;
	}

}// end of namespace

#endif // __RENGINE_RESOURCE_LOADER_H__
