// __!!rengine_copyright!!__ //

#include <rengine/util/Demangle.h>
#include <rengine/lang/Lang.h>

#ifdef __GNUC__
#include <cxxabi.h>
#include <cstdlib>

namespace rengine
{
	std::string demangleType(std::string mangled_type)
	{
		Int status = 0;
		Char* demangled = 0;
		demangled = abi::__cxa_demangle(mangled_type.c_str(), 0, 0, &status);

		std::string demangled_string;

		if (demangled)
		{
			demangled_string = std::string(demangled);
			free(demangled);
		}
		else
		{
			demangled_string = mangled_type;
		}

		return demangled_string;
	}
} //namespace

#else

namespace rengine
{
	std::string demangleType(std::string mangled_type)
	{
		return mangled_type;
	}
} //namespace

#endif

