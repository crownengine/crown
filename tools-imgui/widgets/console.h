#pragma once

#include "config.h"
#include "core/containers/vector.h"
#include "core/memory/temp_allocator.h"
#include "core/network/ip_address.h"
#include "core/network/socket.h"
#include "core/strings/dynamic_string.h"
#include "device/log.h"

namespace crown
{
	struct ConsoleLog
	{
		LogSeverity::Enum _severity;
		crown::DynamicString _message;

		ConsoleLog(LogSeverity::Enum severity = LogSeverity::LOG_INFO, const char* message="")
			: _severity(severity)
			, _message(default_allocator())
		{
			_message = message;
		}
	};

	//-----------------------------------------------------------------------------
	struct Console
	{
		TCPSocket _client;
		Vector<ConsoleLog> _console_items;
		Vector<DynamicString> _console_history;
		Vector<DynamicString> _console_commands;
		bool _open;

		bool _has_focus;
		s32 _history_pos;	// -1: new line, 0 -> (history.size - 1): navigation

		Console()
			: _console_items(default_allocator())
			, _console_history(default_allocator())
			, _console_commands(default_allocator())
			, _open(true)
			, _has_focus(false)
			, _history_pos(-1)
		{
			_client.connect(IP_ADDRESS_LOOPBACK, CROWN_DEFAULT_CONSOLE_PORT);

			// FIXME: clear tmp commands
			TempAllocator128 a;
			DynamicString str(a);
			str = "help - Show this";
			vector::push_back(_console_commands, str);
			str = "clear - Clear console";
			vector::push_back(_console_commands, str);
			str = "history - Show recent commands";
			vector::push_back(_console_commands, str);
		}

		~Console()
		{
			_client.close();
		}
	};

	void console_draw(Console& client);

	// Helpers
	void console_execute_command(Console& console, ConsoleLog& command_line);

	void console_scroll_to_bottom();

} // namespace crown
