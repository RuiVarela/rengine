// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SYSTEM_H__
#define __RENGINE_SYSTEM_H__

#include <rengine/system/SystemFeed.h>
#include <rengine/system/SystemVariable.h>
#include <rengine/system/SystemScript.h>

#include <string>
#include <map>

namespace rengine
{
	class SystemCommand
	{
	public:
		typedef VariableVector Arguments;
		typedef Uint CommandId;

		struct Handler
		{
			virtual ~Handler();
			virtual void operator()(CommandId const command, Arguments const& arguments) = 0;
		};

		SystemCommand();
		SystemCommand(std::string const& name, Handler * handler);
		SystemCommand(std::string const& name, CommandId const id, Handler* handler);
		SystemCommand(std::string const& name, CommandId const id, Handler* handler, std::string const& description);
		~SystemCommand();

		void setHandler(Handler* handler);
		void setName(std::string const& name);
		void setId(CommandId const& id);
		void setDescription(std::string const& description);

		CommandId const& getId();
		std::string const& getName() const;
		std::string const& getDescription() const;
		Handler* getHandler();
	private:
		CommandId id_;
		std::string name_;
		std::string description_;
		Handler* handler_;
	};


	class System : public SystemFeed, public SystemCommand::Handler, public SystemVariable::Handler
	{
	public:
		typedef std::map<std::string, SharedPointer<SystemCommand> > SystemCommands;
		typedef std::map<std::string, SharedPointer<SystemVariable> > SystemVariables;

		enum RuntimeOption
		{
			NoneOption					= 0,
			VariableExpansionOption		= 1
		};

		enum BuiltInCommands
		{
			HelpCommand,
			ListCommand,
			ShowLocationTable,
			VersionCommand,
			ClearCommand,
			EchoCommand,
			SetCommand,
			RunScriptCommand,
			QuitCommand
		};

		System();
		virtual ~System();

		void registerCommand(SharedPointer<SystemCommand> const& command);
		void unregisterCommand(SharedPointer<SystemCommand> const& command);

		void registerVariable(SharedPointer<SystemVariable> const& variable);
		void unregisterVariable(SharedPointer<SystemVariable> const& variable);

		Bool runScript(std::string const& script_filename);
		Bool runScript(SharedPointer<SystemScript> const& script);

		Bool hasVariable(std::string const& name) const;
		SharedPointer<SystemVariable> const& getVariable(std::string const& name) const;

		void operator()(std::string const& line); // data processor

		//built-in commands and variables
		void operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments);
		Bool operator()(SystemVariable& variable, SystemVariable::Arguments const& arguments);

		//
		// Built-in SystemCommands
		//
		void help();
		void list();
		void version();
		void clear();
		void showLocationTable();
		void echo(std::string const& data);
		void echo(SystemCommand::Arguments const& arguments);
		void set(SystemCommand::Arguments const& arguments);
		void runScript(SystemCommand::Arguments const& arguments);
		void quit();

		void changeSystemVariable(SystemVariable* variable, SystemCommand::Arguments const& arguments);
	private :
		void expandVariable(SharedPointer<Variable>& variable);
		void buildArgumentList(std::string const& line, SystemCommand::Arguments& arguments);
		void echoSystemVariable(SystemVariable const* variable);
		

		SystemCommands commands;
		SystemVariables variables;

		SharedPointer<SystemVariable> system_version;
		SharedPointer<SystemVariable> copyright;

		RuntimeOption runtime_options;
		std::string variable_prefix;
	};


	//
	// Implementation
	//

	RENGINE_INLINE SystemCommand::Handler::~Handler()
	{

	}

	RENGINE_INLINE SystemCommand::SystemCommand() :
		id_(0), name_("not_defined"), description_("description not available.")
	{
	}

	RENGINE_INLINE SystemCommand::SystemCommand(std::string const& name, SystemCommand::Handler* handler) :
		id_(0), name_(name), handler_(handler)
	{
	}

	RENGINE_INLINE SystemCommand::SystemCommand(std::string const& name, CommandId const id, SystemCommand::Handler* handler) :
		id_(id), name_(name), handler_(handler)
	{
	}

	RENGINE_INLINE SystemCommand::SystemCommand(std::string const& name, CommandId const id, SystemCommand::Handler* handler, std::string const& description) :
		id_(id), name_(name), description_(description), handler_(handler)
	{
	}

	RENGINE_INLINE SystemCommand::~SystemCommand()
	{
	}

	RENGINE_INLINE void SystemCommand::setHandler(Handler* handler)
	{
		handler_ = handler;
	}

	RENGINE_INLINE void SystemCommand::setName(std::string const& name)
	{
		name_ = name;
	}

	RENGINE_INLINE void SystemCommand::setId(CommandId const& id)
	{
		id_ = id;
	}

	RENGINE_INLINE void SystemCommand::setDescription(std::string const& description)
	{
		description_ = description;
	}

	RENGINE_INLINE SystemCommand::CommandId const& SystemCommand::getId()
	{
		return id_;
	}

	RENGINE_INLINE std::string const& SystemCommand::getName() const
	{
		return name_;
	}

	RENGINE_INLINE std::string const& SystemCommand::getDescription() const
	{
		return description_;
	}

	RENGINE_INLINE SystemCommand::Handler* SystemCommand::getHandler()
	{
		return handler_;
	}


} // namespace rengine

#endif // __RENGINE_SYSTEM_H__
