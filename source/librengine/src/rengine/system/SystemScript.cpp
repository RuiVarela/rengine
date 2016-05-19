// __!!rengine_copyright!!__ //

#include <rengine/system/SystemScript.h>
#include <rengine/CoreEngine.h>
#include <rengine/outputstream/Console.h>
#include <rengine/string/String.h>
#include <rengine/lang/debug/Debug.h>

#include <sstream>


namespace rengine
{
	SystemScript::SystemScript()
	{
	}

	SystemScript::SystemScript(std::string const& script)
	{
		fromScriptText(script);
	}

	SystemScript::SystemScript(CommandList const& commands)
	:command_list(commands)
	{
	}

	Uint SystemScript::numberOfCommands() const
	{
		return Uint(command_list.size());
	}

	SystemScript::Command const& SystemScript::getCommand(Uint index) const
	{
		RENGINE_ASSERT(index < numberOfCommands());
		return command_list[index];
	}

	void SystemScript::clear()
	{
		command_list.clear();
	}

	void SystemScript::addCommand(Command const& command)
	{
		command_list.push_back(command);
	}

	Bool SystemScript::fromScriptText(std::string const& script)
	{
		std::stringstream string_stream(script);
		std::string current_line;

		while (!string_stream.eof())
		{
			std::getline(string_stream, current_line);
			trim(current_line);

			if (!current_line.empty())
			{
				addCommand(current_line);
			}
		}
		return true;
	}
}
