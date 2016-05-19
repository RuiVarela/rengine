// __!!rengine_copyright!!__ //

#include <rengine/string/String.h>
#include <rengine/lang/debug/Debug.h>

#include <cstdio>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iomanip>


#if RENGINE_COMPILER == RENGINE_COMPILER_MSVC
#define r_snprintf _snprintf
#endif //RENGINE_COMPILER == RENGINE_COMPILER_GNUC

#if RENGINE_COMPILER == RENGINE_COMPILER_GNUC
#define r_snprintf snprintf
#endif //RENGINE_COMPILER == RENGINE_COMPILER_GNUC


namespace rengine
{
	std::vector<std::string> split(std::string const& input, std::string const& delimiter)
	{
		StringElements output;

		if (input.size() != 0)
		{
			std::string::size_type start = 0;

			do
			{
				std::string::size_type end = input.find(delimiter, start);
				if (end == std::string::npos)
				{
					end = input.size();
				}

				std::string selection = input.substr(start, end - start);
				if (selection != "")
				{
					output.push_back(selection);
				}

				start = end + delimiter.size();

			}
			while (start < input.size());

		}
		return output;
	}

	std::string extractToken(std::string const& input, std::string const& delimiter, Uint token_number)
	{
		if (input.size() != 0)
		{
			std::string::size_type start = 0;
			std::string::size_type end = start;

			Bool searching = true;

			do
			{
				end = input.find(delimiter, start);
				if (end == std::string::npos)
				{
					searching = false;
					end = input.size();
				}

				if (token_number == 0)
				{
					searching = false;
				}

				if (searching)
				{
					start = end + delimiter.size();
				}
			}
			while (searching);

			return input.substr(start, end - start);
		}
		return input;
	}

	void trimRight(std::string &string)
	{
		std::string whitespaces(" \t\f\v\n\r");
		std::string::size_type pos = string.find_last_not_of(whitespaces);

		if (pos != std::string::npos)
		{
			string.erase(pos + 1);
		}
		else
		{
			string.clear();
		}
	}

	void uppercase(std::string &string)
	{
		std::transform(string.begin(), string.end(), string.begin(), (Int(*)(Int)) toUppercase);
	}

	void lowercase(std::string &string)
	{
		std::transform(string.begin(), string.end(), string.begin(), (Int(*)(Int)) toLowercase);
	}

	void trimLeft(std::string &string)
	{
		std::string whitespaces(" \t\f\v\n\r");
		std::string::size_type pos = string.find_first_not_of(whitespaces);

		if (pos != std::string::npos)
		{
			string.erase(0, pos);
		}
		else
		{
			string.clear();
		}
	}

	void replace(std::string &target, std::string const& match, std::string const& replace_string)
	{
		std::string::size_type position = target.find(match);

		while (position != std::string::npos)
		{
			target.replace(position, match.size(), replace_string);
			position = target.find(match, position + replace_string.size());
		}
	}

	Bool startsWith(std::string const& string, std::string const& prefix)
	{
		return string.find(prefix) == std::string::size_type(0);
	}

	Bool endsWith(std::string const& string, std::string const& sufix)
	{
		if (string.size() < sufix.size())
		{
			return false;
		}
		else
		{
			return string.rfind(sufix) == (string.size() - sufix.size());
		}
	}

	//
	// Number functions
	//

	Bool isNumber(std::string const& string)
	{
		return (isInteger(string) || isReal(string));
	}

	Bool isInteger(std::string const& string)
	{
		if (string.empty())
		{
			return false;
		}

		std::string::const_iterator i = string.begin();
		if (!isDigit(string[0]))
		{
			if ((string[0] != '-') &&  (string[0] != '+'))
			{
				return false;
			}
			else
			{
				if (++i == string.end())
				{
					return false;
				}
			}
		}

		while (i != string.end())
		{
			if (!isDigit(*i))
			{
				return false;
			}
			++i;
		}

		return true;
	}



	Bool isReal(std::string const& string)
	{
		if (string.empty())
		{
			return false;
		}

		std::string::const_iterator i = string.begin();
		Bool dot_found = false;

		if (!isDigit(string[0]))
		{
			if ((string[0] != '-') && (string[0] != '.') && (string[0] != '+'))
			{
				return false;
			}
			else
			{
				if (string[0] == '.')
				{
					dot_found = true;
				}

				if (++i == string.end())
				{
					return false;
				}
			}
		}

		while (i != string.end())
		{
			if (!isDigit(*i))
			{
				if (*i == '.')
				{
					if (dot_found)
					{
						return false;
					}
					else
					{
						dot_found = true;
					}

				}
				else
				{
					return false;
				}
			}
			++i;
		}

		return dot_found;
	}

	Bool isBoolean(std::string const& string)
	{
		if (isInteger(string))
		{
			return ((string.size() == 1) && ((string[0] == '0') ||  (string[0] == '1')) );
		}
		else
		{
			return ( (string == "true") || (string == "false") );
		}
	}

	Bool isDigit(Int character)
	{
		return std::isdigit(character) != 0;
	}

	Bool isAlphabetic(Int character)
	{
		return std::isalpha(character) != 0;
	}

	Bool isLowercase(Int character)
	{
		return std::islower(character) != 0;
	}

	Bool isUppercase(Int character)
	{
		return std::isupper(character) != 0;
	}

	Int toUppercase(Int character)
	{
		return std::toupper(character);
	}

	Int toLowercase(Int character)
	{
		return std::tolower(character);
	}

	Bool equalCaseInsensitive(std::string const& lhs, std::string const& rhs)
	{
		if (lhs.size() != rhs.size())
		{
			return false;
		}

		std::string::const_iterator litr = lhs.begin();
		std::string::const_iterator ritr = rhs.begin();

		while (litr != lhs.end())
		{
			if (toLowercase(*litr) != toLowercase(*ritr))
			{
				return false;
			}

			++litr;
			++ritr;
		}
		return true;
	}

	Bool equalCaseInsensitive(std::string const& lhs, Char const* rhs)
	{
		if (rhs == 0 || lhs.size() != strlen(rhs))
		{
			return false;
		}

		std::string::const_iterator litr = lhs.begin();
		Char const* cptr = rhs;

		while (litr != lhs.end())
		{
			if (toLowercase(*litr) != toLowercase(*cptr))
			{
				return false;
			}

			++litr;
			++cptr;
		}
		return true;
	}


	//
	// String Printer
	//
	struct StringPrintReplaceable
	{
		StringPrintReplaceable()
		{
			type = ' ';

			foundIndex = -1;
			stringPosition = -1;
			stringSize = -1;
			parameterIndex = -1;
			valid = true;
		}

		Char type;
		std::string expression;
		Int stringPosition;
		Int stringSize;
		Int foundIndex;
		Int parameterIndex;
		bool valid;
	};
	typedef std::vector<StringPrintReplaceable> Replaceables;

	static bool replaceableSorter(StringPrintReplaceable const& lhs, StringPrintReplaceable const& rhs)
	{
		return (lhs.foundIndex < rhs.foundIndex);
	}


	#define RENGINE_SP_ND(n, param) , PrintArgument const * const a ## n
	#define RENGINE_SP_INIT_PARAMS(n, param) arguments[n] = a ## n;

	static Replaceables getReplaceables(std::string const& text)
	{
		Replaceables replaceables;
		StringPrintReplaceable current;
		int total = 0;
		for (std::string::size_type i = 0; i != text.size(); ++i)
		{
			Bool stringEnded = false;
			if (text[i] == '%')
			{
				// previous is start code?
				if ((i > 0u) && (current.stringPosition == int(i - 1u)))
				{
					current.type = '-';
					stringEnded = true;
				}
				else if (current.stringPosition == -1)
				{
					current.stringPosition = i;
				}
				else
				{
					current.type = ' ';
					stringEnded = true;
				}
			}
			else if (current.stringPosition != -1)
			{
				stringEnded = (text[i] == 'c') ||
						      (text[i] == 'd') || (text[i] == 'i') ||
						      (text[i] == 'e') || (text[i] == 'E') ||
						      (text[i] == 'f') || (text[i] == 'g') || (text[i] == 'G') ||
						      (text[i] == 'o') ||
						      (text[i] == 's') ||
						      (text[i] == 'u') ||
						      (text[i] == 'x') || (text[i] == 'X') ||
						      (text[i] == 'p');

				if (stringEnded)
				{
					current.type = text[i];
				}
			}

			//c	Character
			//d	Signed decimal integer
			//i	Signed decimal integer
			//e	Scientific notation (mantise/exponent) using e character
			//E	Scientific notation (mantise/exponent) using E character
			//f	Decimal floating point
			//g	Use the shorter of %e or %f
			//G	Use the shorter of %E or %f
			//o	Signed octal
			//s	String of characters
			//u	Unsigned decimal integer
			//x	Unsigned hexadecimal integer
			//X	Unsigned hexadecimal integer (capital letters)
			//p	Pointer address

			if (stringEnded)
			{
				current.foundIndex = total;
				total++;

				current.stringSize = i - current.stringPosition + 1;
				RENGINE_ASSERT(current.foundIndex >= 0);
				RENGINE_ASSERT(current.stringPosition >= 0);
				RENGINE_ASSERT(current.stringPosition < Int(text.size()) );
				RENGINE_ASSERT(current.stringSize > 0);
				RENGINE_ASSERT((current.stringPosition + current.stringSize) <= Int(text.size()) );

				current.expression = text.substr(current.stringPosition, current.stringSize);
				replaceables.push_back(current);
				current = StringPrintReplaceable();
			}
		}

		//
		// Compute parameter index
		//
		int currentMax = -1;
		for (Replaceables::iterator i = replaceables.begin(); i != replaceables.end(); ++i)
		{
			if (i->type == ' ')
			{
				std::string expression = i->expression;
				replace(expression, "%", "");

				i->parameterIndex = lexical_cast<Int>(expression, -1);

				if (i->parameterIndex != -1)
				{
					i->parameterIndex -= 1;
				}
			}
			else
			{
				i->parameterIndex = currentMax + 1;
			}

			if (i->parameterIndex > currentMax)
			{
				currentMax = i->parameterIndex;
			}
		}


		return replaceables;
	}

	void expandReplaceable(std::ostream& stream, StringPrintReplaceable &replaceable, PrintArgument const* argument)
	{
		bool inError = false;

		Int const buffer_size = 256;
		char expanded_buffer[buffer_size];
		expanded_buffer[0] = '\0';

		Int width = -1;

		std::string expression_no_size;

		if (replaceable.type != ' ')
		{
			char const * start = replaceable.expression.c_str();
			char const * expression = replaceable.expression.c_str();
			
			while (*expression == '%')			 { ++expression; } //%
			while (strchr("-+0 #", *expression)) { ++expression; } // flags

			// width
			if (isDigit(*expression))
			{
				width = 0;
				while (isDigit(*expression))
				{
					width = (width * 10) + (*expression - '0');
					++expression;
				}
			}

			// precision
			if (*expression == '.')
			{
				++expression;
				while (isDigit(*expression)) { ++expression; }
			}

			expression_no_size = std::string(start, expression);
		}

		if (replaceable.type == ' ')
		{
			if (argument->type == PrintArgument::PointerType)
			{
				expression_no_size = std::string("%p");
				r_snprintf(expanded_buffer, buffer_size, expression_no_size.c_str(), argument->value.pointer);
			}
		}
		else if (replaceable.type == 's')
		{
			if (argument->type != PrintArgument::StringType)
			{
				inError = true;
			}
		}
		else
		{
			if ((replaceable.type == 'd') || (replaceable.type == 'i') || (replaceable.type == 'o'))
			{
				expression_no_size += RENGINE_INT64_MASK;
				expression_no_size += replaceable.type;

				if (argument->type != PrintArgument::SignedIntegralType)
				{
					inError = true;
				}

				r_snprintf(expanded_buffer, buffer_size, expression_no_size.c_str(), argument->value.signedIntegral);
			}
			else if ((replaceable.type == 'u') || (replaceable.type == 'x') || (replaceable.type == 'X'))
			{
				expression_no_size += RENGINE_INT64_MASK;
				expression_no_size += replaceable.type;

				if (argument->type != PrintArgument::UnsignedIntegralType)
				{
					inError = true;
				}

				r_snprintf(expanded_buffer, buffer_size, expression_no_size.c_str(), argument->value.unsignedIntegral);
			}
			else if ((replaceable.type == 'e') || (replaceable.type == 'E') || (replaceable.type == 'f') || (replaceable.type == 'g') || (replaceable.type == 'g'))
			{
				expression_no_size += replaceable.type;

				if (argument->type != PrintArgument::RealType)
				{
					inError = true;
				}

				r_snprintf(expanded_buffer, buffer_size, expression_no_size.c_str(), argument->value.real);
			}
			else if (replaceable.type == 'p')
			{
				expression_no_size += replaceable.type;

				if ((argument->type != PrintArgument::PointerType) && (argument->type != PrintArgument::StringType))
				{
					inError = true;
				}

				expression_no_size = std::string("%p");
				r_snprintf(expanded_buffer, buffer_size, expression_no_size.c_str(), argument->value.pointer);
			}
		}

		if (inError)
		{
			stream << "{" << replaceable.parameterIndex << ":error}";
		}
		else if (expanded_buffer[0] != '\0')
		{
			stream << expanded_buffer;
		}
		else if (argument->type == PrintArgument::SignedIntegralType)
		{
			stream << argument->value.signedIntegral;
		}
		else if (argument->type == PrintArgument::UnsignedIntegralType)
		{
			stream << argument->value.unsignedIntegral;
		}	
		else if (argument->type == PrintArgument::RealType)
		{
			stream << argument->value.real;
		}
		else if ( (argument->type == PrintArgument::BoolType) || (argument->type == PrintArgument::StringType) )
		{
			if (width != -1)
			{
				stream << std::setw(width);
			}

			stream << argument->string_value;
		}	
	}

	std::string stringPrint(std::string const& text RENGINE_REPEAT(RENGINE_SP_MAX_ARGS, RENGINE_SP_ND, _))
	{
		PrintArgument const* arguments[RENGINE_SP_MAX_ARGS];
		RENGINE_REPEAT(RENGINE_SP_MAX_ARGS, RENGINE_SP_INIT_PARAMS, _)

		// Count available parameters
		Int parameterCount = 0;
		for (parameterCount = 0; parameterCount != RENGINE_SP_MAX_ARGS; ++parameterCount)
		{
			if (!arguments[parameterCount])
			{
				break;
			}
		}

		//
		// find replaceable
		//
		Replaceables replaceables = getReplaceables(text);

		if (replaceables.empty())
		{
			return text;
		}


		//
		// Check valid replaceables
		//
		Replaceables temporary;
		for (Replaceables::iterator i = replaceables.begin(); i != replaceables.end(); ++i)
		{
			if (i->type != '-')
			{
				i->valid &= i->parameterIndex >= 0;
				i->valid &= i->parameterIndex < parameterCount;
			}

			if (i->valid)
			{
				temporary.push_back(*i);
			}
		}
		std::swap(temporary, replaceables);

		//
		// Sort by found position
		//
		std::sort(replaceables.begin(), replaceables.end(), replaceableSorter);

		//
		// Replace
		//
		std::stringstream output;
		std::string::size_type start_index = 0;
		for (Replaceables::iterator i = replaceables.begin(); i != replaceables.end(); ++i)
		{
			std::string::size_type delta = i->stringPosition - start_index;
			
			output << text.substr(start_index, delta);

			if (i->type == '-')
			{
				output << "%";
			}
			else
			{		
				expandReplaceable(output, *i, arguments[i->parameterIndex]);
			}

			start_index = i->stringPosition + i->stringSize;
		}

		if (start_index < text.size())
		{
			output << text.substr(start_index);
		}

		return output.str();
	}

} // namespace rengine

