// __!!rengine_copyright!!__ //

#ifndef __RENGINE_OPAQUE_PROPERTY__
#define __RENGINE_OPAQUE_PROPERTY__

#include <rengine/util/Any.h>
#include <rengine/lang/debug/Debug.h>

#include <vector>
#include <algorithm>
#include <string>


namespace rengine
{
	struct OpaqueProperty
	{
		OpaqueProperty()
		{
		}

		OpaqueProperty(std::string const& property_name, Any const& property_value) :
			name(property_name), value(property_value)
		{
		}

		explicit OpaqueProperty(std::string const& property_name) :
			name(property_name)
		{
		}

		explicit OpaqueProperty(Char const* property_name) :
			name(property_name)
		{
		}

		OpaqueProperty(Any const& property_value) :
			value(property_value)
		{
		}

		std::string name;
		Any value;
	};

	struct OpaquePropertySameNameComparable
	{
		OpaquePropertySameNameComparable()
		{
		}

		OpaquePropertySameNameComparable(std::string const& search_name) :
			name(search_name)
		{
		}

		Bool operator()(OpaqueProperty const& operand) const
		{
			return (operand.name == name);
		}

		std::string name;
	};

	class OpaqueProperties : public std::vector<OpaqueProperty>
	{
	public:
		typedef std::vector<OpaqueProperty> ConcreteType;

		Bool hasProperty(std::string const& name) const
		{
			 return (std::find_if(begin(), end(), OpaquePropertySameNameComparable(name)) != end());
		}

		OpaqueProperty const& operator[](std::string const& name) const
		{
			RENGINE_ASSERT(hasProperty(name));
			return *std::find_if(begin(), end(), OpaquePropertySameNameComparable(name));
		}

		OpaqueProperty& operator[](std::string const& name)
		{
			RENGINE_ASSERT(hasProperty(name));
			return *std::find_if(begin(), end(), OpaquePropertySameNameComparable(name));
		}

		OpaqueProperty const& operator[](ConcreteType::size_type const index) const
		{
			return ConcreteType::operator[](index);
		}

		OpaqueProperty& operator[](ConcreteType::size_type const index)
		{
			return ConcreteType::operator[](index);
		}
	};
}

#endif //__RENGINE_OPAQUE_PROPERTY__
