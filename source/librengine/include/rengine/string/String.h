// __!!rengine_copyright!!__ //

#ifndef __RENGINE_STRING__
#define __RENGINE_STRING__

#include <rengine/lang/Lang.h>
#include <rengine/util/Any.h>
#include <string>
#include <sstream>
#include <vector>
#include <limits>

namespace rengine
{
	enum CaseSensitivity
	{
		CaseSensitive,
		CaseInsensitive
	};

	enum CaseType
	{
		Uppercase,
		Lowercase
	};

	enum TrimType
	{
		TrimRight,
		TrimLeft,
		TrimBoth
	};

	template<typename OutputType, typename InputType>
	OutputType lexical_cast(InputType const& value);

	template<typename OutputType, typename InputType>
	OutputType lexical_cast(InputType const& value, OutputType const& default_value);

	typedef std::vector<std::string> StringElements;
	StringElements split(std::string const& input, std::string const& delimiter);

	std::string extractToken(std::string const& input, std::string const& delimiter, Uint token_number);

	void trimRight(std::string &string);
	void trimLeft(std::string &string);
	void trim(std::string &string);
	std::string trim(TrimType const& trim_type, std::string string);

	void uppercase(std::string &string);
	void lowercase(std::string &string);
	std::string convertCase(CaseType const& case_type, std::string string);

	void replace(std::string &target, std::string const& match, std::string const& replace_string);

	Bool startsWith(std::string const& string, std::string const& prefix);
	Bool endsWith(std::string const& string, std::string const& sufix);

	Bool isNumber(std::string const& string);
	Bool isInteger(std::string const& string);
	Bool isReal(std::string const& string);
	Bool isBoolean(std::string const& string);

	Bool isDigit(Int character);
	Bool isAlphabetic(Int character);
	Bool isLowercase(Int character);
	Bool isUppercase(Int character);

	Int toUppercase(Int character);
	Int toLowercase(Int character);

	Bool equalCaseInsensitive(std::string const& lhs, std::string const& rhs);
	Bool equalCaseInsensitive(std::string const& lhs, Char const* rhs);

	template<typename T>
	std::string separateElements(T const* const array, Int const N, std::string separator);

	//
	// String Printer
	//
	class PrintArgument
	{
	public:
		typedef void const * PrintPointer;

		enum Type
		{
			SignedIntegralType,
			UnsignedIntegralType,
			RealType,
			StringType,
			BoolType,
			PointerType
		};

		union Value
		{
			Uint64 unsignedIntegral;
			Int64 signedIntegral;
			Real64 real;
			PrintPointer pointer;
		};

		PrintArgument(std::string const& print_value) : type(StringType) { string_value = print_value; value.pointer = PrintPointer(0); }
		PrintArgument(Char * const print_value) : type(StringType) { string_value = print_value; value.pointer = PrintPointer(print_value); }
		PrintArgument(Char const * const print_value) : type(StringType) { string_value = print_value; value.pointer = PrintPointer(print_value); }

		PrintArgument(Real32 const print_value) : type(RealType) { value.real = print_value; }
		PrintArgument(Real64 const print_value) : type(RealType) { value.real = print_value; }

		PrintArgument(Int8 const print_value) : type(SignedIntegralType) { value.signedIntegral = print_value; }
		PrintArgument(Int16 const print_value) : type(SignedIntegralType) { value.signedIntegral = print_value; }
		PrintArgument(Int32 const print_value) : type(SignedIntegralType) { value.signedIntegral = print_value; }
		PrintArgument(Int64 const print_value) : type(SignedIntegralType) { value.signedIntegral = print_value; }

		PrintArgument(Uint8 const print_value) : type(UnsignedIntegralType) { value.unsignedIntegral = print_value; }
		PrintArgument(Uint16 const print_value) : type(UnsignedIntegralType) { value.unsignedIntegral = print_value; }
		PrintArgument(Uint32 const print_value) : type(UnsignedIntegralType) { value.unsignedIntegral = print_value; }
		PrintArgument(Uint64 const print_value) : type(UnsignedIntegralType) { value.unsignedIntegral = print_value; }

		PrintArgument(Bool const print_value) : type(StringType){ string_value = (print_value ? "true" : "false"); }

		template <typename T> PrintArgument(T * const print_value) : type(PointerType) { value.pointer = PrintPointer(print_value); }
		template <typename T> PrintArgument(T const * const print_value) : type(PointerType) { value.pointer = PrintPointer(print_value); }

		// Everything else
		template <typename T> PrintArgument(T const& print_value);

		Type type;
		Value value;
		std::string string_value;
	};

	#define RENGINE_SP_MAX_ARGS 25
	#define RENGINE_SP_WD(n, param) , PrintArgument const * a ## n = 0
	#define RENGINE_SPR(n, param) , PrintArgument const& a ## n
	#define RENGINE_SPR_ARGS(n, param) , &a ## n
	#define RENGINE_SP_FUNCTION(N_ARGS) RENGINE_INLINE std::string stringPrinter(std::string const& text RENGINE_REPEAT(N_ARGS, RENGINE_SPR, _ )) { return stringPrint(text RENGINE_REPEAT(N_ARGS, RENGINE_SPR_ARGS, _ )); }

	// Description
	//	Type safe string printer,
	//	user should not use stringPrint directly. instead the corresponding stringPrinter version should be used!
	std::string stringPrint(std::string const& text RENGINE_REPEAT(RENGINE_SP_MAX_ARGS, RENGINE_SP_WD, _ ));

	RENGINE_SP_FUNCTION(0)
	RENGINE_SP_FUNCTION(1)
	RENGINE_SP_FUNCTION(2)
	RENGINE_SP_FUNCTION(3)
	RENGINE_SP_FUNCTION(4)
	RENGINE_SP_FUNCTION(5)
	RENGINE_SP_FUNCTION(6)
	RENGINE_SP_FUNCTION(7)
	RENGINE_SP_FUNCTION(8)
	RENGINE_SP_FUNCTION(9)
	RENGINE_SP_FUNCTION(10)
	RENGINE_SP_FUNCTION(11)
	RENGINE_SP_FUNCTION(12)
	RENGINE_SP_FUNCTION(13)
	RENGINE_SP_FUNCTION(14)
	RENGINE_SP_FUNCTION(15)
	RENGINE_SP_FUNCTION(16)
	RENGINE_SP_FUNCTION(17)
	RENGINE_SP_FUNCTION(18)
	RENGINE_SP_FUNCTION(19)
	RENGINE_SP_FUNCTION(20)
	RENGINE_SP_FUNCTION(21)
	RENGINE_SP_FUNCTION(22)
	RENGINE_SP_FUNCTION(23)
	RENGINE_SP_FUNCTION(24)
	RENGINE_SP_FUNCTION(25)


	//
	// Implementation
	//
	template<typename T>
	std::string separateElements(T const* const array, Int const N, std::string separator)
	{
		std::stringstream stream;

		if (std::numeric_limits<T>::is_specialized)
		{
			stream.precision(std::numeric_limits<T>::digits10 + 1);

			if (!std::numeric_limits<T>::is_integer)
			{
				stream.setf(std::ios::showpoint | std::ios::fixed);
			}
		}

		for (Int i = 0; i != N; ++i)
		{
			if (i > 0)
			{
				stream << separator;
			}

			stream << array[i];
		}

		return stream.str();
	}

	//
	// Implementation
	//
	template<typename OutputType, typename InputType>
	OutputType lexical_cast(InputType const& value)
	{
		std::stringstream stream;
		//skip leading whitespace characters on input
		stream.unsetf(std::ios::skipws);

		//set the correct precision
		if (std::numeric_limits<OutputType>::is_specialized)
		{
			stream.precision(std::numeric_limits<OutputType>::digits10 + 1);
		}
//		else if (std::numeric_limits<InputType>::is_specialized)
//		{
//			stream.precision(std::numeric_limits<InputType>::digits10 + 1);
//		}

		stream << value;

		OutputType output;
		stream >> output;
		return output;
	}

	template<typename OutputType, typename InputType>
	OutputType lexical_cast(InputType const& value, OutputType const& default_value)
	{
		std::stringstream stream;
		//skip leading whitespace characters on input
		stream.unsetf(std::ios::skipws);

		//set the correct precision
		if (std::numeric_limits<OutputType>::is_specialized)
		{
			stream.precision(std::numeric_limits<OutputType>::digits10 + 1);
		}
//		else if (std::numeric_limits<InputType>::is_specialized)
//		{
//			stream.precision(std::numeric_limits<InputType>::digits10 + 1);
//		}

		stream << value;

		OutputType output;
		stream >> output;

		if (!stream)
		{
			output = default_value;
		}

		return output;
	}

	RENGINE_INLINE std::string convertCase(CaseType const& case_type, std::string string)
	{
		if (case_type == Uppercase)
		{
			uppercase(string);
		}
		else if (case_type == Lowercase)
		{
			lowercase(string);
		}

		return string;
	}

	RENGINE_INLINE std::string trim(TrimType const& trim_type, std::string string)
	{
		if ((trim_type == TrimLeft) || (trim_type == TrimBoth))
		{
			trimLeft(string);
		}

		if ((trim_type == TrimRight) || (trim_type == TrimBoth))
		{
			trimRight(string);
		}

		return string;
	}

	RENGINE_INLINE void trim(std::string &string)
	{
		trimLeft(string);
		trimRight(string);
	}

	template<typename T>
	RENGINE_INLINE PrintArgument::PrintArgument(T const& print_value) :
		type(StringType), value(lexical_cast<std::string> (print_value))
	{
	}


} // namespace rengine

#endif //__RENGINE_STRING__

