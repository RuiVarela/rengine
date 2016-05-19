#include <rengine/outputstream/OutputStream.h>
#include <rengine/outputstream/Log.h>
#include <rengine/string/String.h>

#include <iostream>
#include <string>

namespace rengine
{
	void CoutStringPrinter::operator()(std::string const& string_data)
	{
		std::cout << string_data << std::flush;
	}

	LoggerStringPrinter::LoggerStringPrinter()
	{
		log_name = "system";

		SharedLog log = LogSystem::instance()->getLog("system");
		
		if (log)
		{
			log->setShowFile(false);
			log->setShowLineNumber(false);
			log->setShowCodeScope(false);
			log->setShowSeverity(false);
		}
	}
	
	void LoggerStringPrinter::operator()(std::string const& string_data)
	{
		std::string trimed = string_data;
		trimRight(trimed);
		RLOG_INFO(log_name, trimed);
	}


	//
	// StreamBufferConnector
	//

	StreamBufferConnector::StreamBufferConnector()
	:string_printer(0)
	{
		setp(buffer, buffer + buffer_size);
	}

	StreamBufferConnector::~StreamBufferConnector()
	{
	}

	StreamBufferConnector::int_type StreamBufferConnector::sync()
	{
		size_t flush_size = pptr() - pbase();
		if (flush_size > 0)
		{
			if (string_printer)
			{
				(*string_printer)(std::string(buffer, flush_size));
			}
			setp(buffer, buffer + buffer_size);
		}
		return 0;
	}

	StreamBufferConnector::int_type StreamBufferConnector::overflow(StreamBufferConnector::int_type data)
	{
		sync();
		if (data != traits_type::eof())
		{
			sputc(data); // *buffer = data; pbump(1);
		}
		return data;
	}

	//
	// OutputStream
	//

	OutputStream::OutputStream()
		:std::ios(0), std::ostream(0)
	{
		stream_buffer_connector = new StreamBufferConnector();
		stream_buffer_connector->registerPrinter(this);
		rdbuf(stream_buffer_connector.get());
	}

	OutputStream::~OutputStream()
	{
		clearPrinters();
	}

	void OutputStream::operator()(std::string const& string_data)
	{
		for (StringPrinters::iterator iterator = string_printers.begin();
			iterator != string_printers.end();
			++iterator)
		{
			(*iterator)->operator()(string_data);
		}
	}

	void OutputStream::registerPrinter(SharedPointer<StringPrinter> const& printer)
	{
		string_printers.push_back(printer);
	}

	void OutputStream::removePrinter(StringPrinter *const printer)
	{
		StringPrinters::iterator iterator = string_printers.begin();
		while(iterator != string_printers.end())
		{
			if (*iterator!= printer)
			{
				string_printers.erase(iterator);
			}
			else
			{
				++iterator;
			}

		}
	}

	void OutputStream::clearPrinters()
	{
		string_printers.clear();
	}
}
