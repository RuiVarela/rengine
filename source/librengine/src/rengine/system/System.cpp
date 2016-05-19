// __!!rengine_copyright!!__ //

#include <rengine/CoreEngine.h>
#include <rengine/outputstream/Console.h>

#include <rengine/system/System.h>
#include <rengine/string/String.h>
#include <rengine/resource/ResourceManager.h>
#include <rengine/file/File.h>
#include <rengine/util/StringTable.h>

#include <sstream>

using namespace std;

namespace rengine
{
	System::System()
	{
		runtime_options = VariableExpansionOption;
		variable_prefix = "$";

		registerCommand( new SystemCommand("help", HelpCommand, this, "display help") );
		registerCommand( new SystemCommand("version", VersionCommand, this, "current raw engine build version") );
		registerCommand( new SystemCommand("list", ListCommand, this, "lists available commands") );
		registerCommand( new SystemCommand("echo", EchoCommand, this, "prints a message") );
		registerCommand( new SystemCommand("set", SetCommand, this, "change: '/set <var> <value>'. display: '/set <var>', list: '/set'") );
		registerCommand( new SystemCommand("runScript", RunScriptCommand, this, "runs a system script") );
		registerCommand( new SystemCommand("clear", ClearCommand, this, "clears output") );
		registerCommand( new SystemCommand("quit", QuitCommand, this, "request immediate shutdown") );
		registerCommand( new SystemCommand("showLocationTable", ShowLocationTable, this, "lists locations configured") );


		system_version = new SystemVariable("version","Build : " + std::string(__DATE__) + " " + std::string(__TIME__), SystemVariable::ConstFlag);

		copyright = new SystemVariable("copyright", "Copyright 2008, Rui Varela <rui.filipe.varela@gmail.com>");
		copyright->setHandler(this);

		registerVariable(system_version);
		registerVariable(copyright);
	}

	System::~System()
	{
	}

	void System::operator()(SystemCommand::CommandId const command, SystemCommand::Arguments const& arguments)
	{
		switch (command)
		{
			case HelpCommand:
			help();
				break;
			case ListCommand:
			list();
				break;
			case ShowLocationTable:
			showLocationTable();
				break;
			case ClearCommand:
			clear();
				break;
			case QuitCommand:
			quit();
				break;
			case EchoCommand:
			echo(arguments);
				break;
			case SetCommand:
			set(arguments);
				break;
			case RunScriptCommand:
			runScript(arguments);
				break;
			case VersionCommand:
			version();
				break;
			default:
			echo("Unbound command typed");
				break;
		}
	}

	Bool System::operator()(SystemVariable& variable, SystemVariable::Arguments const& arguments)
	{
		if (&variable == copyright.get())
		{
			echo("You cannot change copyrights!");
		}

		return true;
	}

	void System::expandVariable(SharedPointer<Variable>& variable)
	{
		RENGINE_ASSERT(variable->asString().size() > variable_prefix.size());
		RENGINE_ASSERT(startsWith(variable->asString(),variable_prefix));

		std::string::size_type prefix_size = variable_prefix.size();

		std::string name = variable->asString();
		name = name.substr(prefix_size, name.size() - prefix_size);

		if (hasVariable(name))
		{
			SharedPointer<SystemVariable> system_variable = getVariable(name);
			SharedPointer<Variable> swap_var = new Variable(system_variable->type());
			swap_var->set(*system_variable);

			variable.swap(swap_var);
		}
	}

	//TODO : use "" "to allow whitespaces"
	void System::buildArgumentList(std::string const& line, SystemCommand::Arguments& arguments)
	{
		StringElements elements = split(line, " ");

		if (elements.size() > 1)
		{
			for (StringElements::iterator iterator = (++elements.begin());
					iterator != elements.end();
					++iterator)
			{
				SharedPointer<Variable> current_variable = Variable::instance(*iterator);

				if (runtime_options & VariableExpansionOption)
				{
					if ((current_variable->type() == Variable::StringType) &&
						(current_variable->asString().size() > variable_prefix.size()) &&
						startsWith(current_variable->asString(), variable_prefix))
					{
						//test if variable prefix is escaped, i.e $$var
						if (startsWith(current_variable->asString(), variable_prefix + variable_prefix))
						{
							//we need to escape the var
							std::string name = current_variable->asString();
							name = name.substr(variable_prefix.size(), name.size() - variable_prefix.size());
							current_variable->set(name);
						}
						else
						{
							expandVariable(current_variable);
						}

					}
				}

				arguments.push_back(current_variable);
			}
		}
	}

	void System::operator()(std::string const& line)
	{
		//echo("-> " + line);
		if (line.empty())
		{
			echo("Input is empty.");
		}
//		else if (line[0] != '/')
//		{
//			echo("Input is not a command.");
//		}
		else
		{
			std::string command = line;
			trim(command);
			command = extractToken(command, " ", 0);
			replace(command, "/", ""); // remove /

			SystemCommands::iterator found = commands.find(command);
			if (found != commands.end())
			{
				SystemCommand::Arguments arguments;
				buildArgumentList(line, arguments);
				found->second->getHandler()->operator()(found->second->getId(), arguments);
			}
			else
			{
				echo("Invalid command.");
			}
		}
	}

	void System::registerCommand(SharedPointer<SystemCommand> const& command)
	{
		commands[command->getName()] = command;
	}

	void System::registerVariable(SharedPointer<SystemVariable> const& variable)
	{
		variables[variable->name()] = variable;
	}

	void System::unregisterCommand(SharedPointer<SystemCommand> const& command)
	{
		SystemCommands::iterator found = commands.find(command->getName());

		if (found != commands.end())
		{
			commands.erase(found);
		}
	}

	void System::unregisterVariable(SharedPointer<SystemVariable> const& variable)
	{
		SystemVariables::iterator found = variables.find(variable->name());

		if (found != variables.end())
		{
			variables.erase(found);
		}
	}

	Bool System::runScript(std::string const& script_filename)
	{
		SharedPointer<SystemScript> system_script;
		system_script = CoreEngine::instance()->resourceManager().load<SystemScript>(convertFileNameToNativeStyle(script_filename));

		if (system_script)
		{
			return runScript(system_script);
		}
		else
		{
			return false;
		}
	}

	Bool System::runScript(SharedPointer<SystemScript> const& script)
	{
		Bool state_ok = true;

		for (Uint command = 0; command != script->numberOfCommands(); ++command)
		{
			operator()( script->getCommand(command) );
		}

		return state_ok;
	}

	Bool System::hasVariable(std::string const& name) const
	{
		SystemVariables::const_iterator found = variables.find(name);

		return (found != variables.end());
	}

	SharedPointer<SystemVariable> const& System::getVariable(std::string const& name) const
	{
		RENGINE_ASSERT(hasVariable(name));
		SystemVariables::const_iterator found = variables.find(name);
		return found->second;
	}

	//
	// Basic commands
	//
	void System::quit()
	{
		CoreEngine::instance()->requestShutdown();
	}

	void System::clear()
	{
		CoreEngine::instance()->console().clearMessageBuffer();
	}

	void System::echo(std::string const& data)
	{
		CoreEngine::instance()->log() << data << std::endl;
	}

	void System::version()
	{
		echo(system_version->asString());
	}

	void System::echo(SystemCommand::Arguments const& arguments)
	{
		for (SystemCommand::Arguments::const_iterator iterator = arguments.begin();
			iterator != arguments.end();
			++iterator)
		{
			CoreEngine::instance()->log() << (*iterator)->toString();

			if (iterator != arguments.end())
			{
				CoreEngine::instance()->log() << ' ';
			}

		}
		CoreEngine::instance()->log() << std::endl;
	}

	void System::help()
	{
		echo("Basic system usage involves command execution.");
		echo("To list available commands use '/list'");
		echo("Changing variables is achieved through '/set'");
	}

	void System::list()
	{
		echo("Available commands :");
		for (SystemCommands::iterator i = commands.begin(); i != commands.end(); ++i)
		{
			std::stringstream string_stream;
			string_stream << "/" << i->second->getName() << " : " << i->second->getDescription();

			echo(string_stream.str());
		}
	}

	void System::showLocationTable()
	{
		echo("Location table :");
		for (StringTable::const_iterator i = CoreEngine::instance()->locationTable().begin(); i != CoreEngine::instance()->locationTable().end(); ++i)
		{
			std::stringstream string_stream;
			string_stream << "[" << i->first << "] " << i->second.text;

			echo(string_stream.str());
		}
	}

	void System::echoSystemVariable(SystemVariable const* variable)
	{
		// print the var flags
		std::ostringstream string_stream;
		string_stream << ( (variable->flags() & SystemVariable::ConstFlag) ? "R" : "*");
		string_stream << ( (variable->flags() & SystemVariable::LatchFlag) ? "L" : "*");
		string_stream << ( (variable->flags() & SystemVariable::ArchiveFlag) ? "A" : "*");
		string_stream << ' '  << variable->name();
		string_stream << " <" << variable->toString();
		string_stream << "> " << variable->description();

		echo(string_stream.str());
	}

	void System::changeSystemVariable(SystemVariable* variable, SystemCommand::Arguments const& arguments)
	{
		if (arguments.size() < 2)
		{
			echo("Invalid number of arguments.");
			echo("/set <variable name> <variable value>");
		}
		else if(variable->flags() & SystemVariable::ConstFlag)
		{
			echo("The variable is read only.");
		}
		else if(variable->flags() & SystemVariable::LatchFlag)
		{
			echo("The variable is latched.");
		}
		else
		{
			Bool do_change = true;

			if (variable->handler())
			{
				do_change = variable->handler()->operator()(*variable, arguments);
			}

			if (do_change)
			{
				if (variable->type() == Variable::StringType)
				{
					std::stringstream string_stream;
					SystemVariable::Arguments::const_iterator iterator = ++arguments.begin();

					while (iterator != arguments.end())
					{
						string_stream << (*iterator)->toString();
						++iterator;

						if (iterator != arguments.end())
						{
							string_stream << " ";
						}
					}

					variable->set(string_stream.str());
				}
				else
				{
					variable->set(*arguments[1].get());
				}
			}
		}

	}

	void System::set(SystemCommand::Arguments const& arguments)
	{
		if (arguments.size())
		{
			std::string name = arguments[0]->toString();
			trim(name);

			SystemVariables::iterator found = variables.find(name);

			if (found != variables.end())
			{
				if (arguments.size() == 1)  // show
				{
					echoSystemVariable(found->second.get());
				}
				else // change
				{
					changeSystemVariable(found->second.get(), arguments);
				}
			}
			else //TODO: implement name search and auto-completion
			{
				echo("[" +  name + "] Invalid variable.");
			}

		}
		else // list
		{
			echo("Defined Variables : ");
			for (SystemVariables::iterator i = variables.begin(); i != variables.end(); ++i)
			{
				echoSystemVariable(i->second.get());
			}
		}
	}

	void System::runScript(SystemCommand::Arguments const& arguments)
	{
		std::string syntax_error;
		Bool show_syntax = false;

		if (arguments.size() < 1)
		{
			syntax_error = "Invalid number of arguments.";
			show_syntax = true;
		}

		if (show_syntax)
		{
			CoreEngine::instance()->log() << "Bad Syntax : " << syntax_error << std::endl;
			CoreEngine::instance()->log() << "Arguments : <script filename>" << std::endl;
		}
		else
		{
			std::string script_filename = arguments[0]->asString();
			if (!runScript(script_filename))
			{
				CoreEngine::instance()->log() << "Unable to run system script : " << script_filename << std::endl;
			}
		}
	}


} //namespace rengine


