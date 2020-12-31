/*
 * Copyright (c) 2012-2021 Daniele Bartolini et al.
 * License: https://github.com/dbartolini/crown/blob/master/LICENSE
 */

#include "core/memory/temp_allocator.inl"
#include "core/network/ip_address.h"
#include "core/network/socket.h"
#include "core/strings/dynamic_string.inl"
#include "core/strings/string_stream.inl"
#include <string.h> // strtok_r
#if CROWN_COMPILER_MSVC
	#define strtok_r strtok_s
#endif

namespace crown
{
static void console_send(TCPSocket& client, const char* json)
{
	const u32 size = strlen32(json);
	client.write(&size, sizeof(u32));
	client.write(json, size);
}

static void console_sanitize_json_string(StringStream& json, const char* str)
{
	const char* ch = str;
	for (; *ch; ch++)
	{
		if (*ch == '"' || *ch == '\\')
			json << "\\";
		json << *ch;
	}
}

void console_send_script(TCPSocket& client, const char* lua)
{
	TempAllocator1024 ta;
	StringStream json(ta);

	json << "{\"type\":\"script\",\"script\":\"";
	console_sanitize_json_string(json, lua);
	json << "\"}";

	console_send(client, string_stream::c_str(json));
}

void console_send_repl(TCPSocket& client, const char* lua)
{
	TempAllocator1024 ta;
	StringStream json(ta);

	json << "{\"type\":\"repl\",\"repl\":\"";
	console_sanitize_json_string(json, lua);
	json << "\"}";

	console_send(client, string_stream::c_str(json));
}

void console_send_command(TCPSocket& client, char* cmd)
{
	TempAllocator1024 ta;
	StringStream json(ta);

	char* saveptr;
	const char* delim = " ";
	const char* token = strtok_r(cmd, delim, &saveptr);

	if (token == NULL)
		return;

	json << "{\"type\":\"command\",\"args\":[";
	while (token != NULL)
	{
		json << "\"";
		console_sanitize_json_string(json, token);
		json << "\",";
		token = strtok_r(NULL, delim, &saveptr);
	}
	json << "]}";

	console_send(client, string_stream::c_str(json));
}

Console::Console()
	: _num_items(0)
	, _history(default_allocator())
	, _commands(default_allocator())
	, _open(true)
	, _has_focus(false)
	, _history_pos(-1)
	, _scroll_to_bottom(true)
{
	for (u32 i = 0; i < countof(_items); ++i)
		_items[i].severity = LogSeverity::COUNT;

	// FIXME: clear tmp commands
	TempAllocator128 a;
	DynamicString str(a);
	str = "help - Show this";
	vector::push_back(_commands, str);
	str = "clear - Clear console";
	vector::push_back(_commands, str);
	str = "history - Show recent commands";
	vector::push_back(_commands, str);

	strcpy(_input_text, "> ");
}

void Console::add_log(LogSeverity::Enum severity, const char* message)
{
	_items[_num_items].severity = severity;
	strncpy(_items[_num_items].message, message, sizeof(_items[_num_items].message)-1);
	_items[_num_items].message[sizeof(_items[_num_items].message)-1] = '\0';
	_num_items = (_num_items + 1) % countof(_items);
}

int console_inputtext_callback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackHistory)
	{
		Console* console = (Console*) data->UserData;
		const Vector<DynamicString>& history = console->_history;
		s32& history_pos = console->_history_pos;
		const s32 prev_history_pos = history_pos;

		if (ImGuiKey_UpArrow == data->EventKey)
		{
			if (history_pos == -1)
				history_pos = vector::size(history) - 1;
			else if (history_pos > 0)
				history_pos--;
		}
		else if (ImGuiKey_DownArrow == data->EventKey)
		{
			if (history_pos != -1)
			{
				if (++history_pos >= (int)vector::size(history))
					history_pos = -1;
			}
		}

		if (prev_history_pos != history_pos)
		{
			s32 len = snprintf(data->Buf, (size_t) data->BufSize, "%s", (history_pos >= 0) ? history[history_pos].c_str() : "");
			data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = len;
			data->BufDirty = true;
		}
	}

	return 0;
}

void console_draw(Console& console, TCPSocket& client)
{
	ImGui::Begin("Console", &console._open, ImGuiWindowFlags_NoScrollbar);

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
	static ImGuiTextFilter filter;
	filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
	ImGui::PopStyleVar();
	ImGui::Separator();

	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
	ImGui::BeginChild("ScrollingRegion"
		, ImVec2(0.0f, -footer_height_to_reserve)
		, false
		, ImGuiWindowFlags_HorizontalScrollbar
		);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1)); // Tighten spacing
	for (u32 i = console._num_items, n = 0; n < countof(console._items); i = (i + 1) % countof(console._items), ++n)
	{
		ConsoleLog& item = console._items[i];
		if (item.severity == LogSeverity::COUNT)
			continue;

		if (!filter.PassFilter(item.message))
			continue;

		ImVec4 col = ImVec4(1.0f,1.0f,1.0f,1.0f);

		switch (item.severity)
		{
			case LogSeverity::LOG_INFO: col = ImColor(0.7f, 0.7f, 0.7f, 1.0f); break;
			case LogSeverity::LOG_WARN: col = ImColor(1.0f, 1.0f, 0.4f, 1.0f); break;
			case LogSeverity::LOG_ERROR: col = ImColor(1.0f, 0.4f, 0.4f, 1.0f); break;
			default: CE_FATAL("Unknown Severity"); break;
		}
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::PushFont(ImGui::Font::Mono);
		ImGui::TextUnformatted(item.message);
		ImGui::PopFont();
		ImGui::PopStyleColor();
	}

	if (console._scroll_to_bottom)
		ImGui::SetScrollHereY();

	console._scroll_to_bottom = false;
	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	ImGui::PushItemWidth(-1);
	ImGui::PushFont(ImGui::Font::Mono);
	if (ImGui::InputText("##label"
		, console._input_text + 2
		, IM_ARRAYSIZE(console._input_text) - 2
		, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory
		, console_inputtext_callback
		, &console
		))
	{
		// FIXME: use rtrim()
		char* input_end = console._input_text + strlen(console._input_text);
		while (input_end > console._input_text && input_end[-1] == ' ') input_end--;
		*input_end = 0;

		console.add_log(LogSeverity::LOG_INFO, console._input_text);

		if (console._input_text[2] != 0)
			console_execute_command(console, client, console._input_text + 2);

		strcpy(console._input_text, "> ");
		console._history_pos = -1;

		ImGui::SetKeyboardFocusHere(-1);
		console._scroll_to_bottom = true;
	}
	ImGui::PopFont();
	ImGui::PopItemWidth();

	ImGui::End();
}

void console_execute_command(Console& console, TCPSocket& client, const char* command)
{
	Vector<DynamicString>& history = console._history;
	Vector<DynamicString>& commands = console._commands;

	{
		TempAllocator512 ta;
		DynamicString str(ta);
		str = command;
		vector::push_back(history, str);
	}

	// Process command
	if (strcmp(command, "clear") == 0)
	{
		console._num_items = 0;
	}
	else if (strcmp(command, "history") == 0)
	{
		int first = vector::size(history)-10;
		for (u32 i = first > 0 ? first : 0; i < vector::size(history); i++)
		{
			char buf[64];
			sprintf(buf, "%3d: %s\n", i, history[i].c_str());
			console.add_log(LogSeverity::LOG_INFO, buf);
		}
	}
	else if (strcmp(command, "help") == 0)
	{
		console.add_log(LogSeverity::LOG_INFO, "commands: ");
		for (u32 i = 0; i < vector::size(commands); i++)
			console.add_log(LogSeverity::LOG_INFO, commands[i].c_str());
	}
	else if (command[0] == ':')
	{
		console_send_command(client, (char*)&command[1]);
	}
	else
	{
		console_send_repl(client, command);
	}

	console._scroll_to_bottom = true;
}

void console_scroll_to_bottom(Console& console)
{
	console._scroll_to_bottom = true;
}

} // namespace crown
