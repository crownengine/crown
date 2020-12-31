/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#pragma once

#include "core/containers/vector.inl"
#include "core/network/types.h"
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
		void add_log(LogSeverity::Enum severity, const char* message);
	};

	// Draws the console
	void console_draw(Console& console, TCPSocket& client);

	// Helpers
	void console_execute_command(Console& console, TCPSocket& client, const char* command);

	// Scroll the console to the latest log message
	void console_scroll_to_bottom(Console& console);

	///
	void console_send_script(TCPSocket& client, const char* lua);

	///
	void console_send_command(TCPSocket& client, char* cmd);

} // namespace crown
