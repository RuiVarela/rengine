// __!!rengine_copyright!!__ //

#ifndef __RENGINE_STRING_TABLE_H__
#define __RENGINE_STRING_TABLE_H__

#include <string>
#include <map>

namespace rengine
{
	struct StringElement
	{
		std::string text;
	};

	class StringTable : public std::map<std::string, StringElement>
	{
	public:
		typedef std::map<std::string, StringElement> MapType;

		std::string lookUp(std::string const& key) const;
	};

	//
	// Implementation
	//

	RENGINE_INLINE std::string StringTable::lookUp(std::string const& key) const
	{
		std::string output;

		const_iterator found = find(key);

		if (found != end())
		{
			output = found->second.text;
		}

		return output;
	}
}

#endif //__RENGINE_STRING_TABLE_H__
