// __!!rengine_copyright!!__ //

#include <rengine/outputstream/Log.h>
#include <rengine/file/File.h>
#include <rengine/time/Timestamp.h>
#include <rengine/time/Timer.h>
#include <iomanip>

static int const header_line_size = 100;
static char const header_char = '*';



namespace rengine
{

	//todo remove me
	static Timer gtimer;

	static std::string TimestampNow()
	{
		return Timestamp().toString();
	}

	static std::string severityToString(Log::Severity const& severity)
	{
		RENGINE_QUOTE_ENUM_BEGIN(output_string, severity);
			RENGINE_QUOTE_ENUM_NS(output_string, Log, Trace);
			RENGINE_QUOTE_ENUM_NS(output_string, Log, Debug);
			RENGINE_QUOTE_ENUM_NS(output_string, Log, Info);
			RENGINE_QUOTE_ENUM_NS(output_string, Log, Warn);
			RENGINE_QUOTE_ENUM_NS(output_string, Log, Error);
			RENGINE_QUOTE_ENUM_NS(output_string, Log, Fatal);
		RENGINE_QUOTE_ENUM_END(output_string, "invalid_value");

		return output_string;
	}

	std::ostream& operator << (std::ostream& out, Log::Severity const& severity)
	{
		std::string value = severityToString(severity);
		value = value.substr(0, 3);
		uppercase(value);

		out << value;
		return out;
	}

	Log::Log(std::string const& name, Severity const& severity)
		:m_name(name), m_severity(severity)
	{
		m_show_severity = true;
		m_show_line_number = false;
		m_show_file = false;
		m_show_code_scope = false;

		m_max_file_size = 10 * 1024 * 1024;
	}

	Log::~Log()
	{
		flushMessages(0);

		printFooter();

		if (m_file.is_open())
		{
			m_file.close();
		}
	}

	void Log::append(Message const& message)
	{
		if (message.severity >= m_severity)
		{
			m_message_queue.push(message);
		}
	}

	void Log::printHeader()
	{
		if (m_file.is_open())
		{
			std::string message = " " + TimestampNow() + "> Opened " + m_name + " : " + m_filename + " ";

			for (Int i = 0; i != header_line_size; ++i) { m_file << header_char; }
			m_file << std::endl;

			Int message_size = (Int)message.size();
			Int margin = 0;

			if (header_line_size > message_size)
			{
				margin = header_line_size - message_size;
				margin /= 2;
			}

			if (margin)
			{
				for (Int i = 0; i != margin; ++i) { m_file << header_char; }
			}

			m_file << message;

			if (margin)
			{
				if ((2 * margin + message_size) == (header_line_size - 1))
				{
					++margin;
				}

				for (Int i = 0; i != margin; ++i) { m_file << header_char; }
			}
			m_file << std::endl;

			for (Int i = 0; i != header_line_size; ++i) { m_file << header_char; }
			m_file << std::endl;
		}
	}

	void Log::printFooter()
	{
		if (m_file.is_open())
		{
			std::string message = " " + TimestampNow() + "> Closed " + m_name + " : " + m_filename + " ";

			for (Int i = 0; i != header_line_size; ++i) { m_file << header_char; }
			m_file << std::endl;

			Int message_size = (Int)message.size();
			Int margin = 0;

			if (header_line_size > message_size)
			{
				margin = header_line_size - message_size;
				margin /= 2;
			}

			if (margin)
			{
				for (Int i = 0; i != margin; ++i) { m_file << header_char; }
			}

			m_file << message;

			if (margin)
			{	
				if ((2 * margin + message_size) == (header_line_size - 1))
				{
					++margin;
				}

				for (Int i = 0; i != margin; ++i) { m_file << header_char; }
			}
			m_file << std::endl;

			for (Int i = 0; i != header_line_size; ++i) { m_file << header_char; }
			m_file << std::endl;
			m_file << std::endl;
			m_file << std::endl;
		}
	}

	std::string Log::getPrintableCodeLocation(SourceCodeLocation const& location, bool const show_line_number, bool show_file, bool show_code_scope)
	{
		std::stringstream output;

		bool addSpace = false;

		if (show_file)
		{
			output << getSimpleFileName(location.filename);
			addSpace = true;
		}

		if (show_line_number)
		{
			if (show_file)
			{
				output << ":";
			}

			output << location.line_number;
			addSpace = true;
		}

		if (show_code_scope)
		{
			if (addSpace)
			{
				output << " ";
			}
			
			std::string name = location.function_pretty;
			replace(name, "__thiscall ", "");

			output << name;
		}

		return output.str();
	}

	Uint Log::flushMessages(Uint numberOfMessagesToFlush)
	{
		if (m_message_queue.empty())
		{
			return 0;
		}

		if (!m_file.is_open())
		{
			Uint const max_tests = 1000;
			Uint current_test = 0;
			bool none_file = false;

			do
			{
				std::stringstream stream;
				stream << m_output_path << "/" << m_name <<"_" << std::setw(4) << std::setfill('0') << current_test << ".log";
				m_filename = stream.str();
				m_filename = convertFileNameToNativeStyle(m_filename);
				none_file = (fileSize(m_filename) > m_max_file_size);

				if (!none_file)
				{
					makeDirectoryForFile(m_filename);
					m_file.open(m_filename.c_str(), std::fstream::out | std::fstream::app);
					none_file = !m_file.is_open();
				}

				++current_test;
			}
			while(none_file && (current_test < max_tests));

			if (m_file.is_open())
			{
				printHeader();
			}
		}


		if (m_file.is_open())
		{
			Uint current = 0;
			while((current <= numberOfMessagesToFlush) && !m_message_queue.empty())
			{
				Message message;
				if (m_message_queue.tryPop(message))
				{
					if (m_show_severity)
					{
						m_file << message.severity << " ";
					}

					m_file << message.timestamp << "> ";

					std::string code_location = getPrintableCodeLocation(message.location, m_show_line_number, m_show_file, m_show_code_scope);
					if (!code_location.empty())
					{
						m_file << getPrintableCodeLocation(message.location, m_show_line_number, m_show_file, m_show_code_scope) << ": ";
					}

					m_file << message.message << std::endl;

					++current;
					if (numberOfMessagesToFlush && (current >= numberOfMessagesToFlush))
					{
						break;
					}
					Thread::microSleep(100);
				}
				else
				{
					break;
				}
			}
		}

		return 0;
	}


	LogStreamAdapter::LogStreamAdapter(Log::Severity const& severity, SourceCodeLocation const& location, SharedLog const& logger)
		:m_logger(logger)
	{
		m_message.timestamp = TimestampNow();
		m_message.severity = severity;
		m_message.location = location;
	}

	LogStreamAdapter::~LogStreamAdapter()
	{
		m_message.message = str();

		if (m_logger)
		{
			m_logger->append(m_message);
		}
	}

	std::string const LogSystem::RootLoggerName = "root";

	LogSystem::Worker::Worker(LogSystem *log_system)
		:m_log_system(log_system)
	{

	}

	void LogSystem::Worker::run()
	{
		while(keepRunning())
		{
			Uint flushed = 0;
			SharedLog logger = m_log_system->getNextLogger();
			if (logger)
			{
				flushed = logger->flushMessages(m_log_system->getConfig().flush_size);
			}

			if (flushed == 0)
			{
				Thread::microSleep(10 * 1000);
			}
		}
	}

	LogSystem::LogSystem()
	{
		m_disabled = false;

		SharedLog root = getLog(RootLoggerName);
		Log::Message message;
		message.location = RENGINE_SOURCE_CODE_LOCATION();
		message.message = "System startup";
		message.severity = Log::Trace;
		message.timestamp = TimestampNow();
		root->append(message);

		startThreading();
	}

	LogSystem::~LogSystem()
	{
		SharedLog root = getLog(RootLoggerName);
		Log::Message message;
		message.location = RENGINE_SOURCE_CODE_LOCATION();
		message.message = "System shutdown";
		message.severity = Log::Trace;
		message.timestamp = TimestampNow();
		root->append(message);

		m_disabled = true;
		stopThreading();
		releaseLogs();
	}

	void LogSystem::stopThreading()
	{
		m_workers.clear();
	}

	void LogSystem::startThreading()
	{
		stopThreading();

		for (Uint i = 0; i != m_config.thread_pool_size; ++i)
		{
			m_workers.push_back( new Worker(this) );
		}

		for (Workers::iterator i = m_workers.begin(); i != m_workers.end(); ++i)
		{
			(*i)->start();
		}
	}

	LogSystem* LogSystem::instance()
	{
		static LogSystem system;
		return &system;
	}

	void LogSystem::setConfig(Config const& config)
	{
		m_config = config;
	}

	void LogSystem::releaseLog(std::string const& name)
	{
		WriteScopedLock lock(m_loggerMutex);

		Loggers::iterator found = m_loggers.find(name);
		if (found != m_loggers.end())
		{
			m_loggers.erase(found);
		}
	}

	void LogSystem::releaseLogs()
	{
		WriteScopedLock lock(m_loggerMutex);
		m_loggers.clear();
	}
	
	SharedLog LogSystem::getLog(std::string const& name)
	{
		{
			ReadScopedLock lock(m_loggerMutex);

			if (!m_disabled)
			{
				Loggers::iterator found = m_loggers.find(name);
				if (found != m_loggers.end())
				{
					return found->second;
				}
			}
		}


		{
			WriteScopedLock lock(m_loggerMutex);

			Log::Severity default_severity = Log::Info;

			if (!m_disabled)
			{
				SharedLog log = new Log(name, default_severity);
				if (log)
				{
					log->setOutputPath(m_config.output_path);
					log->getMessageQueue().setMaxSize(m_config.logger_max_messages);
					m_loggers[name] = log;
					return log;
				}
			}
		}

		return SharedLog();
	}

	SharedLog LogSystem::getNextLogger()
	{
		ReadScopedLock lock(m_loggerMutex);

		Atomic::AtomicValue size = (Atomic::AtomicValue)m_loggers.size();
		if (size > 0)
		{
			Atomic::AtomicValue current_index = ++m_logger_index_cycle % size;

			for (Loggers::const_iterator i = m_loggers.begin(); i != m_loggers.end(); ++i)
			{
				if (current_index == 0)
				{
					return i->second;
				}
				--current_index;
			}
		}

		return SharedLog();
	}


} // namespace rengine
