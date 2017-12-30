// forward declarations
#include "core/containers/vector.h"
#include "core/network/socket.h"
#include "core/strings/dynamic_string.h"
#include "device/log.h"

namespace ImGui
{
	using namespace crown;

	struct ConsoleLog
	{
		LogSeverity::Enum _severity;
		DynamicString _message;

		ConsoleLog(LogSeverity::Enum severity = LogSeverity::LOG_INFO, const char* message="")
			: _severity(severity)
			, _message(default_allocator())
		{
			_message = message;
		}
	};

	void console_draw(TCPSocket& client
		, Vector<ConsoleLog>& items
		, Vector<DynamicString>& history
		, Vector<DynamicString>& commands
		);

	// Helpers
	void console_execute_command(TCPSocket& client
		, Vector<ConsoleLog>& items
		, Vector<DynamicString>& history
		, Vector<DynamicString>& commands
		, ConsoleLog& command_line
		);

} // namespace ImGui
