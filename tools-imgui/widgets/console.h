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
		LogSeverity::Enum severity;
		char message[128];
	};

	struct Console
	{
		TCPSocket _client;
		ConsoleLog _items[1024];
		u32 _num_items;
		Vector<DynamicString> _history;
		Vector<DynamicString> _commands;
		bool _open;

		bool _has_focus;
		s32 _history_pos; // -1: new line, 0 -> (history.size - 1): navigation
		bool _scroll_to_bottom;
		char _input_text[1024];

		///
		Console();

		///
		~Console();

		///
		void add_log(LogSeverity::Enum severity, const char* message);
	};

	// Draws the console
	void console_draw(Console& client);

	// Helpers
	void console_execute_command(Console& console, const char* command);

	// Scroll the console to the latest log message
	void console_scroll_to_bottom(Console& console);

} // namespace crown
