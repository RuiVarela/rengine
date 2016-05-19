// __!!rengine_copyright!!__ //

#ifndef __RENGINE_IDIOMS_H__
#define __RENGINE_IDIOMS_H__

namespace rengine
{
	//
	// NonCopyable
	//
	class NonCopyable
	{
	  protected:
	    NonCopyable() {}
	    ~NonCopyable() {}

	  private:
	    NonCopyable(NonCopyable const&);
	    NonCopyable const& operator = (NonCopyable const&);
	};


} // end of namespace

#endif // __RENGINE_IDIOMS_H__
