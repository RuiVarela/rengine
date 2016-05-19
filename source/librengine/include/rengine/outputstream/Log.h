// __!!rengine_copyright!!__ //

#ifndef __RENGINE_LOG__
#define __RENGINE_LOG__

#include <rengine/lang/Lang.h>
#include <rengine/lang/SourceCodeLocation.h>
#include <rengine/thread/Thread.h>
#include <rengine/util/SynchronizedObjects.h>
#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>

namespace rengine
{

#ifndef RENGINE_LOG_MINIMUM_SEVERITY
	#define RENGINE_LOG_MINIMUM_SEVERITY 0
#endif //RENGINE_LOG_MINIMUM_SEVERITY

#define RLOG(logger_name, severity, message) { LogStreamAdapter RENGINE_UNIQUE_LINE(lsa)(severity, RENGINE_SOURCE_CODE_LOCATION(), LogSystem::instance()->getLog(logger_name)); RENGINE_UNIQUE_LINE(lsa) << message; }

#if RENGINE_LOG_MINIMUM_SEVERITY <= 0
	#define RLOG_TRACE(logger_name, message) RLOG(logger_name, Log::Trace, message)
#else
	#define RLOG_TRACE(logger_name, message)
#endif //RENGINE_LOG_MINIMUM_SEVERITY <= 0

#if RENGINE_LOG_MINIMUM_SEVERITY <= 1
	#define RLOG_DEBUG(logger_name, message) RLOG(logger_name, Log::Debug, message)
#else
	#define RLOG_DEBUG(logger_name, message)
#endif //RENGINE_LOG_MINIMUM_SEVERITY <= 1

#if RENGINE_LOG_MINIMUM_SEVERITY <= 2
	#define RLOG_INFO(logger_name, message) RLOG(logger_name, Log::Info, message)
#else
	#define RLOG_INFO(logger_name, message)
#endif //RENGINE_LOG_MINIMUM_SEVERITY <= 2

#if RENGINE_LOG_MINIMUM_SEVERITY <= 3
	#define RLOG_WARN(logger_name, message)	RLOG(logger_name, Log::Warn, message)
#else
	#define RLOG_WARN(logger_name, message)
#endif //RENGINE_LOG_MINIMUM_SEVERITY <= 3

#if RENGINE_LOG_MINIMUM_SEVERITY <= 4
	#define RLOG_ERROR(logger_name, message) RLOG(logger_name, Log::Error, message)
#else
	#define RLOG_ERROR(logger_name, message)
#endif //RENGINE_LOG_MINIMUM_SEVERITY <= 4

#if RENGINE_LOG_MINIMUM_SEVERITY <= 5
	#define RLOG_FATAL(logger_name, message) RLOG(logger_name, Log::Fatal, message)
#else
	#define RLOG_FATAL(logger_name, message)
#endif //RENGINE_LOG_MINIMUM_SEVERITY <= 5


	//
	// Log
	//
	class Log
	{
	public:
		enum Severity
		{
			Trace	= 0,
			Debug	= 1,
			Info	= 2,
			Warn	= 3,
			Error	= 4,
			Fatal	= 5
		};

		struct Message
		{
			Severity severity;
			std::string message;
			SourceCodeLocation location;
			std::string timestamp;
		};

		typedef SynchronizedQueue<Message> MessageQueue;

		void setSeverity(Severity const& severity) { m_severity = severity; }
		Severity getSeverity() const { return m_severity; }

		void setShowSeverity(bool show = true) { m_show_severity = show; }
		bool getShowSeverity() const { return m_show_severity; }

		void setShowLineNumber(bool show = true) { m_show_line_number = show; }
		bool getShowLineNumber() const { return m_show_line_number; }

		void setShowFile(bool show = true) { m_show_file = show; }
		bool getShowFile() const { return m_show_file; }

		void setShowCodeScope(bool show = true) { m_show_code_scope = show; }
		bool getShowCodeScope() const { return m_show_code_scope; }

		void setMaxFileSize(int size) { m_max_file_size = size; }
		int getMaxFileSize() const { return m_max_file_size; }
		

		void append(Message const& message);


		std::string const& getName() { return m_name; }
		static std::string getPrintableCodeLocation(SourceCodeLocation const& location, bool const show_line_number, bool show_file, bool show_code_scope);

		MessageQueue& getMessageQueue() { return m_message_queue; }
		MessageQueue const& getMessageQueue() const { return m_message_queue; }

		Uint flushMessages(Uint numberOfMessagesToFlush);

		void setOutputPath(std::string const& output_path) { m_output_path = output_path; }
	private:
		Log(std::string const& name, Severity const& severity);
		~Log();

		void printHeader();
		void printFooter();

		friend class LogSystem;
		friend class SharedPointer<Log>;

		std::string m_output_path;
		std::string m_name;
		std::ofstream m_file;
		MessageQueue m_message_queue;
		std::string m_filename;

		Severity m_severity;

		bool m_show_severity;
		bool m_show_line_number;
		bool m_show_file;
		bool m_show_code_scope;

		int m_max_file_size;
	};

	typedef SharedPointer<Log> SharedLog;
	typedef std::map<std::string, SharedLog> Loggers;


	//
	// This adapter flushes data to logger when the instance is destroyed
	//
	class LogStreamAdapter : public std::ostringstream
	{
	public:
		LogStreamAdapter(Log::Severity const& severity, SourceCodeLocation const& location, SharedLog const& logger);
		virtual ~LogStreamAdapter();
	private:
		SharedLog m_logger;
		Log::Message m_message;
	};


	//
	// LogSystem Manager
	//
	class LogSystem
	{
	public:
		struct Config
		{
			Config()
				:logger_max_messages(100),
				 thread_pool_size(1),
				 flush_size(20),
				 output_path("logs")
			{}

			Uint logger_max_messages; 		// max messages a log handles in queue before dropping
			Uint thread_pool_size;			// number of working threads
			Uint flush_size;				// number of messages a thread flushes per step
			std::string output_path;		// output directory
		};


		static const std::string RootLoggerName;

		static LogSystem* instance();

		void setConfig(Config const& config);
		Config const& getConfig() { return m_config; }

		void releaseLog(std::string const& name);
		void releaseLogs();
		SharedLog getLog(std::string const& name);
	private:
		LogSystem();
		~LogSystem();

		void startThreading();
		void stopThreading();

		volatile Bool m_disabled;
		Config m_config;
		Loggers m_loggers;
		ReadWriteMutex m_loggerMutex;

		class Worker : public Thread
		{
		public:
			Worker(LogSystem *log_system);
			virtual void run();

		private:
			LogSystem *m_log_system;
		};

		typedef SharedPointer<Worker> SharedWorker;
		typedef std::vector<SharedWorker> Workers;
		Workers m_workers;
		Atomic m_logger_index_cycle;

		SharedLog getNextLogger();
	};

} // namespace rengine

#endif //__RENGINE_LOG__
