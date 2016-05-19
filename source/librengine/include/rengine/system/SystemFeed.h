// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SYSTEM_FEED_H__
#define __RENGINE_SYSTEM_FEED_H__

#include <string>

namespace rengine
{
	//
	// routes data to be run by the system
	//

	class SystemFeed
	{
	public:
		virtual ~SystemFeed(){}
		virtual void operator()(std::string const& line) = 0;
	};

} // namespace rengine

#endif // __RENGINE_SYSTEM_FEED_H__
