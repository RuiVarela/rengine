// __!!rengine_copyright!!__ //

#ifndef __RENGINE_OUTPUT_STREAM__
#define __RENGINE_OUTPUT_STREAM__

#include <rengine/lang/Lang.h>
#include <ostream>
#include <string>
#include <vector>

namespace rengine
{
	struct StringPrinter
	{
		virtual ~StringPrinter() {};
		virtual void operator()(std::string const& string_data) = 0;
	};

	struct CoutStringPrinter : public StringPrinter
	{
		virtual ~CoutStringPrinter() {};
		virtual void operator()(std::string const& string_data);
	};

	struct LoggerStringPrinter : public StringPrinter
	{
		LoggerStringPrinter();
		virtual ~LoggerStringPrinter(){}
		virtual void operator()(std::string const& string_data);
		std::string log_name;
	};

	class StreamBufferConnector : public std::streambuf
	{
	public:
		static Uint const buffer_size = 100 * 1024;

		StreamBufferConnector();
		~StreamBufferConnector();

		void registerPrinter(StringPrinter* string_printer) { this->string_printer = string_printer; }
		StringPrinter* printer() const { return string_printer; }

	private:
		char_type buffer[buffer_size];
		StringPrinter* string_printer;

	protected:
		virtual int_type sync();
		virtual int_type overflow(int_type data = std::streambuf::traits_type::eof());
	};

	class OutputStream : public std::ostream, public StringPrinter
	{
	public:
		typedef std::vector< SharedPointer<StringPrinter> > StringPrinters;

		OutputStream();
		~OutputStream();

		virtual void operator()(std::string const& string_data);

		void registerPrinter(SharedPointer<StringPrinter> const& printer);
		void removePrinter(StringPrinter *const printer);
		void clearPrinters();

		StringPrinters const& printers() const { return string_printers; }
	private:
		SharedPointer<StreamBufferConnector> stream_buffer_connector;
		StringPrinters string_printers;
	};

} // namespace rengine

#endif //__RENGINE_OUTPUT_STREAM__
