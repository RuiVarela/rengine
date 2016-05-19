// __!!rengine_copyright!!__ //

#ifndef __RENGINE_SYSTEM_SCRIPT_H__
#define __RENGINE_SYSTEM_SCRIPT_H__

#include <rengine/lang/Lang.h>
#include <string>
#include <vector>

namespace rengine
{
	class SystemScript
	{
	public:
		typedef std::string Command;
		typedef std::vector<Command> CommandList;

		SystemScript();

		SystemScript(std::string const& script);
		SystemScript(CommandList const& commands);

		Bool fromScriptText(std::string const& script);

		Uint numberOfCommands() const;
		Command const& getCommand(Uint index) const;

		void clear();
		void addCommand(Command const& command);
	private:
		CommandList command_list;
		Bool is_valid;
	};
}

#endif //__RENGINE_SYSTEM_SCRIPT_H__
