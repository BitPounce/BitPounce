#include <bp_pch.h>
#include "Console.h"
#include "ConsoleRegistrars.h"

namespace BitPounce
{
	std::vector<std::string> Console::s_Logs = std::vector<std::string>();

	BP_REGISTER_CMD("help", "just some info",
	[](const std::vector<std::string>& args)
	{
		BP_INFO("Help command executed");
		BP_INFO("Use (var_name) (var_value) to set a var, or use (var_name) to see a var!");
		BP_INFO("Use (cmd_name) (?cmd_args?) to run a cmd");
	});
}