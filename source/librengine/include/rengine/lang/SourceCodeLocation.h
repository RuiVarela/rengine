// __!!rengine_copyright!!__ //

#ifndef __RENGINE_CODE_LOCATION_H__
#define __RENGINE_CODE_LOCATION_H__

#include <rengine/lang/Platform.h>
#include <rengine/lang/debug/MemoryAllocator.h>
#include <cstring>
#include <cstdlib>

namespace rengine
{
	struct SourceCodeLocation
	{
		SourceCodeLocation()
			:line_number(0), filename(0), function(0), function_pretty(0)
		{
		}

		~SourceCodeLocation()
		{
			if (filename)
			{
				rg_free(filename);
				filename = 0;
			}

			if (function)
			{
				rg_free(function);
				function = 0;
			}

			if (function_pretty)
			{
				rg_free(function_pretty);
				function_pretty = 0;
			}
		}

		SourceCodeLocation(SourceCodeLocation const& copy)
			:line_number(copy.line_number), filename(0), function(0), function_pretty(0)
		{
			if (copy.filename) { filename = rg_strdup(copy.filename); }
			if (copy.function) { function = rg_strdup(copy.function); }
			if (copy.function_pretty) { function_pretty = rg_strdup(copy.function_pretty); }
		}

		SourceCodeLocation& operator=(SourceCodeLocation copy)
		{
			swap(copy);
			return *this;
		}

	    void swap(SourceCodeLocation& other)
	    {
	    	int int_swapper = line_number;
	    	line_number = other.line_number;
	    	other.line_number = int_swapper;

	    	char* char_swapper = filename;
	    	filename = other.filename;
	    	other.filename = char_swapper;

	    	char_swapper = function;
	    	function = other.function;
	    	other.function = char_swapper;

	    	char_swapper = function_pretty;
	    	function_pretty = other.function_pretty;
	    	other.function_pretty = char_swapper;
	    }

		SourceCodeLocation(int const line,
						   char const* const file,
						   char const* const current_function,
						   char const* const current_function_pretty) :
			line_number(line), filename(0), function(0), function_pretty(0)
		{

			if (file) { filename = rg_strdup(file); }
			if (current_function) { function = rg_strdup(current_function); }
			if (current_function_pretty) { function_pretty = rg_strdup(current_function_pretty); }
		}

		int line_number;
		char* filename;
		char* function;
		char* function_pretty;
	};

#define RENGINE_SOURCE_CODE_LOCATION() SourceCodeLocation( __LINE__ , __FILE__ , __FUNCTION__, RENGINE_FUNCTION)

} // end of namespace

#endif // __RENGINE_CODE_LOCATION_H__
